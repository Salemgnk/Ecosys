# Ecosys — Engine Spec v1 (Design)

Date: 2026-06-30
Statut: validé (design), à implémenter
Langage: C++ / CMake

## Objectif

Construire le **moteur de simulation pur** d'Ecosys : un monde virtuel qui tourne
en ticks, où des organismes naissent, se nourrissent, se reproduisent, se font
concurrence et meurent dans des zones. Le moteur émet des `Event`s structurés ;
un interpreter basique les traduit en phrases anglaises ; un petit programme de
démo imprime la narration.

Hors périmètre v1 : ingestion `/proc` (observer), TUI, dashboard web,
interpreter narratif riche, mode hybride.

## Objectif pédagogique

Ce projet sert à s'entraîner à la **POO de base**. La règle directrice :

> **L'héritage décide du _comportement_ (le `act()` propre à chaque type),
> la composition (`Species`) décide des _chiffres_ (les traits réglables).**

On exerce ainsi en même temps :
- héritage + polymorphisme (classe abstraite `Organism`, méthodes virtuelles) ;
- composition (un organisme _possède_ un `Species`) ;
- encapsulation (champs privés/protégés + accesseurs) ;
- responsabilité unique (chaque classe a un rôle clair).

## Décisions de cadrage

| Sujet | Décision |
|---|---|
| Périmètre | Simulation pure (pas de `/proc`, pas d'UI) |
| Règles de vie | Énergie/métabolisme + mort + reproduction + interactions (compétition) |
| Espace | Zones nommées, chacune avec un rendement d'énergie |
| Sortie | `Event`s structurés + interpreter basique + `main` de démo |
| Style code | POO : héritage **et** composition |
| Sous-classes v1 | `Explorer`, `Grazer` (2). `Predator` repoussé (compétition générique suffit) |
| Partage des traits | `std::shared_ptr<const Species>` (pas de dangling) |
| Déterminisme | RNG à graine fixe (`std::mt19937`) dans `World` |
| Tests | Exécutable de tests par `assert`, sans dépendance externe |

## Architecture

```
apps/
└── cli/
    └── main.cpp            → démo : crée le monde, lance N ticks, imprime la narration

engine/
├── core/
│   ├── Organism.hpp/.cpp   → classe abstraite (énergie, vie/mort, métabolisme)
│   └── Event.hpp           → objet-valeur décrivant un évènement
├── ecology/
│   ├── Species.hpp/.cpp    → traits réglables (composition)
│   ├── Explorer.hpp/.cpp   → sous-classe : migre vers les zones riches
│   └── Grazer.hpp/.cpp     → sous-classe : broute sur place
├── simulation/
│   ├── Zone.hpp/.cpp       → lieu nommé, rendement d'énergie partagé
│   └── World.hpp/.cpp      → agrégat racine, orchestre tick()
└── interpreter/
    └── Interpreter.hpp/.cpp → Event → phrase anglaise

tests/
└── engine_tests.cpp        → assertions sur les règles du moteur
```

CMake : une bibliothèque `ecosys_engine` (core + ecology + simulation + interpreter),
un exécutable `ecosys` (apps/cli), un exécutable `engine_tests` (tests).

## Classes et responsabilités

### `Species` (ecology/) — composition

Données de traits, partagée et immuable. Aucun comportement décisionnel.

Champs :
- `name` (string) — ex. « Explorer »
- `metabolicCost` (double) — énergie perdue par tick
- `reproductionThreshold` (double) — énergie nécessaire pour se reproduire
- `reproductionCost` (double) — énergie cédée à l'enfant / perdue par le parent
- `aggressiveness` (double, 0..1) — poids dans la répartition de l'énergie en zone

### `Organism` (core/) — classe abstraite, cœur du polymorphisme

```cpp
class Organism {
protected:
    double energy_;
    bool alive_ = true;
    std::shared_ptr<const Species> species_;   // composition
    std::string zoneName_;                      // handle stable vers la zone
public:
    Organism(std::shared_ptr<const Species> sp, double startEnergy, std::string zone);
    virtual ~Organism() = default;              // destructeur virtuel (obligatoire)

    // Comportement propre à chaque type :
    virtual void act(World& world, std::vector<Event>& out) = 0;   // pure virtual
    virtual std::unique_ptr<Organism> reproduce() = 0;             // enfant du même type

    // Comportement commun, non virtuel :
    void metabolize();           // energy_ -= species_->metabolicCost
    void feed(double amount);
    bool isAlive() const;
    bool canReproduce() const;   // energy_ >= species_->reproductionThreshold

    // accesseurs
    double energy() const;
    const Species& species() const;
    const std::string& zoneName() const;
    void setZone(std::string zone);
};
```

Notions exercées : méthode purement virtuelle (`= 0`) → classe abstraite ;
destructeur **virtuel** (destruction via pointeur de base) ; séparation
protégé/public.

### `Explorer`, `Grazer` (ecology/) — héritage / polymorphisme

Chacune redéfinit (`override`) `act()` et `reproduce()` :
- `Explorer::act` — s'il existe une zone au rendement supérieur à sa zone
  courante, il migre (`Migrated`) ; sinon il se nourrit sur place.
- `Grazer::act` — se nourrit sur place ; espèce à `metabolicCost` plus faible
  (émet `Thrived` quand il gagne de l'énergie).
- `reproduce()` renvoie un `std::unique_ptr<Organism>` du **bon type concret**
  (covariance de comportement via polymorphisme).

### `Zone` (simulation/)

Lieu nommé avec un rendement d'énergie partagé entre ses occupants.

Champs : `name` (string), `energyYield` (double, énergie disponible par tick).
Méthodes : accesseurs ; la répartition entre occupants est calculée par `World`
(la `Zone` ne connaît pas la liste des organismes → couplage faible).

### `Event` (core/) — objet-valeur

```cpp
enum class EventType { Born, Died, Reproduced, Migrated, Competed, Thrived, Starved };

struct Event {
    EventType type;
    std::string speciesName;   // acteur principal
    std::string zoneName;
    std::string otherSpecies;  // optionnel (compétition)
};
```

Donnée pure, sans logique de formatage. Le moteur ne sait pas écrire de phrases.

### `World` (simulation/) — agrégat racine

Possède :
- `std::vector<std::unique_ptr<Organism>>` — possession **polymorphe**.
- `std::map<std::string, Zone>` — zones résolues par nom.
- `std::mt19937` — RNG à **graine fixe** → simulation déterministe.
- compteur de ticks.

API :
- `void addZone(Zone z);`
- `void addOrganism(std::unique_ptr<Organism> o);`
- `std::vector<Event> tick();` — exécute un pas, renvoie les évènements du tick.
- requêtes utilisées par `act()` : `const Zone* richestZoneOtherThan(const std::string&) const;`,
  `std::vector<Organism*> organismsInZone(const std::string&) const;`, `std::mt19937& rng();`

### `Interpreter` (interpreter/)

`std::string narrate(const Event&) const;` — `switch` sur `EventType` → phrase
anglaise. Ex. `Migrated` → « The Explorer organism migrated toward a high-energy zone. »

## Le pipeline `World::tick()`

Quatre phases ordonnées ; chacune empile des `Event`s :

1. **Métabolisme** — chaque organisme `metabolize()`.
2. **Action** (polymorphe) — chaque `organism->act(world, events)` : migration,
   alimentation, et compétition générique (si plusieurs organismes partagent une
   zone, le rendement se répartit au prorata de `aggressiveness` → `Competed`).
3. **Reproduction** — tout organisme `canReproduce()` appelle `reproduce()`
   (enfant du bon type), parent paie `reproductionCost`. Deux évènements : le
   parent émet `Reproduced`, l'enfant émet `Born`.
4. **Mort** — les organismes à `energy <= 0` sont retirés. L'évènement émis est
   `Starved` (mort par épuisement d'énergie, seul cas de mort en v1) ; `Died`
   est réservé aux causes futures (prédation, vieillesse).

Ordre figé pour la reproductibilité. L'aléa (départage, choix de migration en cas
d'égalité) passe par `World::rng()` à graine fixe.

## Modèle de possession (mémoire)

- `World` possède les organismes (`unique_ptr`) et les zones (par valeur dans la map).
- Un organisme **référence** sa zone par **nom** (string) — pas de pointeur vers
  la `Zone` (évite le dangling si la map est modifiée).
- Un organisme **partage** son `Species` via `shared_ptr<const Species>` — immuable,
  partagé entre tous les individus de l'espèce, jamais dangling.

## Stratégie de test

Exécutable `engine_tests` sans dépendance externe (`assert` + `main`), couvrant :
- un organisme à énergie 0 est retiré au tick suivant (mort) ;
- un organisme au-dessus du seuil produit exactement un enfant du bon type (repro) ;
- le métabolisme décroît l'énergie de `metabolicCost` par tick ;
- une compétition à deux dans une zone répartit l'énergie selon `aggressiveness` ;
- déterminisme : deux `World` à même graine + même population produisent la même
  suite d'évènements sur N ticks.

Catch2 envisageable plus tard ; v1 reste sans dépendance pour minimiser la friction.

## Évolutions futures (hors v1)

- `Predator` comme nouvelle sous-classe (le design héritage rend l'ajout trivial).
- Adjacence réelle entre zones (graphe de migration).
- Observer Linux (`/proc`) alimentant le monde en mode observation.
- Interpreter narratif riche (variations, ton, regroupement d'évènements).
