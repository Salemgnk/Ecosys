# Ecosys 🌱💻

> A living interpretation layer for operating systems.

Ecosys transforms a Linux system into a living ecosystem where processes, memory, files, and network activity become organisms interacting inside a dynamic world.

---

## 🌍 Concept

A computer is not just a machine.

It is a living system:

- Processes → Organisms
- CPU usage → Energy
- RAM → Nutrition
- Files → Territory
- Network connections → Migration routes
- Process relationships → Ecological interactions

Ecosys interprets system activity as a biological simulation.

---

## 🧠 Philosophy

Traditional system tools show data.

Ecosys tells stories.

Instead of:

```
PID 1234 firefox CPU 12%
```

You see:

```
The Explorer organism is thriving in a high-energy zone.
It is expanding its influence across the network.
```

---

## 🧱 Architecture

Ecosys is built as a layered engine:

```
apps/
└── cli/                → User interface (terminal commands)

engine/
├── core/               → Fundamental entities (Organism, Event, WorldState)
├── simulation/         → Time system and world evolution (tick engine)
├── ecology/            → Species, relationships, evolution rules
├── observer/           → Linux data ingestion (/proc, system metrics)
└── interpreter/        → System → narrative translation layer

tui/                    → Terminal UI (visual ecosystem view)
web/                    → Localhost dashboard (future visualization layer)
docs/                   → Design notes and ecosystem theory
```

---

## ⚙️ Execution Modes

### 🧪 Simulation Mode

A fully virtual ecosystem with no dependency on the host system.

Used for:

- testing rules
- balancing ecosystem behavior
- prototyping evolution logic

### 🖥️ Observation Mode

Maps real Linux processes into ecosystem entities.

Used for:

- real-time system visualization
- process behavior interpretation
- system monitoring through ecological lens

### 🔁 Hybrid Mode

A mix of simulation + real system influence.

Used for:

- emergent behavior experiments
- adaptive ecosystem evolution

---

## 🚀 Roadmap

- [x] Core engine (Organism, Event, World)
- [x] Tick-based simulation system
- [x] CLI visualization
- [x] Narrative interpreter system
- [x] Linux process observer
- [x] TUI ecosystem viewer
- [x] Web dashboard (localhost)
- [x] Relationship graph engine

---

## 🌿 Vision

Ecosys explores a simple idea:

> What if an operating system could be understood like a living ecosystem instead of a machine?

It aims to make system behavior:

- visible
- interpretable
- and emergent

---

## 🧪 Status

Both execution modes are alive:

- **Simulation** (`./build/ecosys`) — organisms metabolize, compete, migrate,
  reproduce and starve across named zones, narrated tick by tick.
- **Observation** (`./build/ecosys-observe`) — a real-time mirror of your
  Linux system: the top 15 processes by memory become organisms, process
  states become zones (`running`, `sleeping`, `zombie`...), and `/proc` diffs
  become the story — new PID = born, vanished PID = died, state change =
  migration, memory growth = thriving.

```bash
cmake -S . -B build && cmake --build build
./build/ecosys            # simulation mode (plain narration)
./build/ecosys-tui        # simulation mode, full-terminal ecosystem view
./build/ecosys-observe    # observation mode (mirrors /proc)
./build/ecosys-web --observe          # living system monitor at http://localhost:8080
./build/ecosys-web --observe --allow-kill   # …and enable process termination
./build/ecosys-web                    # same view, running the simulation instead
ctest --test-dir build    # run the test suites
```

The TUI (built on [FTXUI](https://github.com/ArthurSonzogni/FTXUI), fetched
automatically by CMake) shows each zone as a panel with its organisms and
their energy bars — a full bar means ready to reproduce — plus a color-coded
event feed.

The web dashboard is a **living system monitor** — `htop` reimagined as an
ecosystem. Every process is a cell in a force-directed constellation:

- **size = memory (RSS)** — the biggest consumers dominate the field
- **glow = CPU** — busy processes pulse and flare; idle ones stay dim
- **hue = state** — running (green), sleeping (blue), zombie (coral)…
- **threads = the process tree (PPID)** — parents wired to children, so
  `systemd`, your shell and the browser process become visible hubs (memory
  ranking keeps the top ~50, and their ancestors are pulled in to keep the
  tree whole)

The left rail is the instrument side: live vitals, a search filter, sortable
columns (memory / cpu / name) and a process list wired to the graph. Click any
cell to **trace its lineage** — its whole ancestry and descendancy light up as
bioluminescent threads while everything else fades to black — and to open an
inspection card (pid, parent, state, memory, cpu, threads, full command line).
With `--allow-kill` the card gains a guarded **Terminate** button (SIGTERM,
two-step confirm). The C++ server (cpp-httplib, fetched by CMake) streams the
whole state as JSON on `/state`; the frontend is a single dependency-free
Canvas page.

---

## 📌 Naming

**Ecosys = Ecosystem + System**
