use std::path::Path;

use chrono::DateTime;
use rusqlite::Connection;

use crate::models::{Project, ProjectId, Result, Task, TaskId, Timer};

const SCHEMA_V1: &str = r#"
CREATE TABLE IF NOT EXISTS schema_meta (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS projects (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT NOT NULL,
    created_at INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS tasks (
    id INTEGER PRIMARY KEY,
    project_id INTEGER NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
    description TEXT NOT NULL,
    duration_sec REAL NOT NULL,
    created_at INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_tasks_project ON tasks(project_id);
"#;

pub struct Store {
    conn: Connection,
}

impl Store {
    pub fn open(path: &Path) -> Result<Self> {
        let conn = Connection::open(path)?;
        conn.pragma_update(None, "foreign_keys", true)?;
        conn.pragma_update(None, "journal_mode", "WAL")?;
        conn.execute_batch(SCHEMA_V1)?;
        Self::write_schema_version(&conn)?;
        Ok(Self { conn })
    }

    #[cfg(test)]
    pub fn connect(conn: Connection) -> Result<Self> {
        conn.pragma_update(None, "foreign_keys", true)?;
        conn.pragma_update(None, "journal_mode", "WAL")?;
        conn.execute_batch(SCHEMA_V1)?;
        Self::write_schema_version(&conn)?;
        Ok(Self { conn })
    }

    fn write_schema_version(conn: &Connection) -> Result<()> {
        conn.execute(
            r#"INSERT INTO schema_meta (key, value) VALUES ('version', '1')
               ON CONFLICT(key) DO UPDATE SET value = excluded.value"#,
            [],
        )?;
        Ok(())
    }

    pub fn insert_project(&self, project: &Project) -> Result<()> {
        self.conn.execute(
            r#"INSERT INTO projects (id, name, description, created_at)
                   VALUES (?1, ?2, ?3, ?4)"#,
            rusqlite::params![
                project.id.0,
                project.name,
                project.description,
                project.created_at.timestamp()
            ],
        )?;
        Ok(())
    }

    pub fn insert_task(&self, task: &Task) -> Result<()> {
        self.conn.execute(
            r#"INSERT INTO tasks (id, project_id, description, duration_sec, created_at)
                   VALUES (?1, ?2, ?3, ?4, ?5)"#,
            rusqlite::params![
                task.id.0,
                task.project_id.0,
                task.description,
                task.duration_sec,
                task.created_at.timestamp()
            ],
        )?;
        Ok(())
    }

    pub fn delete_project(&self, project_id: ProjectId) -> Result<bool> {
        let affected = self
            .conn
            .execute("DELETE FROM projects WHERE id = ?1", [project_id.0])?;
        Ok(affected > 0)
    }

    pub fn delete_task(&self, project_id: ProjectId, task_id: TaskId) -> Result<bool> {
        let affected = self.conn.execute(
            "DELETE FROM tasks WHERE id = ?1 AND project_id = ?2",
            rusqlite::params![task_id.0, project_id.0],
        )?;
        Ok(affected > 0)
    }

    pub fn load_projects(&self) -> Result<Vec<Project>> {
        let mut stmt = self
            .conn
            .prepare("SELECT id, name, description, created_at FROM projects ORDER BY id")?;
        let rows = stmt.query_map([], row_to_project)?;
        Ok(rows.collect::<std::result::Result<Vec<_>, _>>()?)
    }

    pub fn load_tasks(&self) -> Result<Vec<Task>> {
        let mut stmt = self.conn.prepare(
            "SELECT id, project_id, description, duration_sec, created_at FROM tasks ORDER BY id",
        )?;
        let rows = stmt.query_map([], row_to_task)?;
        Ok(rows.collect::<std::result::Result<Vec<_>, _>>()?)
    }

    pub fn save_next_ids(&self, next_project_id: u32, next_task_id: u32) -> Result<()> {
        self.save_meta("next_project_id", &next_project_id.to_string())?;
        self.save_meta("next_task_id", &next_task_id.to_string())?;
        Ok(())
    }

    fn save_meta(&self, key: &str, value: &str) -> Result<()> {
        self.conn.execute(
            r#"INSERT INTO schema_meta (key, value) VALUES (?1, ?2)
               ON CONFLICT(key) DO UPDATE SET value = excluded.value"#,
            rusqlite::params![key, value],
        )?;
        Ok(())
    }
}

fn row_to_project(row: &rusqlite::Row<'_>) -> rusqlite::Result<Project> {
    Ok(Project {
        id: ProjectId(row.get(0)?),
        name: row.get(1)?,
        description: row.get(2)?,
        created_at: DateTime::from_timestamp(row.get(3)?, 0)
            .unwrap_or_else(|| DateTime::from_timestamp(0, 0).unwrap()),
    })
}

fn row_to_task(row: &rusqlite::Row<'_>) -> rusqlite::Result<Task> {
    Ok(Task {
        id: TaskId(row.get(0)?),
        project_id: ProjectId(row.get(1)?),
        description: row.get(2)?,
        duration_sec: row.get(3)?,
        created_at: DateTime::from_timestamp(row.get(4)?, 0)
            .unwrap_or_else(|| DateTime::from_timestamp(0, 0).unwrap()),
        timer: Timer::new(row.get(3)?),
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::models::*;
    use chrono::Utc;

    fn make_store() -> Store {
        let conn = Connection::open_in_memory().unwrap();
        Store::connect(conn).unwrap()
    }

    fn sample_project(id: u32) -> Project {
        Project {
            id: ProjectId(id),
            name: "demo".into(),
            description: "a project".into(),
            created_at: Utc::now(),
        }
    }

    fn sample_task(project: ProjectId, id: u32) -> Task {
        Task {
            id: TaskId(id),
            project_id: project,
            description: "a task".into(),
            duration_sec: 30.0,
            created_at: Utc::now(),
            timer: Timer::new(30.0),
        }
    }

    fn sample_project_at(id: u32, ts: chrono::DateTime<Utc>) -> Project {
        Project {
            id: ProjectId(id),
            name: "demo".into(),
            description: "a project".into(),
            created_at: ts,
        }
    }

    fn truncated_to_sec(ts: chrono::DateTime<Utc>) -> chrono::DateTime<Utc> {
        chrono::DateTime::from_timestamp(ts.timestamp(), 0).unwrap()
    }

    #[test]
    fn insert_and_load_roundtrip() {
        let store = make_store();
        let now = Utc::now();
        let project = sample_project_at(1, now);
        let task = Task {
            created_at: now,
            ..sample_task(project.id, 1)
        };
        store.insert_project(&project).unwrap();
        store.insert_task(&task).unwrap();

        let projects = store.load_projects().unwrap();
        let tasks = store.load_tasks().unwrap();

        let expected_project = Project {
            created_at: truncated_to_sec(now),
            ..project
        };
        let expected_task = Task {
            created_at: truncated_to_sec(now),
            timer: tasks[0].timer.clone(),
            ..task
        };
        assert_eq!(projects, vec![expected_project]);
        assert_eq!(tasks, vec![expected_task]);
    }

    #[test]
    fn delete_cascades_tasks() {
        let store = make_store();
        store.insert_project(&sample_project(1)).unwrap();
        store.insert_task(&sample_task(ProjectId(1), 1)).unwrap();
        assert!(store.delete_project(ProjectId(1)).unwrap());
        assert!(store.load_tasks().unwrap().is_empty());
        assert!(!store.delete_project(ProjectId(1)).unwrap());
    }

    #[test]
    fn rejects_task_for_missing_project() {
        let store = make_store();
        let err = store
            .insert_task(&sample_task(ProjectId(99), 1))
            .unwrap_err();
        assert!(matches!(err, SelfmaError::Sql(_)));
    }
}
