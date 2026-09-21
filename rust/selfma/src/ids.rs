use std::collections::HashSet;

use crate::models::{Result, SelfmaError};

pub struct IdPool {
    next: u32,
    freed: HashSet<u32>,
    max: u32,
}

impl IdPool {
    pub fn new(max: u32) -> Self {
        Self {
            next: 0,
            freed: HashSet::new(),
            max,
        }
    }

    pub fn allocate(&mut self) -> Result<u32> {
        if let Some(id) = self.freed.iter().copied().min() {
            self.freed.remove(&id);
            return Ok(id);
        }
        if self.next >= self.max {
            return Err(SelfmaError::IdExhausted);
        }
        let id = self.next;
        self.next += 1;
        Ok(id)
    }

    pub fn release(&mut self, id: u32) -> bool {
        if id >= self.next || self.freed.contains(&id) {
            return false;
        }
        self.freed.insert(id);
        true
    }

    pub fn reseed(&mut self, next: u32) {
        self.freed.clear();
        self.next = next.min(self.max);
    }

    pub fn next_value(&self) -> u32 {
        self.next
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn allocates_consecutive_ids() {
        let mut pool = IdPool::new(1024);
        assert_eq!(pool.allocate().unwrap(), 0);
        assert_eq!(pool.allocate().unwrap(), 1);
        assert_eq!(pool.allocate().unwrap(), 2);
    }

    #[test]
    fn reuses_released_ids() {
        let mut pool = IdPool::new(1024);
        let id = pool.allocate().unwrap();
        assert!(pool.release(id));
        assert_eq!(pool.allocate().unwrap(), id);
    }

    #[test]
    fn refuses_unknown_release() {
        let mut pool = IdPool::new(1024);
        assert!(!pool.release(99));
    }

    #[test]
    fn exhausts() {
        let mut pool = IdPool::new(2);
        assert_eq!(pool.allocate().unwrap(), 0);
        assert_eq!(pool.allocate().unwrap(), 1);
        assert!(matches!(pool.allocate(), Err(SelfmaError::IdExhausted)));
    }
}
