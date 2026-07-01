#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "Interpreter.hpp"
#include "Observer.hpp"
#include "ProcReader.hpp"

// Mode observation : l'écosystème est un miroir du système Linux.
// Chaque seconde on relit /proc ; les naissances, morts, migrations d'état
// et poussées de mémoire des vrais processus deviennent la narration.
int main()
{
    ProcReader reader(15);
    Observer observer;
    Interpreter interpreter;

    std::cout << "Observing the Linux ecosystem (top 15 organisms by memory). "
                 "Ctrl+C to stop.\n";

    // Premier passage : établit la référence, sans narration.
    observer.observe(reader.snapshot());

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::vector<Event> events = observer.observe(reader.snapshot());
        if (events.empty()) {
            continue;   // un écosystème calme n'a rien à raconter
        }

        for (const Event& event : events) {
            std::cout << "  " << interpreter.narrate(event) << "\n";
        }
        std::cout.flush();
    }

    return 0;
}
