use std::fmt;

use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use thiserror::Error;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]
#[repr(transparent)]
pub struct ProjectId(pub u32);

impl fmt::Display for ProjectId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.0)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]
#[repr(transparent)]
pub struct TaskId(pub u32);

impl fmt::Display for TaskId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.0)
    }
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Project {
    pub id: ProjectId,
    pub name: String,
    pub description: String,
    pub created_at: DateTime<Utc>,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Timer {
    started_at: DateTime<Utc>,
    duration_sec: f64,
}

impl Timer {
    pub fn new(duration_sec: f64) -> Self {
        Self {
            started_at: Utc::now(),
            duration_sec,
        }
    }

    pub fn is_finished(&self) -> bool {
        (Utc::now() - self.started_at).num_milliseconds() as f64 / 1000.0 >= self.duration_sec
    }

    pub fn remaining_sec(&self) -> f64 {
        let elapsed = (Utc::now() - self.started_at).num_milliseconds() as f64 / 1000.0;
        (self.duration_sec - elapsed).max(0.0)
    }
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Task {
    pub id: TaskId,
    pub project_id: ProjectId,
    pub description: String,
    pub duration_sec: f64,
    pub created_at: DateTime<Utc>,
    pub timer: Timer,
}

impl Task {
    pub fn elapsed(&self) -> bool {
        self.timer.is_finished()
    }

    pub fn remaining_sec(&self) -> f64 {
        self.timer.remaining_sec()
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct ProjectSnapshot {
    pub project: Project,
    pub tasks: Vec<Task>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum EventKind {
    MaxTimeSleep,
    TaskTimeElapsed,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Event {
    pub kind: EventKind,
    pub project_id: Option<ProjectId>,
    pub task_id: Option<TaskId>,
}

#[derive(Debug, Error)]
pub enum SelfmaError {
    #[error("sqlite error: {0}")]
    Sql(#[from] rusqlite::Error),
    #[error("io error: {0}")]
    Io(#[from] std::io::Error),
    #[error("project not found: {0}")]
    ProjectNotFound(ProjectId),
    #[error("task not found: {0}")]
    TaskNotFound(TaskId),
    #[error("name must not be empty")]
    EmptyName,
    #[error("description must not be empty")]
    EmptyDescription,
    #[error("duration must be finite and non-negative")]
    InvalidDuration,
    #[error("id space exhausted")]
    IdExhausted,
}

pub type Result<T> = std::result::Result<T, SelfmaError>;
