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
