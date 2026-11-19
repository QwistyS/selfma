//! `selfma`: tasks from the command line, and a `sync` for systems that send their own task list.
//!
//! The database is `$SELFMA_DB`, or `~/.local/share/selfma/selfma.db`.
//!
//! ```text
//! selfma add --project NAME [--key KEY] TEXT...     add a task (or update the one with KEY)
//! selfma list [--project NAME] [--open] [--json]
//! selfma done ID | --key KEY
//! selfma sync --project NAME --prefix PREFIX        stdin: JSON [{"key": .., "text": ..}]
//! ```
//! `sync` makes the project's open tasks whose key starts with PREFIX match stdin exactly: new keys
//! are added, known keys get the new text, and open tasks missing from stdin are marked done.
use std::io::Read;
use std::path::PathBuf;
use std::process::ExitCode;

use selfma::{ProjectId, SelfmaContext, SelfmaError, Task, TaskId};
use serde::{Deserialize, Serialize};

#[derive(Deserialize)]
struct SyncItem {
    key: String,
    text: String,
}

#[derive(Serialize)]
struct TaskRow<'a> {
    id: u32,
    project: &'a str,
    key: Option<&'a str>,
    text: &'a str,
    created_at: String,
    done_at: Option<String>,
}

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().skip(1).collect();
    match run(&args) {
        Ok(()) => ExitCode::SUCCESS,
        Err(error) => {
            eprintln!("selfma: {error}");
            ExitCode::FAILURE
        }
    }
}

fn db_path() -> Result<PathBuf, String> {
    if let Ok(path) = std::env::var("SELFMA_DB") {
        return Ok(PathBuf::from(path));
    }
    let home = std::env::var("HOME").map_err(|_| "HOME is not set; set SELFMA_DB")?;
    let dir = PathBuf::from(home).join(".local/share/selfma");
    std::fs::create_dir_all(&dir).map_err(|e| format!("{}: {e}", dir.display()))?;
    Ok(dir.join("selfma.db"))
}

type Options = Vec<(String, String)>;

/// Splits `--flag value` options from positional words; `--open`/`--json` take no value.
fn parse(args: &[String]) -> Result<(Options, Vec<String>), String> {
    let (mut options, mut words) = (Vec::new(), Vec::new());
    let mut iter = args.iter();
    while let Some(arg) = iter.next() {
        match arg.as_str() {
            "--open" | "--json" => options.push((arg.clone(), String::new())),
            flag if flag.starts_with("--") => {
                let value = iter.next().ok_or(format!("{flag} needs a value"))?;
                options.push((flag.to_string(), value.clone()));
            }
            _ => words.push(arg.clone()),
        }
    }
    Ok((options, words))
}

fn option<'a>(options: &'a [(String, String)], name: &str) -> Option<&'a str> {
    options
        .iter()
        .find(|(k, _)| k == name)
        .map(|(_, v)| v.as_str())
}

fn project_id(ctx: &mut SelfmaContext, name: &str) -> Result<ProjectId, SelfmaError> {
    match ctx.project_by_name(name) {
        Some(project) => Ok(project.id),
        None => ctx.add_project(name, name),
    }
}

fn run(args: &[String]) -> Result<(), String> {
    let (command, rest) = args.split_first().ok_or(USAGE)?;
    let (options, words) = parse(rest)?;
    let mut ctx = SelfmaContext::new(&db_path()?).map_err(|e| e.to_string())?;
    ctx.deserialize().map_err(|e| e.to_string())?;
    match command.as_str() {
        "add" => {
            let project = option(&options, "--project").ok_or("add needs --project")?;
            let text = words.join(" ");
            let project = project_id(&mut ctx, project).map_err(|e| e.to_string())?;
            let id = match option(&options, "--key") {
                Some(key) => ctx
                    .upsert_task_by_key(project, key, &text, 0.0)
                    .map(|(id, _)| id),
                None => ctx.add_task(project, &text, 0.0),
            }
            .map_err(|e| e.to_string())?;
            println!("{}", id.0);
        }
        "list" => {
            let project = option(&options, "--project");
            let open = option(&options, "--open").is_some();
            let tasks: Vec<&Task> = ctx
                .list_tasks()
                .into_iter()
                .filter(|t| !open || !t.is_done())
                .filter(|t| project.is_none_or(|name| project_name(&ctx, t) == name))
                .collect();
            if option(&options, "--json").is_some() {
                let rows: Vec<TaskRow> = tasks.iter().map(|t| row(&ctx, t)).collect();
                println!(
                    "{}",
                    serde_json::to_string(&rows).map_err(|e| e.to_string())?
                );
            } else {
                for t in tasks {
                    let mark = if t.is_done() { "x" } else { " " };
                    println!(
                        "[{mark}] {:>4}  {}: {}",
                        t.id.0,
                        project_name(&ctx, t),
                        t.description
                    );
                }
            }
        }
        "done" => {
            let id = match (option(&options, "--key"), words.first()) {
                (Some(key), _) => ctx
                    .task_by_key(key)
                    .map(|t| t.id)
                    .ok_or_else(|| SelfmaError::TaskKeyNotFound(key.to_string()).to_string())?,
                (None, Some(id)) => TaskId(id.parse().map_err(|_| format!("not a task id: {id}"))?),
                (None, None) => return Err("done needs a task id or --key".into()),
            };
            ctx.complete_task(id).map_err(|e| e.to_string())?;
        }
        "sync" => {
            let project = option(&options, "--project").ok_or("sync needs --project")?;
            let prefix = option(&options, "--prefix").ok_or("sync needs --prefix")?;
            let mut input = String::new();
            std::io::stdin()
                .read_to_string(&mut input)
                .map_err(|e| e.to_string())?;
            let items: Vec<SyncItem> = serde_json::from_str(&input)
                .map_err(|e| format!("stdin is not a task list: {e}"))?;
            if let Some(bad) = items.iter().find(|i| !i.key.starts_with(prefix)) {
                return Err(format!("key {:?} does not start with {prefix:?}", bad.key));
            }
            let project = project_id(&mut ctx, project).map_err(|e| e.to_string())?;
            let (mut added, mut closed) = (0, 0);
            for item in &items {
                let (_, created) = ctx
                    .upsert_task_by_key(project, &item.key, &item.text, 0.0)
                    .map_err(|e| e.to_string())?;
                added += usize::from(created);
            }
            let stale: Vec<TaskId> = ctx
                .list_tasks()
                .into_iter()
                .filter(|t| t.project_id == project && !t.is_done())
                .filter(|t| t.key.as_deref().is_some_and(|k| k.starts_with(prefix)))
                .filter(|t| {
                    !items
                        .iter()
                        .any(|i| Some(i.key.as_str()) == t.key.as_deref())
                })
                .map(|t| t.id)
                .collect();
            for id in stale {
                ctx.complete_task(id).map_err(|e| e.to_string())?;
                closed += 1;
            }
            eprintln!("selfma: {} sent, {added} new, {closed} closed", items.len());
        }
        _ => return Err(USAGE.into()),
    }
    ctx.serialize().map_err(|e| e.to_string())
}

fn project_name<'a>(ctx: &'a SelfmaContext, task: &Task) -> &'a str {
    ctx.get_project_name(task.project_id).unwrap_or("?")
}

fn row<'a>(ctx: &'a SelfmaContext, t: &'a Task) -> TaskRow<'a> {
    TaskRow {
        id: t.id.0,
        project: project_name(ctx, t),
        key: t.key.as_deref(),
        text: &t.description,
        created_at: t.created_at.to_rfc3339(),
        done_at: t.done_at.map(|at| at.to_rfc3339()),
    }
}

const USAGE: &str = "usage: selfma add --project NAME [--key KEY] TEXT... | list [--project NAME] [--open] [--json] | done ID|--key KEY | sync --project NAME --prefix PREFIX < tasks.json";
