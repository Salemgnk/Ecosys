# Ecosys — God Game (design)

Date: 2026-07-02
Statut: proposé, à valider
Cible: transformer le **mode simulation** en jeu « dieu de l'écosystème »

## Vision

Vue du dessus (la vue solaire actuelle). Le joueur est le **dieu** d'un
écosystème virtuel : il ensemence des espèces, façonne les biomes, déclenche
des évènements, et regarde une **chaîne alimentaire** et l'**évolution** se
dérouler. Objectif : garder l'écosystème vivant et divers le plus longtemps.

L'observation de `/proc` reste inchangée (moniteur). Le jeu, c'est la simulation.

## 1. Chaîne alimentaire (enrichir l'écosystème)

Trois niveaux :

```
énergie de la zone  ──►  herbivores (Grazer, Explorer)  ──►  Predator
```

- **Grazer / Explorer** : herbivores, se nourrissent de l'énergie de leur zone
  (mécanique actuelle).
- **Predator** (nouvel archétype) : à chaque tick, dans sa zone, il cible une
  proie herbivore (la plus faible, départage aléatoire seedé), la **mord** :
  le prédateur gagne `min(proie.énergie, morsure)`, la proie perd d'autant ;
  à 0 la proie meurt (`Died`, « chassée »). Coût métabolique plus élevé, pas de
  broutage. S'il n'y a pas de proie, il jeûne.

Émergence attendue : trop de prédateurs → proies s'effondrent → prédateurs
meurent → proies repartent (cycle proie-prédateur de Lotka-Volterra).

## 2. Évolution / génétique

Aujourd'hui les traits sont dans un `Species` **partagé**. Pour l'évolution, on
donne à chaque organisme son **génome** propre :

- `Genome { metabolicCost, reproductionThreshold, reproductionCost,
  aggressiveness, biteSize (prédateur) }`.
- Un `Species` devient l'**archétype** : un nom + un génome de départ pour
  l'ensemencement.
- À la reproduction : `enfant.genome = parent.genome` avec **mutation** (chaque
  trait ± petit bruit gaussien, borné). Les lignées dérivent.
- Les méthodes (`metabolize`, `canReproduce`, répartition d'énergie) lisent le
  **génome de l'organisme**, plus le `Species` partagé.

Émergence attendue : sélection naturelle — dans un biome pauvre, les lignées à
faible coût métabolique survivent ; face aux prédateurs, l'agressivité/vitesse
sélectionne, etc.

*Refactor moteur clé* : déplacer les traits de `Species` vers un `Genome`
porté par `Organism`.

## 3. Monde plus grand et vivant

- 5 biomes aux rendements variés (ex. forest 20, plains 14, shoreline 10,
  savanna 7, desert 4) + éventuellement une **capacité de charge**.
- Paramètres réglés pour des **dynamiques** (vagues, effondrements, reprises)
  plutôt qu'un équilibre plat.

## 4. Pouvoirs divins (la boucle de jeu)

Barre d'outils « pouvoirs » dans la vue web ; chaque pouvoir = un endpoint POST
appliqué par la boucle de simulation :

- **Ensemencer** : choisir une espèce + une zone → y faire naître quelques
  organismes.
- **Bénir une zone** : augmenter son rendement pour un temps.
- **Assécher une zone** : baisser son rendement.
- **Foudroyer** : tuer une partie des organismes d'une zone.
- **Évènement global** : sécheresse / abondance temporaire sur tout le monde.

Le « curseur divin » : cliquer une zone applique le pouvoir sélectionné.

## 5. Objectifs / score

- **Compteur de générations** (profondeur de lignée max, ou ticks).
- **Biodiversité** : nb d'espèces présentes + variance des génomes (des lignées
  distinctes émergent).
- **Population** par espèce.
- **Score** = générations survécues × biodiversité. **Défaite** = extinction
  totale. Mode sans fin (on joue pour le meilleur score) + éventuels défis
  (« garder les 3 espèces vivantes 200 générations »).

HUD : score, génération, biodiversité, populations par espèce.

## Ce qui change côté architecture

- Moteur (C++) : `Genome` sur `Organism` + mutation ; archétype `Predator` ;
  prédation dans `World::tick()` ; monde plus grand ; hooks pour les pouvoirs
  (addOrganism, setZoneYield, cullZone…) et compteurs (générations).
- Serveur web : endpoints POST des pouvoirs ; le `/state` expose le score, la
  génération, la biodiversité, l'espèce de chaque organisme.
- Frontend : barre de pouvoirs + curseur divin, HUD de score, couleurs par
  espèce (Grazer/Explorer/Predator), fiche organisme montrant son génome.

## Plan par phases

1. **Écosystème vivant** : `Predator` + chaîne alimentaire, `Genome` +
   mutation, monde à 5 biomes. (moteur)
2. **Pouvoirs divins** : endpoints + barre d'outils + curseur. (serveur + UI)
3. **Objectifs** : générations, biodiversité, score, HUD, défaite/extinction.

Chaque phase est jouable/observable et livrée séparément.
