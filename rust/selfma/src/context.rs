use std::collections::BTreeMap;
use std::path::Path;

use chrono::Utc;

use crate::event::EventCallback;
use crate::ids::IdPool;
use crate::models::{
    Event, EventKind, Project, ProjectId, ProjectSnapshot, Result, SelfmaError, Task, TaskId, Timer,
};
use crate::storage::Store;

pub struct SelfmaContext {
    store: Store,
    projects: BTreeMap<ProjectId, Project>,
    tasks: BTreeMap<TaskId, Task>,
    project_ids: IdPool,
    task_ids: IdPool,
    handlers: Vec<EventCallback>,
}

impl SelfmaContext {
    pub fn new(path: &Path) -> Result<Self> {
        let store = Store::open(path)?;
        Ok(Self {
            store,
            projects: BTreeMap::new(),
            tasks: BTreeMap::new(),
            project_ids: IdPool::new(u32::MAX),
            task_ids: IdPool::new(u32::MAX),
            handlers: Vec::new(),
        })
    }

    pub fn add_project(&mut self, name: &str, description: &str) -> Result<ProjectId> {
        validate_name(name).and_then(|_| validate_description(description))?;
        let id = ProjectId(self.project_ids.allocate()?);
        let project = Project {
            id,
            name: name.to_string(),
            description: description.to_string(),
            created_at: Utc::now(),
        };
        self.store.insert_project(&project)?;
        self.projects.insert(id, project);
        Ok(id)
    }

    pub fn remove_project(&mut self, project_id: ProjectId) -> Result<()> {
        if !self.projects.contains_key(&project_id) {
            return Err(SelfmaError::ProjectNotFound(project_id));
        }
        self.store.delete_project(project_id)?;
        self.projects.remove(&project_id);
        let finished: Vec<TaskId> = self
            .tasks
            .values()
            .filter(|task| task.project_id == project_id)
            .map(|task| task.id)
            .collect();
        for task_id in finished {
            self.tasks.remove(&task_id);
            self.task_ids.release(task_id.0);
        }
        self.project_ids.release(project_id.0);
        Ok(())
    }

    pub fn add_task(
        &mut self,
        project_id: ProjectId,
        description: &str,
        duration_sec: f64,
    ) -> Result<TaskId> {
        if !self.projects.contains_key(&project_id) {
            return Err(SelfmaError::ProjectNotFound(project_id));
        }
        validate_description(description)?;
        validate_duration(duration_sec)?;
        let id = TaskId(self.task_ids.allocate()?);
        let task = Task {
            id,
            project_id,
            description: description.to_string(),
            duration_sec,
            created_at: Utc::now(),
            timer: Timer::new(duration_sec),
        };
        self.store.insert_task(&task)?;
        self.tasks.insert(id, task);
        Ok(id)
    }

    pub fn remove_task(&mut self, project_id: ProjectId, task_id: TaskId) -> Result<()> {
        let task = self
            .tasks
            .get(&task_id)
            .ok_or(SelfmaError::TaskNotFound(task_id))?;
        if task.project_id != project_id {
            return Err(SelfmaError::TaskNotFound(task_id));
        }
        self.store.delete_task(project_id, task_id)?;
        self.tasks.remove(&task_id);
        self.task_ids.release(task_id.0);
        Ok(())
    }

    pub fn get_project(&self, project_id: ProjectId) -> Option<ProjectSnapshot> {
        let project = self.projects.get(&project_id)?.clone();
        let tasks: Vec<Task> = self
            .tasks
            .values()
            .filter(|task| task.project_id == project_id)
            .cloned()
            .collect();
        Some(ProjectSnapshot { project, tasks })
    }

    pub fn list_projects(&self) -> Vec<&Project> {
        self.projects.values().collect()
    }

    pub fn get_task(&self, task_id: TaskId) -> Option<&Task> {
        self.tasks.get(&task_id)
    }

    pub fn serialize(&self) -> Result<()> {
        self.store
            .save_next_ids(self.project_ids.next_value(), self.task_ids.next_value())?;
        Ok(())
    }

    pub fn deserialize(&mut self) -> Result<()> {
        let projects = self.store.load_projects()?;
        let tasks = self.store.load_tasks()?;
        self.projects = projects.into_iter().map(|p| (p.id, p)).collect();
        self.tasks = tasks.into_iter().map(|t| (t.id, t)).collect();
        let next_project = self
            .projects
            .keys()
            .map(|id| id.0)
            .max()
            .unwrap_or(0)
            .saturating_add(1);
        let next_task = self
            .tasks
            .keys()
            .map(|id| id.0)
            .max()
            .unwrap_or(0)
            .saturating_add(1);
        self.project_ids.reseed(next_project);
        self.task_ids.reseed(next_task);
        Ok(())
    }

    pub fn on_event(&mut self, callback: EventCallback) {
        self.handlers.push(callback);
    }

    pub fn update(&mut self) -> Result<()> {
        let finished: Vec<TaskId> = self
            .tasks
            .iter()
            .filter(|(_, task)| task.elapsed())
            .map(|(id, _)| *id)
            .collect();
        for task_id in finished {
            let task = self
                .tasks
                .get(&task_id)
                .cloned()
                .ok_or(SelfmaError::TaskNotFound(task_id))?;
            self.store.delete_task(task.project_id, task_id)?;
            self.tasks.remove(&task_id);
            self.task_ids.release(task_id.0);
            let event = Event {
                kind: EventKind::TaskTimeElapsed,
                project_id: Some(task.project_id),
                task_id: Some(task_id),
            };
            self.dispatch(&event);
        }
        Ok(())
    }

    fn dispatch(&self, event: &Event) {
        for handler in &self.handlers {
            handler(event);
        }
    }
}

fn validate_name(name: &str) -> Result<()> {
    if name.is_empty() {
        return Err(SelfmaError::EmptyName);
    }
    Ok(())
}

fn validate_description(description: &str) -> Result<()> {
    if description.is_empty() {
        return Err(SelfmaError::EmptyDescription);
    }
    Ok(())
}

fn validate_duration(duration_sec: f64) -> Result<()> {
    if !duration_sec.is_finite() || duration_sec < 0.0 {
        return Err(SelfmaError::InvalidDuration);
    }
    Ok(())
}
