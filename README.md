# Deterministic Finite Automata (DFA) System

A theory-of-computation tool that automates two tasks: the **formal validity
analysis** of a Deterministic Finite Automaton and the **union operation**
between two valid DFAs, producing the product automaton that recognizes the
union language.

Implemented in C++17, with every data structure and search algorithm written
from scratch.

> **Where the code lives.** The core — data structures, validation and union
> algorithms — is C++17: `include/`, `src/` and the C++ files in `server/`.
> `server/static/` (HTML/CSS/JS) and the PowerShell scripts in `server/` are a
> thin demo and test layer around that core, which is why they are excluded
> from GitHub's language statistics (see [`.gitattributes`](.gitattributes)).
>
> Identifiers (`ArregloDinamico`, `Transicion`, `agregar`, ...) and the
> program's own output (menus, validation messages) are in Spanish. They are
> quoted verbatim in this document so they match what you see on screen.

---

## Table of contents

1. [Overview](#1-overview)
2. [Technical constraint: custom data structures](#2-technical-constraint-custom-data-structures)
3. [Architecture and project structure](#3-architecture-and-project-structure)
4. [Data model](#4-data-model)
5. [Module 1 — Strict DFA validation](#5-module-1--strict-dfa-validation)
6. [Module 2 — Automata union](#6-module-2--automata-union)
7. [Output and string testing](#7-output-and-string-testing)
8. [Build and run](#8-build-and-run)
9. [Usage guide: console menu](#9-usage-guide-console-menu)
10. [Usage guide: visual interface](#10-usage-guide-visual-interface)
11. [Custom algorithms and complexity](#11-custom-algorithms-and-complexity)
12. [Tests](#12-tests)
13. [Design decisions](#13-design-decisions)
14. [Scope](#14-scope)

---

## 1. Overview

A DFA is formally defined as the 5-tuple **M = (Q, Σ, δ, q₀, F)**, where Q is
the set of states, Σ the alphabet, δ: Q × Σ → Q the transition function, q₀
the initial state and F ⊆ Q the set of accepting states.

The system lets you:

1. **Define** automata by providing states, alphabet, initial state,
   accepting states and transitions.
2. **Validate** strictly that the definition satisfies the properties of a
   DFA, with a specific error report. An automaton that fails validation is
   not stored and is not available for later operations.
3. **Unite** two valid DFAs through the Cartesian product of their states,
   with acceptance by disjunction.
4. **Visualize** the result as a transition table, an explicit listing of
   components and a state diagram.
5. **Test strings** with step-by-step traceability and a triple verdict: does
   Automaton 1 accept it? Automaton 2? The union?

The project ships two executables on top of the same logic:

| Executable | What it is |
|---|---|
| `dfa_system` | Console menu. This is the interface the assignment describes. |
| `dfa_server` | Local HTTP server that exposes the same modules in the browser. |

---

## 2. Technical constraint: custom data structures

The assignment forbids the language's collections and helper algorithms. The
constraint is met literally across the **entire automaton model** (`include/`
and `src/`).

**Forbidden and not used:**

- STL containers: `std::vector`, `std::list`, `std::set`, `std::map`,
  `std::unordered_map`, `std::unordered_set`.
- High-level algorithms and helpers: `std::find`, `std::sort`,
  `.contains()`, anything from `<algorithm>`, `std::to_string`, `<sstream>`
  and regular expressions (`<regex>`).

**How it is verified.** The only system headers the model includes are
`<string>` and `<iostream>`:

```
> Select-String -Path include\*.h, src\*.cpp -Pattern '^#include <'

include\Automata.h:4:#include <string>
include\UnionAutomatas.h:4:#include <string>
include\ValidadorAutomata.h:4:#include <string>
src\Automata.cpp:2:#include <iostream>
src\main.cpp:1:#include <iostream>
src\main.cpp:2:#include <string>
```

Six lines in total: `<string>` for state names and `<iostream>` for console
input and output. Nothing else.

`std::string` and `char` are used as **value types** for state names and
symbols: the assignment forbids collections and algorithms, not the
language's basic types.

Every search, insertion, existence check and duplicate detection over states,
alphabet, final states and transitions is written with explicit loops and
one-by-one comparison.

### The presentation layer

The visual interface is a separate layer that **takes no part in any decision**
about states, alphabet, transitions or validation:

- `server/vendor/httplib.h` — a third-party single-header library, used
  exclusively to accept connections and parse HTTP. It moves bytes over the
  network and nothing more.
- `server/ApiHandlers.cpp` — calls the public methods of `Automata`,
  `ValidadorAutomata` and `UnionAutomatas` and builds the JSON by concatenating
  `std::string` by hand, with its own digit-by-digit integer conversion and
  its own parameter parsing. STL containers remain forbidden here: the only
  container is `ArregloDinamico`.
- `server/static/` — the front end, **with no third-party libraries**. The
  diagram is generated by creating the SVG elements one at a time and
  computing the layout with trigonometry. The JavaScript follows the same
  discipline as the C++: flat arrays traversed with `for` loops, no `Map`, no
  `Set`, and no `find`, `includes`, `filter`, `map`, `sort` or `indexOf`. The
  lookups (`contieneTexto`, `posicionDeTexto`, `indiceDeNodo`,
  `buscarDestino`) are hand-written.

---

## 3. Architecture and project structure

```
├── CMakeLists.txt
├── include/
│   ├── ArregloDinamico.h      # the only container: a custom dynamic array
│   ├── Automata.h             # DFA structure
│   ├── ValidadorAutomata.h    # Module 1
│   └── UnionAutomatas.h       # Module 2
├── src/
│   ├── Automata.cpp
│   ├── ValidadorAutomata.cpp
│   ├── UnionAutomatas.cpp
│   └── main.cpp               # console menu  ->  dfa_system
└── server/                    # visual interface  ->  dfa_server
    ├── main_server.cpp        # startup, mounts the front end, opens the browser
    ├── ApiHandlers.cpp        # /api/... routes and JSON assembly
    ├── ApiHandlers.h
    ├── vendor/httplib.h       # third-party header, HTTP transport only
    ├── pruebas_api.ps1        # integration test suite
    ├── pruebas_api2.ps1       # exhaustive test suite
    └── static/
        ├── index.html
        ├── styles.css
        └── app.js             # includes the SVG drawing engine
```

Dependencies between components:

```
        ArregloDinamico<T>
                │
             Automata
            ╱        ╲
ValidadorAutomata   UnionAutomatas
            ╲        ╱
          ┌──────────────┐
          │   dfa_core   │   (shared static library)
          └──────────────┘
             ╱         ╲
      dfa_system    dfa_server
      (console)     (HTTP + front end)
```

The logic lives in `dfa_core` and both interfaces consume it. Neither one
duplicates automaton rules: if a rule changes, it changes in a single place.

---

## 4. Data model

### `ArregloDinamico<T>`

A custom replacement for `std::vector`, with manual resizing through
`new` / `delete[]`. Public interface:

| Operation | Description |
|---|---|
| `agregar(valor)` | Appends at the end; doubles the capacity when needed. |
| `obtener(indice)` | Access by index (const and non-const versions). |
| `eliminarEnIndice(i)` | Removes the element and shifts the following ones. |
| `tamano()` | Number of elements. |
| `estaVacio()` | Whether there are no elements. |
| `vaciar()` | Discards the contents. |

It starts with capacity 4 and doubles it when full, so `agregar` is amortized
O(1). It implements a copy constructor and `operator=` with deep copy, which is
what allows returning an `Automata` by value from the union without sharing
pointers.

**It deliberately offers no search or existence check.** Those algorithms are
the project's responsibility and are implemented in `Automata`,
`ValidadorAutomata` and `UnionAutomatas`.

### `Transicion` and `Automata`

```cpp
struct Transicion {
    std::string origen;
    char        simbolo;
    std::string destino;
};
```

`Automata` stores the five components of the tuple in custom arrays:

```cpp
std::string                    nombre;
ArregloDinamico<std::string>   estados;         // Q
ArregloDinamico<char>          alfabeto;        // Σ
std::string                    estadoInicial;   // q₀
bool                           hayEstadoInicial;
ArregloDinamico<std::string>   estadosFinales;  // F
ArregloDinamico<Transicion>    transiciones;    // δ
```

Transitions are stored as a **flat list of triples**, not as a matrix. This is
what makes it possible to represent a malformed automaton — with a repeated
(state, symbol) pair, or with a missing target — so the validator has
something real to examine. An indexed matrix would make non-determinism
structurally impossible, and Module 1 would have nothing to detect.

Query operations, all with explicit traversal:

| Method | What it does |
|---|---|
| `existeEstado(e)` | Walks Q comparing one by one. |
| `existeSimbolo(c)` | Walks Σ comparing one by one. |
| `esEstadoFinal(e)` | Walks F comparing one by one. |
| `obtenerTransicion(q, s, destino)` | Walks δ looking for the first (q, s) pair. |
| `imprimir()` | Aligned transition table, with markers. |

---

## 5. Module 1 — Strict DFA validation

`ValidadorAutomata::validar` returns a `ResultadoValidacion` with a boolean and
the **complete list of errors found**: it does not stop at the first one, so
an automaton with several problems shows them all at once.

```cpp
struct ResultadoValidacion {
    bool esValido;
    ArregloDinamico<std::string> errores;
};
```

Checks, in execution order:

| # | Check | What it verifies |
|---|---|---|
| 1 | **Non-emptiness** | That Q and Σ are not empty. |
| 2 | **State uniqueness** | That there are no repeated names in Q. |
| 3 | **Alphabet uniqueness** | That there are no repeated symbols in Σ. |
| 4 | **Valid symbols** | That Σ contains no spaces, tabs, line feeds, carriage returns, hyphen or null character. |
| 5 | **Initial state** | That there is exactly one and it belongs to Q. |
| 6 | **Final states** | That every element of F belongs to Q. An empty F is valid. |
| 7 | **Target integrity** | That every transition target is registered in Q. |
| 8 | **Totality and determinism of δ** | That for every (state, symbol) pair there is **exactly one** transition. |

Checks 2 and 3 detect duplicates with a double loop and keep a list of values
already reported, so a state repeated three times produces one error and not
two.

Check 8 is the heart of the module: it iterates every state against every
alphabet symbol and **counts** how many transitions match. Zero transitions
means an incomplete function; more than one means non-determinism.

### Error message catalog

Each error points to the exact failure, naming the specific element. The
messages are the program's actual (Spanish) output:

```
el conjunto de estados esta vacio
el alfabeto esta vacio
el estado 'q0' esta duplicado en el conjunto de estados
el simbolo 'a' esta duplicado en el alfabeto
el alfabeto contiene el guion '-', que es un simbolo reservado y no puede usarse
el alfabeto contiene un espacio en blanco, que es un simbolo reservado y no puede usarse
no se definio un estado inicial
el estado inicial 'zz' no pertenece al conjunto de estados
el estado final 'q9' no pertenece al conjunto de estados
el estado de destino 'q5' no esta registrado en el conjunto de estados
el estado 'q2' carece de transicion para el simbolo 'b'
el estado 'q0' tiene mas de una transicion definida para el simbolo 'a'
```

In order, they mean: the set of states is empty; the alphabet is empty; a state
is duplicated; a symbol is duplicated; the alphabet contains the hyphen or a
blank (reserved symbols); no initial state was defined; the initial state is
not in Q; a final state is not in Q; a target state is not registered in Q; a
state lacks a transition for a symbol; a state has more than one transition
for a symbol.

Reserved multibyte symbols (**ε**, **λ**) do not fit in a `char`, so they are
detected when the input is read, before reaching the alphabet, and reported by
name:

```
El simbolo 'ε' (epsilon) representa la cadena vacia: es un simbolo reservado
y no puede formar parte del alfabeto.
```

### Consequence of a failed validation

An invalid automaton is **not stored** and therefore is not available for the
union or for string testing. The rule applies in both interfaces: in the
console when creating, and in the server before adding it to the store. The
automaton resulting from a union is also validated before being stored.

---

## 6. Module 2 — Automata union

### Alphabet compatibility

Before operating, both alphabets are walked manually in both directions: every
symbol of Σ₁ is looked up in Σ₂ and vice versa. If any difference appears, the
operation is blocked, stating **which specific symbols** do not match.

### Product automaton construction

Given M₁ = (Q₁, Σ, δ₁, q₁, F₁) and M₂ = (Q₂, Σ, δ₂, q₂, F₂):

```
M = (Q₁ × Q₂,  Σ,  δ,  (q₁, q₂),  F)

δ((r₁, r₂), a) = (δ₁(r₁, a), δ₂(r₂, a))
F = { (r₁, r₂) : r₁ ∈ F₁  or  r₂ ∈ F₂ }
```

- **Composite states**: a double loop over Q₁ and Q₂; each pair is explicitly
  represented with the name `(state1,state2)`.
- **Combined transitions**: for each pair and each symbol, `obtenerTransicion`
  is queried on **both original structures** simultaneously, and the target is
  the pair of targets.
- **Acceptance by disjunction (OR)**: the pair is final if the first component
  is final in M₁ **or** the second is final in M₂, evaluated with
  `esEstadoFinal`.
- **Initial state**: the pair of initial states.

### Worked example

**M₁ = TerminaEnA** — strings that end in `a`:
Q₁ = {q0, q1}, Σ = {a, b}, q₁ = q0, F₁ = {q1}

**M₂ = ParesDeB** — strings with an even number of `b`:
Q₂ = {p0, p1}, Σ = {a, b}, q₂ = p0, F₂ = {p0}

Cartesian product: 2 × 2 = 4 composite states. Initial state `(q0,p0)`.

Acceptance by disjunction:

| Pair | q ∈ F₁ | p ∈ F₂ | Final? |
|---|---|---|---|
| `(q0,p0)` | no | **yes** | **yes** |
| `(q0,p1)` | no | no | no |
| `(q1,p0)` | **yes** | **yes** | **yes** |
| `(q1,p1)` | **yes** | no | **yes** |

Resulting transition table (`→` initial, `*` accepting):

|   | State | a | b |
|---|---|---|---|
| →* | `(q0,p0)` | `(q1,p0)` | `(q0,p1)` |
|  | `(q0,p1)` | `(q1,p1)` | `(q0,p0)` |
| * | `(q1,p0)` | `(q1,p0)` | `(q0,p1)` |
| * | `(q1,p1)` | `(q1,p1)` | `(q0,p0)` |

Check with the string `abb` (`RECHAZADA` = rejected, `ACEPTADA` = accepted):

```
Automata 1      RECHAZADA   q0 → q1 → q0 → q0            (does not end in a)
Automata 2      ACEPTADA    p0 → p0 → p1 → p0            (two b, even count)
Automata union  ACEPTADA    (q0,p0) → (q1,p0) → (q0,p1) → (q0,p0)
```

The union's verdict is `REJECTED or ACCEPTED = ACCEPTED`, and each component of
the composite path matches exactly the corresponding individual path.

---

## 7. Output and string testing

### Automaton visualization

- **Transition table**: one row per state and one column per symbol, with a
  `→` marker for the initial state and `*` for accepting ones. In the console
  the column widths are computed by walking the data, so the table stays
  aligned with names of any length.
- **Explicit listing**: obtained by iterating the custom structures — the full
  set of states, the alphabet, the initial state and the complete list of
  accepting states.
- **State diagram** (visual interface): circles for states, a double ring for
  accepting ones, an entry arrow for the initial one, labeled arcs for
  transitions and loops for self-transitions.

### String testing and inspection

Any string over the alphabet is accepted, including the empty string. The walk
is performed step by step, storing each visited state:

- **Traceability**: the complete sequence of states from the initial one to the
  one reached is printed, indicating which symbol produced each jump.
- If at some point no transition is defined, the walk stops and reports **on
  which symbol** it got stuck.
- **Triple verdict**: the same string is evaluated independently on Automaton
  1, Automaton 2 and the union, and the three results are reported separately.

---

## 8. Build and run

**Requirements:** CMake ≥ 3.16 and a C++17 compiler (MinGW g++, MSVC or clang).

```powershell
cmake -S . -B build
cmake --build build
```

Two executables are generated:

```powershell
.\build\dfa_system.exe    # console menu
.\build\dfa_server.exe    # visual interface at http://localhost:8080
```

Depending on the generator CMake uses, the binaries may end up in
`build\Debug\`. The server opens the browser automatically and stops with
`Ctrl+C`; the front end is served from `server/static`, so editing the HTML,
CSS or JS does not require recompiling.

The project compiles without warnings with `-Wall -Wextra -Wpedantic`.

---

## 9. Usage guide: console menu

```
===== Sistema de Automatas Finitos Deterministas =====
1. Crear automata
2. Listar automatas guardados
3. Validar un automata
4. Unir dos automatas
5. Probar una cadena
0. Salir
```

(1. Create automaton · 2. List stored automata · 3. Validate an automaton ·
4. Unite two automata · 5. Test a string · 0. Exit)

**Create.** Asks for a name, the number of states and their names, the number
of symbols and their values, the initial state, the accepting states and the
transitions in the format `origen simbolo destino` (source symbol target). When
finished, it validates automatically: if there are errors it lists them and
**does not store** the automaton.

**Unite.** Asks for the indices of two stored automata. If the alphabets do not
match, it indicates the symbols that differ. If the union is valid, it prints
it and stores it as one more automaton, available for testing.

**Test a string.** Asks for the index of the union and those of the two
original automata, then the string (`-` for the empty string). It shows the
step-by-step walk over the union and the triple verdict.

Example session creating an invalid automaton:

```
--- Crear automata ---
...
El automata NO es valido. Errores encontrados:
  - el estado final 'zz' no pertenece al conjunto de estados
No se guarda ni queda disponible para la union.
```

(The automaton is NOT valid. Errors found: the final state 'zz' does not
belong to the set of states. It is not stored and is not available for the
union.)

---

## 10. Usage guide: visual interface

`dfa_server` starts a local server at `http://localhost:8080` with four tabs:

- **Create** — a form with the δ matrix generated from the defined states and
  alphabet. The *Free* mode allows loading loose transitions, repeating a
  (state, symbol) pair or pointing to nonexistent states, to see the validator
  in action. The buttons separate *Validate without saving* from *Create and
  save*.
- **Automaton** — diagram, explicit listing and transition table of the
  selected automaton.
- **Union** — selection of the two operands and a **step-by-step
  construction**: starting formal definition, Cartesian product, initial
  state, derivation of each transition δ((r₁,r₂), a) = (δ₁(r₁,a), δ₂(r₂,a))
  and evaluation of the disjunction for each pair.
- **Test string** — a player with step forward, step back and a slider. It
  highlights the current state on the diagrams and shows the input tape with
  the symbol being read, the sequence of composite states and the formal
  development of the extended transition function δ̂.

### HTTP API

| Method | Route | Function |
|---|---|---|
| `GET` | `/api/automatas` | Lists the stored automata. |
| `GET` | `/api/automata?id=N` | Returns a specific one. |
| `POST` | `/api/automatas` | Creates and validates; stores only if valid. |
| `POST` | `/api/validar` | Revalidates a stored automaton. |
| `POST` | `/api/unir` | Unites two automata by index. |
| `POST` | `/api/probar-cadena` | Walk and verdict (single or triple). |

Responses are JSON built by string concatenation, with custom escaping of
quotes, slashes and control characters.

Transitions travel from the front end to the server as
`origen + SEP + simbolo + SEP + destino`, where `SEP` is the control character
`0x1F`. Since it cannot be typed into a form, no state name or symbol can
contain it, which makes the splitting unambiguous with no need for escaping or
regular expressions.

---

## 11. Custom algorithms and complexity

No operation delegates to the language. Let *n* = |Q|, *m* = |Σ| and *t* = |δ|:

| Algorithm | Technique | Complexity |
|---|---|---|
| `ArregloDinamico::agregar` | Capacity doubling | Amortized O(1) |
| `ArregloDinamico::eliminarEnIndice` | Shifting the tail | O(n) |
| `existeEstado`, `existeSimbolo`, `esEstadoFinal` | Linear search | O(n) |
| `obtenerTransicion` | Linear search over δ | O(t) |
| State / alphabet uniqueness | Double loop with a list of reported values | O(n²) |
| Target integrity | Per transition, a search in Q | O(t · n) |
| Totality and determinism of δ | For each (state, symbol) pair, a count over δ | O(n · m · t) |
| Alphabet compatibility | Double cross traversal | O(m²) |
| Union construction | Cartesian product and a double lookup per symbol | O(n₁ · n₂ · m · t) |
| Simulating a string *w* | One transition per symbol | O(&#124;w&#124; · t) |

All bounds correspond to linear search, which is what the project's
constraint imposes: with no hash tables or ordered containers, every query
walks the array comparing one by one. None of these operations relies on a
library function; all are written with explicit loops over `ArregloDinamico`.

---

## 12. Tests

The project includes two PowerShell integration suites that exercise the full
API against the running server: **635 checks**.

```powershell
.\build\dfa_server.exe          # in one terminal
.\server\pruebas_api.ps1        # in another
.\server\pruebas_api2.ps1
```

Coverage:

| Suite | Sections |
|---|---|
| `pruebas_api.ps1` | Valid automata, invalid cases, dirty input, hostile names and symbols (Unicode, quotes, HTML, 300-character names), malformed transitions, unions, invalid parameters, single and triple string testing, routes and methods, load and consistency. |
| `pruebas_api2.ps1` | A sweep of the 94 printable ASCII characters as a symbol, large alphabets, verification of the "binary divisible by 3" language over 126 strings, an exhaustive check that the union is exactly the OR of the languages, pair-name collisions, error combinations, determinism and idempotence. |

Two checks stand out for what they demonstrate:

- **Verified language**: the DFA for binary numbers divisible by 3 is built and
  126 strings are checked against the direct arithmetic computation.
- **Union = OR**: every string up to a given length is enumerated and the union
  is checked to accept exactly when M₁ or M₂ does.

---

## 13. Design decisions

**Permissive insertion, centralized validation.** `agregarEstado`,
`agregarSimbolo` and `agregarEstadoFinal` accept whatever they receive. The
alternative, filtering at insertion, would leave half of Module 1's checks
unused: if a duplicate state never makes it into the structure, the validator
has nothing to report about it. By concentrating the criterion in a single
place, the validator is the one that judges and the one that issues the
specific diagnosis the assignment asks for.

**δ as a list of triples, not a matrix.** A `state × symbol` matrix represents
by construction a total, deterministic function, so an incomplete or
non-deterministic automaton would be inexpressible and Module 1 would have
nothing to verify. The flat list can represent those cases and lets the
validator detect and name them.

**Accumulated errors, no short-circuit.** `validar` always runs all eight
checks and returns every error. An automaton with four problems shows all four,
instead of forcing them to be fixed one at a time.

**The union is validated too.** The name of a composite state is formed as
`(state1,state2)`, so if the original names contain commas or parentheses two
different pairs could produce the same name. The product automaton goes through
the validator before being stored and, if it does not pass, the operation is
blocked with the reason. It is the same criterion as Module 1, applied to an
automaton generated by the program instead of written by the user.

**A single source of logic.** The console and the server consume the same
`dfa_core` library. Neither interface reimplements an automaton rule.

---

## 14. Scope

**Single-character symbols.** Σ is modeled as a set of `char`, which is the
direct representation of the alphabet in the formal definition: a symbol is an
atomic element of the alphabet, not a string. The reserved symbols that denote
the empty string (ε, λ) are recognized when the input is read and rejected with
their own message, as Module 1 requires.

**Session model.** Defined automata are stored in an
`ArregloDinamico<Automata>` and remain available for validating, uniting and
testing strings throughout the run. Both interfaces share that same model.

**Linear search in every structure.** This is a direct consequence of the
project's constraint: with no hash tables or ordered containers, every query is
resolved by walking the array and comparing one by one. The bounds in section
11 are exactly those of that decision, and each of those operations is written
with explicit loops.
