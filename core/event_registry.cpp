#include "event_registry.h"

#include <yaml-cpp/yaml.h>
#include "qwistys_macros.h"

void EventRegistry::_register(EventDef def) {
    _name_to_id[def.name] = def.id;
    _events.push_back(std::move(def));
}

void EventRegistry::_load_defaults() {
    _events.clear();
    _name_to_id.clear();
    _register({0, "EVENT_MAX_TIME_SLEEP",    "Event loop reached maximum sleep time"});
    _register({1, "TASK_TIME_ELAPSED",       "Task timer completed"});
}

void EventRegistry::load(const std::string& yaml_path) {
    try {
        YAML::Node root = YAML::LoadFile(yaml_path);
        if (!root["events"] || !root["events"].IsSequence()) {
            QWISTYS_ERROR_MSG("EventRegistry: 'events' key missing or not a sequence in %s - using defaults",
                    yaml_path.c_str());
            _load_defaults();
            return;
        }

        _events.clear();
        _name_to_id.clear();

        for (const auto& node : root["events"]) {
            EventDef def;
            def.id          = node["id"].as<uint32_t>();
            def.name        = node["name"].as<std::string>();
            def.description = node["description"] ? node["description"].as<std::string>() : "";
            _register(std::move(def));
        }

        QWISTYS_DEBUG_MSG("EventRegistry: loaded %zu event(s) from %s",
                 _events.size(), yaml_path.c_str());
    } catch (const YAML::Exception& ex) {
        QWISTYS_ERROR_MSG("EventRegistry: failed to parse %s (%s) - using defaults",
                yaml_path.c_str(), ex.what());
        _load_defaults();
    } catch (...) {
        QWISTYS_ERROR_MSG("EventRegistry: unexpected error loading %s - using defaults", yaml_path.c_str());
        _load_defaults();
    }
}

uint32_t EventRegistry::id_for(const std::string& name) const {
    auto it = _name_to_id.find(name);
    if (it != _name_to_id.end()) {
        return it->second;
    }
    return UINT32_MAX;
}

bool EventRegistry::has(uint32_t id) const {
    for (const auto& def : _events) {
        if (def.id == id) return true;
    }
    return false;
}

const std::vector<EventDef>& EventRegistry::all() const {
    return _events;
}
