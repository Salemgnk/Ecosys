#include "Observer.hpp"

std::vector<Event> Observer::observe(const std::vector<ProcessInfo>& snapshot)
{
    std::vector<Event> events;

    std::map<int, ProcessInfo> current;
    for (const ProcessInfo& proc : snapshot) {
        current.emplace(proc.pid, proc);
    }

    if (hasBaseline_) {
        // Nouveaux et changements : on parcourt le présent.
        for (const auto& [pid, proc] : current) {
            const auto before = previous_.find(pid);
            if (before == previous_.end()) {
                events.push_back(Event{EventType::Born, proc.name, proc.zoneName, ""});
                continue;
            }
            if (before->second.zoneName != proc.zoneName) {
                events.push_back(Event{EventType::Migrated, proc.name, proc.zoneName, ""});
            }
            if (before->second.energy > 0.0 &&
                proc.energy > before->second.energy * (1.0 + kGrowthThreshold)) {
                events.push_back(Event{EventType::Thrived, proc.name, proc.zoneName, ""});
            }
        }

        // Disparus : on parcourt le passé. On narre leur mort dans la
        // dernière zone où on les a vus.
        for (const auto& [pid, proc] : previous_) {
            if (current.find(pid) == current.end()) {
                events.push_back(Event{EventType::Died, proc.name, proc.zoneName, ""});
            }
        }
    }

    previous_ = std::move(current);
    hasBaseline_ = true;
    return events;
}

std::size_t Observer::population() const
{
    return previous_.size();
}
