//! Selfma is a task management library backed by SQLite.

mod context;
mod event;
mod ids;
mod models;
mod storage;

pub use context::SelfmaContext;
pub use event::EventCallback;
pub use models::{
    Event, EventKind, Project, ProjectId, ProjectSnapshot, Result, SelfmaError, Task, TaskId, Timer,
};
