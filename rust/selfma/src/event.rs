use crate::models::Event;

pub type EventCallback = Box<dyn Fn(&Event) + Send + Sync>;
