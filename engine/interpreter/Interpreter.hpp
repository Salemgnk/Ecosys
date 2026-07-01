#ifndef INTERPRETER_HPP
    #define INTERPRETER_HPP
    #include <string>
    #include "Event.hpp"

// Traduit un Event (donnée brute de la simulation) en une phrase anglaise.
// Le moteur ne sait pas écrire d'histoires ; c'est le rôle de l'interpreter.
class Interpreter {
    public:
        std::string narrate(const Event& event) const;
};

#endif
