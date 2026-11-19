use selfma::{Event, EventKind, ProjectId, SelfmaContext, SelfmaError};
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;

fn tmp_db() -> (tempfile::TempDir, std::path::PathBuf) {
    let dir = tempfile::tempdir().unwrap();
    let path = dir.path().join("selfma.db");
    (dir, path)
}

#[test]
fn crud_lifecycle() {
    let (_dir, db) = tmp_db();
    let mut ctx = SelfmaContext::new(&db).unwrap();

    let project = ctx.add_project("Ramen", "taste the soup").unwrap();
    let task = ctx.add_task(project, "boil noodles", 300.0).unwrap();
    let other = ctx.add_task(project, "season broth", 600.0).unwrap();

    let snapshot = ctx.get_project(project).unwrap();
    assert_eq!(snapshot.project.name, "Ramen");
    assert_eq!(snapshot.tasks.len(), 2);

    ctx.remove_task(project, task).unwrap();
    assert!(ctx.get_task(task).is_none());
    assert_eq!(ctx.get_project(project).unwrap().tasks.len(), 1);

    ctx.remove_project(project).unwrap();
    assert!(ctx.get_project(project).is_none());
    assert!(matches!(
        ctx.add_task(project, "ghost", 1.0),
        Err(SelfmaError::ProjectNotFound(_))
    ));
    let _ = other;
}

#[test]
fn ids_are_reused_after_remove() {
    let (_dir, db) = tmp_db();
    let mut ctx = SelfmaContext::new(&db).unwrap();

    let first = ctx.add_project("a", "one").unwrap();
    let second = ctx.add_project("b", "two").unwrap();
    ctx.remove_project(first).unwrap();

    let reused = ctx.add_project("c", "three").unwrap();
    assert_eq!(reused, first);
    assert_ne!(reused, second);
}

#[test]
fn serialize_deserialize_roundtrip() {
    let (_dir, db) = tmp_db();

    {
        let mut ctx = SelfmaContext::new(&db).unwrap();
        let project = ctx.add_project("persisted", "survives reload").unwrap();
        ctx.add_task(project, "write to disk", 42.0).unwrap();
        ctx.serialize().unwrap();
    }

    {
        let mut ctx = SelfmaContext::new(&db).unwrap();
        ctx.deserialize().unwrap();
        let projects = ctx.list_projects();
        assert_eq!(projects.len(), 1);
        assert_eq!(projects[0].name, "persisted");
        let snapshot = ctx.get_project(projects[0].id).unwrap();
        assert_eq!(snapshot.tasks.len(), 1);
        assert_eq!(snapshot.tasks[0].description, "write to disk");
    }
}

#[test]
fn serialized_ids_continue_from_previous_context() {
    let (_dir, db) = tmp_db();

    {
        let mut ctx = SelfmaContext::new(&db).unwrap();
        ctx.add_project("p", "d").unwrap();
        ctx.serialize().unwrap();
    }

    let mut ctx = SelfmaContext::new(&db).unwrap();
    ctx.deserialize().unwrap();
    let again = ctx.add_project("q", "e").unwrap();
    assert_eq!(again, ProjectId(1));
}

#[test]
fn update_fires_event_on_elapsed_task() {
    let (_dir, db) = tmp_db();
    let mut ctx = SelfmaContext::new(&db).unwrap();
    let project = ctx.add_project("timers", "elapsing tasks").unwrap();
    let task = ctx.add_task(project, "done quick", 0.0).unwrap();

    let seen = Arc::new(AtomicUsize::new(0));
    let seen_clone = Arc::clone(&seen);
    ctx.on_event(Box::new(move |event: &Event| {
        if event.kind == EventKind::TaskTimeElapsed && event.task_id == Some(task) {
            seen_clone.fetch_add(1, Ordering::SeqCst);
        }
    }));

    ctx.update().unwrap();

    assert_eq!(seen.load(Ordering::SeqCst), 1);
    assert!(ctx.get_task(task).is_none());
}

#[test]
fn validation_rejects_bad_input() {
    let (_dir, db) = tmp_db();
    let mut ctx = SelfmaContext::new(&db).unwrap();

    assert!(matches!(
        ctx.add_project("", "d"),
        Err(SelfmaError::EmptyName)
    ));
    let project = ctx.add_project("valid", "ok").unwrap();
    assert!(matches!(
        ctx.add_task(project, "", 1.0),
        Err(SelfmaError::EmptyDescription)
    ));
    assert!(matches!(
        ctx.add_task(project, "neg", -1.0),
        Err(SelfmaError::InvalidDuration)
    ));
}

#[test]
fn deserializes_into_fresh_context_only_on_request() {
    let (_dir, db) = tmp_db();

    let mut ctx = SelfmaContext::new(&db).unwrap();
    let _project = ctx.add_project("present only after load", "d").unwrap();
    ctx.serialize().unwrap();

    let mut fresh = SelfmaContext::new(&db).unwrap();
    assert!(fresh.list_projects().is_empty());

    fresh.deserialize().unwrap();
    assert_eq!(fresh.list_projects().len(), 1);
}

#[test]
fn keyed_tasks_are_upserted_completed_and_survive_a_reload() {
    let dir = tempfile::tempdir().unwrap();
    let path = dir.path().join("keyed.db");
    {
        let mut ctx = selfma::SelfmaContext::new(&path).unwrap();
        ctx.deserialize().unwrap();
        let project = ctx.add_project("Launjera", "business").unwrap();
        let (first, created) = ctx
            .upsert_task_by_key(project, "pulse:unpaid", "Collect 19", 0.0)
            .unwrap();
        assert!(created);
        let (again, created) = ctx
            .upsert_task_by_key(project, "pulse:unpaid", "Collect 17", 0.0)
            .unwrap();
        assert_eq!((first, created), (again, false));
        assert_eq!(ctx.get_task(first).unwrap().description, "Collect 17");
        // A keyed task with an elapsed timer is not expired by update().
        ctx.update().unwrap();
        assert!(ctx.get_task(first).is_some());
        ctx.complete_task(first).unwrap();
        ctx.serialize().unwrap();
    }
    let mut ctx = selfma::SelfmaContext::new(&path).unwrap();
    ctx.deserialize().unwrap();
    let task = ctx.task_by_key("pulse:unpaid").unwrap();
    assert!(task.is_done());
    assert_eq!(task.description, "Collect 17");
    // Re-sending a done task's key updates its text but keeps it done.
    let project = ctx.project_by_name("Launjera").unwrap().id;
    let (id, created) = ctx
        .upsert_task_by_key(project, "pulse:unpaid", "Collect 15", 0.0)
        .unwrap();
    assert!(!created);
    assert!(ctx.get_task(id).unwrap().is_done());
}

#[test]
fn a_version_one_database_gains_keys_and_done_markers() {
    let dir = tempfile::tempdir().unwrap();
    let path = dir.path().join("v1.db");
    {
        let conn = rusqlite::Connection::open(&path).unwrap();
        conn.execute_batch(
            "CREATE TABLE schema_meta (key TEXT PRIMARY KEY, value TEXT NOT NULL);
             CREATE TABLE projects (id INTEGER PRIMARY KEY, name TEXT NOT NULL, description TEXT NOT NULL, created_at INTEGER NOT NULL);
             CREATE TABLE tasks (id INTEGER PRIMARY KEY, project_id INTEGER NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
                 description TEXT NOT NULL, duration_sec REAL NOT NULL, created_at INTEGER NOT NULL);
             INSERT INTO projects VALUES (1, 'old', 'v1', 0);
             INSERT INTO tasks VALUES (1, 1, 'old task', 60, 0);",
        )
        .unwrap();
    }
    let mut ctx = selfma::SelfmaContext::new(&path).unwrap();
    ctx.deserialize().unwrap();
    let task = ctx.get_task(selfma::TaskId(1)).unwrap();
    assert_eq!((task.key.as_deref(), task.is_done()), (None, false));
    ctx.complete_task(selfma::TaskId(1)).unwrap();
}
