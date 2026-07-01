#include "Interpreter.hpp"

std::string Interpreter::narrate(const Event& event) const
{
    const std::string& who = event.speciesName;
    const std::string& where = event.zoneName;

    switch (event.type) {
        case EventType::Born:
            return "A new " + who + " organism is born in the " + where + " zone.";
        case EventType::Reproduced:
            return "The " + who + " organism reproduced in the " + where + " zone.";
        case EventType::Migrated:
            return "The " + who + " organism migrated toward the " + where + " zone.";
        case EventType::Competed:
            return "The " + who + " organism fought for resources in the " + where + " zone.";
        case EventType::Thrived:
            return "The " + who + " organism is thriving in the " + where + " zone.";
        case EventType::Starved:
            return "The " + who + " organism starved and died in the " + where + " zone.";
        case EventType::Died:
            return "The " + who + " organism died in the " + where + " zone.";
    }
    return "";
}
