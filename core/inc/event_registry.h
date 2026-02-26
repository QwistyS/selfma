#ifndef SELFMA_EVENT_REGISTRY_H
#define SELFMA_EVENT_REGISTRY_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct EventDef {
    uint32_t    id;
    std::string name;
    std::string description;
};

class EventRegistry {
public:
    // Load from YAML; falls back to built-in defaults if file absent or malformed.
    void load(const std::string& yaml_path);

    // Returns the numeric ID for a named event. Returns UINT32_MAX if not found.
    uint32_t id_for(const std::string& name) const;

    bool has(uint32_t id) const;

    const std::vector<EventDef>& all() const;

private:
    std::vector<EventDef>                  _events;
    std::unordered_map<std::string, uint32_t> _name_to_id;

    void _load_defaults();
    void _register(EventDef def);
};

#endif  // SELFMA_EVENT_REGISTRY_H
