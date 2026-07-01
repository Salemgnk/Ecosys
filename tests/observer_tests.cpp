#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "Observer.hpp"
#include "ProcReader.hpp"

static int g_failures = 0;
#define REQUIRE(cond)                                                       \
    do {                                                                    \
        if (!(cond)) {                                                      \
            std::cerr << "FAILED: " #cond " (line " << __LINE__ << ")\n";   \
            ++g_failures;                                                   \
        }                                                                   \
    } while (0)

static bool hasEvent(const std::vector<Event>& events, EventType type,
                     const std::string& who, const std::string& where)
{
    return std::any_of(events.begin(), events.end(), [&](const Event& e) {
        return e.type == type && e.speciesName == who && e.zoneName == where;
    });
}

static ProcessInfo proc(int pid, std::string name, std::string zone, double energy)
{
    return ProcessInfo{pid, std::move(name), std::move(zone), energy};
}

static void test_first_snapshot_is_silent_baseline()
{
    Observer obs;
    std::vector<Event> events = obs.observe({proc(1, "systemd", "sleeping", 10.0)});
    REQUIRE(events.empty());
    REQUIRE(obs.population() == 1);
}

static void test_new_pid_is_born()
{
    Observer obs;
    obs.observe({proc(1, "systemd", "sleeping", 10.0)});
    std::vector<Event> events = obs.observe({
        proc(1, "systemd", "sleeping", 10.0),
        proc(42, "firefox", "running", 500.0),
    });
    REQUIRE(hasEvent(events, EventType::Born, "firefox", "running"));
    REQUIRE(events.size() == 1);
}

static void test_missing_pid_died_in_last_known_zone()
{
    Observer obs;
    obs.observe({proc(42, "firefox", "running", 500.0)});
    std::vector<Event> events = obs.observe({});
    REQUIRE(hasEvent(events, EventType::Died, "firefox", "running"));
    REQUIRE(obs.population() == 0);
}

static void test_state_change_is_migration()
{
    Observer obs;
    obs.observe({proc(42, "firefox", "running", 500.0)});
    std::vector<Event> events = obs.observe({proc(42, "firefox", "sleeping", 500.0)});
    REQUIRE(hasEvent(events, EventType::Migrated, "firefox", "sleeping"));
}

static void test_memory_growth_is_thriving()
{
    Observer obs;
    obs.observe({proc(42, "firefox", "running", 100.0)});
    // +3 % : sous le seuil, silencieux.
    REQUIRE(obs.observe({proc(42, "firefox", "running", 103.0)}).empty());
    // +10 % : narré.
    std::vector<Event> events = obs.observe({proc(42, "firefox", "running", 113.3)});
    REQUIRE(hasEvent(events, EventType::Thrived, "firefox", "running"));
}

static void test_parse_stat_line_with_tricky_comm()
{
    // comm contenant espaces et parenthèses, comme les processus Firefox.
    const std::string line =
        "1234 (Web (Content)) S 1 1234 1234 0 -1 4194304 100 0 0 0 "
        "5 3 0 0 20 0 4 0 12345 104857600 2560 18446744073709551615 "
        "0 0 0 0 0 0 0 0 0 0 0 0 17 3 0 0 0 0 0";
    ProcessInfo info;
    REQUIRE(ProcReader::parseStatLine(line, info));
    REQUIRE(info.pid == 1234);
    REQUIRE(info.name == "Web (Content)");
    REQUIRE(info.zoneName == "sleeping");
    REQUIRE(info.energy > 0.0);   // 2560 pages -> une valeur en MiB
}

static void test_parse_rejects_garbage()
{
    ProcessInfo info;
    REQUIRE(!ProcReader::parseStatLine("", info));
    REQUIRE(!ProcReader::parseStatLine("not a stat line", info));
}

int main()
{
    test_first_snapshot_is_silent_baseline();
    test_new_pid_is_born();
    test_missing_pid_died_in_last_known_zone();
    test_state_change_is_migration();
    test_memory_growth_is_thriving();
    test_parse_stat_line_with_tricky_comm();
    test_parse_rejects_garbage();

    if (g_failures > 0) {
        std::cerr << g_failures << " test(s) failed.\n";
        return 1;
    }
    std::cout << "All observer tests passed.\n";
    return 0;
}
