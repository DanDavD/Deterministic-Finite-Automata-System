# Sistema de Autómatas Finitos Deterministas (DFA)

Proyecto de Teoría de la Computación: validación estricta de un Autómata
Finito Determinista (DFA) y operación de unión entre dos DFA válidos.
Implementado en C++17.

## Regla del proyecto: sin colecciones nativas

Está prohibido usar `std::vector`, `std::list`, `std::set`, `std::map`,
`std::unordered_map`, etc., y funciones auxiliares de alto nivel como
`std::find`, `.contains()`, `std::sort`, expresiones regulares, etc.

Por eso el proyecto trae ya construido `include/ArregloDinamico.h`: un
arreglo dinámico propio (redimensionamiento manual con `new`/`delete[]`)
que reemplaza a `std::vector`. Solo ofrece operaciones primitivas
(`agregar`, `obtener`, `eliminarEnIndice`, `tamano`, `vaciar`). **A
propósito no incluye búsqueda ni verificación de existencia**: esos
algoritmos (recorrido manual, comparación uno a uno, detección de
duplicados, etc.) son parte de lo que pide el enunciado que implementes tú
mismo en `Automata`, `ValidadorAutomata` y `UnionAutomatas`.

Nota: se usa `std::string` como tipo de valor para nombres de estados
(no como contenedor) y `char` para los símbolos del alfabeto, ya que el
enunciado prohíbe *colecciones/contenedores*, no los tipos básicos del
lenguaje.

## Estructura del proyecto

```
├── CMakeLists.txt
├── include/
│   ├── ArregloDinamico.h     # arreglo dinámico propio (ya implementado)
│   ├── Automata.h            # estructura del DFA (declaraciones)
│   ├── ValidadorAutomata.h   # Módulo 1: validación (declaraciones)
│   └── UnionAutomatas.h      # Módulo 2: unión (declaraciones)
└── src/
    ├── main.cpp              # menú de consola (ya funcional)
    ├── Automata.cpp
    ├── ValidadorAutomata.cpp
    └── UnionAutomatas.cpp
```

## Qué ya está hecho

- Entorno de compilación (CMake) y `.gitignore` para C++.
- `ArregloDinamico<T>`: arreglo dinámico propio, completo y funcional.
- `Automata`: estructura de datos completa (estados, alfabeto, estado
  inicial, estados finales, transiciones), con métodos para agregar datos.
- `main.cpp`: menú de consola completo y funcional (crear autómata,
  listar, validar, unir, probar cadena) que ya lee datos del usuario y los
  guarda usando `Automata` / `ArregloDinamico`.
- Firmas de todos los métodos de validación y unión, cada uno documentado
  con un comentario `TODO(alumno)` que indica exactamente qué punto del
  enunciado cubre y cómo implementarlo.

## Qué falta por programar (marcado con `TODO(alumno)`)

**`src/Automata.cpp`**
- `existeEstado`, `existeSimbolo`, `esEstadoFinal`: búsqueda manual.
- `agregarEstado`, `agregarSimbolo`, `agregarEstadoFinal`: rechazar
  duplicados / símbolos inválidos / estados no registrados.
- `obtenerTransicion`: búsqueda manual de la transición (origen, símbolo).

**`src/ValidadorAutomata.cpp`** (Módulo 1, punto 3 del enunciado)
- Unicidad de estados y de símbolos del alfabeto.
- Simbología válida (rechazar `ε`, `λ`, espacios, guiones).
- Existencia y validez del estado inicial.
- Validez de los estados finales.
- Completitud, integridad de destino y determinismo de las transiciones.

**`src/UnionAutomatas.cpp`** (Módulo 2, punto 3 del enunciado)
- `alfabetosCoinciden`: comparación manual de ambos alfabetos.
- `construirAutomataUnion`: producto de estados, transiciones combinadas y
  criterio de aceptación por disyunción (OR).

**`src/main.cpp`**
- `probarCadenaMenu`: simulación paso a paso de una cadena sobre el
  autómata unión, con el veredicto triple (Automata 1 / Automata 2 /
  Unión) que pide el punto 4.2.

## Compilar y ejecutar

Requiere CMake ≥ 3.16 y un compilador C++17 (MSVC, MinGW g++, o clang).

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\dfa_system.exe   # o build\dfa_system.exe según el generador
```

También puedes abrir la carpeta directamente en Visual Studio o CLion:
ambos detectan el `CMakeLists.txt` automáticamente.

## Nota sobre el control de versiones

Este entorno de trabajo no ejecuta comandos `git` (ni hace commits ni
push). Los archivos ya están listos en el directorio de trabajo: revisa
`git status`, haz `git add`, `git commit` y `git push` tú mismo cuando
quieras subir esta primera versión.
