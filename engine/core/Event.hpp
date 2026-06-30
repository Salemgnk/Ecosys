#ifndef EVENT_HPP
    #define EVENT_HPP
    #include <string>

enum class EventType {
    Born,
    Died,
    Reproduced,
    Migrated,
    Competed,
    Thrived,
    Starved,
};

struct Event {
    EventType type;
    std::string speciesName;
    std::string zoneName;
    std::string otherSpecies;
};
#endif