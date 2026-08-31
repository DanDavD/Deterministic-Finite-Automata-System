# Sistema de Autómatas Finitos Deterministas (DFA)

Herramienta de teoría de la computación que automatiza dos tareas: el
**análisis de validez formal** de un Autómata Finito Determinista y la
**operación de unión** entre dos DFA válidos, generando el autómata producto
que reconoce el lenguaje unión.

Implementado en C++17, con todas las estructuras de datos y algoritmos de
búsqueda programados desde cero.

---

## Índice

1. [Descripción general](#1-descripción-general)
2. [Restricción técnica: estructuras propias](#2-restricción-técnica-estructuras-propias)
3. [Arquitectura y estructura del proyecto](#3-arquitectura-y-estructura-del-proyecto)
4. [Modelo de datos](#4-modelo-de-datos)
5. [Módulo 1 — Validación estricta de DFA](#5-módulo-1--validación-estricta-de-dfa)
6. [Módulo 2 — Unión de autómatas](#6-módulo-2--unión-de-autómatas)
7. [Salidas y prueba de cadenas](#7-salidas-y-prueba-de-cadenas)
8. [Compilación y ejecución](#8-compilación-y-ejecución)
9. [Guía de uso: menú de consola](#9-guía-de-uso-menú-de-consola)
10. [Guía de uso: interfaz visual](#10-guía-de-uso-interfaz-visual)
11. [Algoritmos propios y complejidad](#11-algoritmos-propios-y-complejidad)
12. [Pruebas](#12-pruebas)
13. [Decisiones de diseño](#13-decisiones-de-diseño)
14. [Alcance](#14-alcance)

---

## 1. Descripción general

Un DFA se define formalmente como la quíntupla **M = (Q, Σ, δ, q₀, F)**, donde
Q es el conjunto de estados, Σ el alfabeto, δ: Q × Σ → Q la función de
transición, q₀ el estado inicial y F ⊆ Q el conjunto de estados de aceptación.

El sistema permite:

1. **Definir** autómatas indicando estados, alfabeto, estado inicial, estados
   de aceptación y transiciones.
2. **Validar** estrictamente que la definición cumpla las propiedades de un
   DFA, con un informe de errores específico. Un autómata que no pasa la
   validación no se guarda ni queda disponible para operaciones posteriores.
3. **Unir** dos DFA válidos mediante el producto cartesiano de sus estados,
   con aceptación por disyunción.
4. **Visualizar** el resultado como tabla de transiciones, listado explícito
   de componentes y diagrama de estados.
5. **Probar cadenas** con trazabilidad paso a paso y veredicto triple:
   ¿la acepta el Autómata 1?, ¿el Autómata 2?, ¿la unión?

El proyecto entrega dos ejecutables sobre la misma lógica:

| Ejecutable | Qué es |
|---|---|
| `dfa_system` | Menú de consola. Es la interfaz que describe el enunciado. |
| `dfa_server` | Servidor HTTP local que expone los mismos módulos en el navegador. |

---

## 2. Restricción técnica: estructuras propias

El enunciado prohíbe colecciones y algoritmos auxiliares del lenguaje. La
restricción se cumple de forma literal en **todo el modelo del autómata**
(`include/` y `src/`).

**Prohibido y no utilizado:**

- Contenedores STL: `std::vector`, `std::list`, `std::set`, `std::map`,
  `std::unordered_map`, `std::unordered_set`.
- Algoritmos y auxiliares de alto nivel: `std::find`, `std::sort`,
  `.contains()`, cualquier cosa de `<algorithm>`, `std::to_string`,
  `<sstream>` y expresiones regulares (`<regex>`).

**Cómo se verifica.** Las únicas cabeceras del sistema que incluye el modelo
son `<string>` e `<iostream>`:

```
> Select-String -Path include\*.h, src\*.cpp -Pattern '^#include <'

include\Automata.h:4:#include <string>
include\UnionAutomatas.h:4:#include <string>
include\ValidadorAutomata.h:4:#include <string>
src\Automata.cpp:2:#include <iostream>
src\main.cpp:1:#include <iostream>
src\main.cpp:2:#include <string>
```

Seis líneas en total: `<string>` para los nombres de estado y `<iostream>`
para la entrada y salida por consola. Nada más.

`std::string` y `char` sí se usan como **tipos de valor** para nombres de
estado y símbolos: el enunciado prohíbe colecciones y algoritmos, no los tipos
básicos del lenguaje.

Toda búsqueda, inserción, verificación de existencia y detección de duplicados
sobre estados, alfabeto, estados finales y transiciones está programada con
bucles explícitos y comparación uno a uno.

### La capa de presentación

La interfaz visual es una capa aparte que **no participa en ninguna decisión**
sobre estados, alfabeto, transiciones ni validación:

- `server/vendor/httplib.h` — biblioteca de terceros de un solo header, usada
  exclusivamente para aceptar conexiones y parsear HTTP. Mueve bytes por la
  red y nada más.
- `server/ApiHandlers.cpp` — llama a los métodos públicos de `Automata`,
  `ValidadorAutomata` y `UnionAutomatas` y arma el JSON concatenando
  `std::string` a mano, con su propia conversión de enteros dígito por dígito
  y su propio parseo de parámetros. Aquí siguen prohibidos los contenedores
  STL: el único contenedor es `ArregloDinamico`.
- `server/static/` — el front, **sin librerías de terceros**. El diagrama se
  genera creando los elementos SVG uno por uno y calculando el layout con
  trigonometría. En el JavaScript rige la misma disciplina que en C++:
  arreglos planos recorridos con bucles `for`, sin `Map`, sin `Set` y sin
  `find`, `includes`, `filter`, `map`, `sort` ni `indexOf`. Las búsquedas
  (`contieneTexto`, `posicionDeTexto`, `indiceDeNodo`, `buscarDestino`) están
  escritas a mano.

---

## 3. Arquitectura y estructura del proyecto

```
├── CMakeLists.txt
├── include/
│   ├── ArregloDinamico.h      # único contenedor: arreglo dinámico propio
│   ├── Automata.h             # estructura del DFA
│   ├── ValidadorAutomata.h    # Módulo 1
│   └── UnionAutomatas.h       # Módulo 2
├── src/
│   ├── Automata.cpp
│   ├── ValidadorAutomata.cpp
│   ├── UnionAutomatas.cpp
│   └── main.cpp               # menú de consola  ->  dfa_system
└── server/                    # interfaz visual  ->  dfa_server
    ├── main_server.cpp        # arranque, monta el front, abre el navegador
    ├── ApiHandlers.cpp        # rutas /api/... y armado del JSON
    ├── ApiHandlers.h
    ├── vendor/httplib.h       # header de terceros, solo transporte HTTP
    ├── pruebas_api.ps1        # suite de pruebas de integración
    ├── pruebas_api2.ps1       # suite de pruebas exhaustivas
    └── static/
        ├── index.html
        ├── styles.css
        └── app.js             # incluye el motor de dibujo SVG
```

Dependencias entre componentes:

```
        ArregloDinamico<T>
                │
             Automata
            ╱        ╲
ValidadorAutomata   UnionAutomatas
            ╲        ╱
          ┌──────────────┐
          │   dfa_core   │   (biblioteca estática compartida)
          └──────────────┘
             ╱         ╲
      dfa_system    dfa_server
      (consola)     (HTTP + front)
```

La lógica vive en `dfa_core` y las dos interfaces la consumen. Ninguna de las
dos duplica reglas del autómata: si una regla cambia, cambia en un solo lugar.

---

## 4. Modelo de datos

### `ArregloDinamico<T>`

Reemplazo propio de `std::vector`, con redimensionamiento manual mediante
`new` / `delete[]`. Interfaz pública:

| Operación | Descripción |
|---|---|
| `agregar(valor)` | Inserta al final; duplica la capacidad si hace falta. |
| `obtener(indice)` | Acceso por índice (versiones const y no const). |
| `eliminarEnIndice(i)` | Elimina y desplaza los elementos posteriores. |
| `tamano()` | Cantidad de elementos. |
| `estaVacio()` | Si no hay elementos. |
| `vaciar()` | Descarta el contenido. |

Arranca con capacidad 4 y la duplica al llenarse, así que `agregar` es O(1)
amortizado. Implementa constructor de copia y `operator=` con copia profunda,
que es lo que permite devolver un `Automata` por valor desde la unión sin
compartir punteros.

**A propósito no ofrece búsqueda ni verificación de existencia.** Esos
algoritmos son responsabilidad del proyecto y están implementados en
`Automata`, `ValidadorAutomata` y `UnionAutomatas`.

### `Transicion` y `Automata`

```cpp
struct Transicion {
    std::string origen;
    char        simbolo;
    std::string destino;
};
```

`Automata` guarda los cinco componentes de la quíntupla en arreglos propios:

```cpp
std::string                    nombre;
ArregloDinamico<std::string>   estados;         // Q
ArregloDinamico<char>          alfabeto;        // Σ
std::string                    estadoInicial;   // q₀
bool                           hayEstadoInicial;
ArregloDinamico<std::string>   estadosFinales;  // F
ArregloDinamico<Transicion>    transiciones;    // δ
```

Las transiciones se guardan como una **lista plana de tripletas**, no como una
matriz. Es lo que permite representar un autómata mal formado —con un par
(estado, símbolo) repetido, o sin destino— para que el validador tenga algo
real que examinar. Una matriz indexada haría estructuralmente imposible el
no-determinismo, y el Módulo 1 no tendría nada que detectar.

Operaciones de consulta, todas con recorrido explícito:

| Método | Qué hace |
|---|---|
| `existeEstado(e)` | Recorre Q comparando uno a uno. |
| `existeSimbolo(c)` | Recorre Σ comparando uno a uno. |
| `esEstadoFinal(e)` | Recorre F comparando uno a uno. |
| `obtenerTransicion(q, s, destino)` | Recorre δ buscando el primer par (q, s). |
| `imprimir()` | Tabla de transiciones alineada, con marcadores. |

---

## 5. Módulo 1 — Validación estricta de DFA

`ValidadorAutomata::validar` devuelve un `ResultadoValidacion` con un booleano
y la **lista completa de errores encontrados**: no corta en el primero, para
que un autómata con varios problemas los muestre todos de una vez.

```cpp
struct ResultadoValidacion {
    bool esValido;
    ArregloDinamico<std::string> errores;
};
```

Verificaciones, en orden de ejecución:

| # | Verificación | Qué comprueba |
|---|---|---|
| 1 | **No vacuidad** | Que Q y Σ no estén vacíos. |
| 2 | **Unicidad de estados** | Que no haya nombres repetidos en Q. |
| 3 | **Unicidad del alfabeto** | Que no haya símbolos repetidos en Σ. |
| 4 | **Simbología válida** | Que Σ no contenga espacios, tabulaciones, saltos de línea, retornos de carro, el guion ni el carácter nulo. |
| 5 | **Estado inicial** | Que exista exactamente uno y pertenezca a Q. |
| 6 | **Estados finales** | Que cada elemento de F pertenezca a Q. F vacío es válido. |
| 7 | **Integridad de destino** | Que todo destino de una transición esté registrado en Q. |
| 8 | **Totalidad y determinismo de δ** | Que para cada par (estado, símbolo) exista **exactamente una** transición. |

Las verificaciones 2 y 3 detectan duplicados con doble bucle y llevan una
lista de valores ya reportados, para que un estado repetido tres veces genere
un solo error y no dos.

La verificación 8 es el núcleo del módulo: itera cada estado contra cada
símbolo del alfabeto y **cuenta** cuántas transiciones coinciden. Cero
transiciones significa función incompleta; más de una, no-determinismo.

### Catálogo de mensajes de error

Cada error señala la falla exacta, nombrando el elemento concreto:

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

Los símbolos reservados multibyte (**ε**, **λ**) no caben en un `char`, así que
se detectan al leer la entrada, antes de llegar al alfabeto, y se reportan con
su nombre:

```
El simbolo 'ε' (epsilon) representa la cadena vacia: es un simbolo reservado
y no puede formar parte del alfabeto.
```

### Consecuencia de una validación fallida

Un autómata inválido **no se guarda** y por lo tanto no queda disponible para
la unión ni para la prueba de cadenas. La regla se aplica en las dos
interfaces: en la consola al crear, y en el servidor antes de añadirlo al
almacén. El autómata resultante de una unión también se valida antes de
guardarse.

---

## 6. Módulo 2 — Unión de autómatas

### Compatibilidad de alfabetos

Antes de operar se recorren manualmente ambos alfabetos en las dos
direcciones: cada símbolo de Σ₁ se busca en Σ₂ y viceversa. Si aparece alguna
diferencia, la operación se bloquea indicando **qué símbolos concretos** no
coinciden.

### Construcción del autómata producto

Dados M₁ = (Q₁, Σ, δ₁, q₁, F₁) y M₂ = (Q₂, Σ, δ₂, q₂, F₂):

```
M = (Q₁ × Q₂,  Σ,  δ,  (q₁, q₂),  F)

δ((r₁, r₂), a) = (δ₁(r₁, a), δ₂(r₂, a))
F = { (r₁, r₂) : r₁ ∈ F₁  o  r₂ ∈ F₂ }
```

- **Estados compuestos**: doble bucle sobre Q₁ y Q₂; cada par se representa
  explícitamente con el nombre `(estado1,estado2)`.
- **Transiciones combinadas**: para cada par y cada símbolo se consulta
  `obtenerTransicion` en **ambas estructuras originales** de forma simultánea,
  y el destino es el par de destinos.
- **Aceptación por disyunción (OR)**: el par es final si el primer componente
  es final en M₁ **o** el segundo lo es en M₂, evaluado con `esEstadoFinal`.
- **Estado inicial**: el par de estados iniciales.

### Ejemplo trabajado

**M₁ = TerminaEnA** — cadenas que terminan en `a`:
Q₁ = {q0, q1}, Σ = {a, b}, q₁ = q0, F₁ = {q1}

**M₂ = ParesDeB** — cadenas con una cantidad par de `b`:
Q₂ = {p0, p1}, Σ = {a, b}, q₂ = p0, F₂ = {p0}

Producto cartesiano: 2 × 2 = 4 estados compuestos. Estado inicial `(q0,p0)`.

Aceptación por disyunción:

| Par | q ∈ F₁ | p ∈ F₂ | ¿Final? |
|---|---|---|---|
| `(q0,p0)` | no | **sí** | **sí** |
| `(q0,p1)` | no | no | no |
| `(q1,p0)` | **sí** | **sí** | **sí** |
| `(q1,p1)` | **sí** | no | **sí** |

Tabla de transiciones resultante (`→` inicial, `*` aceptación):

|   | Estado | a | b |
|---|---|---|---|
| →* | `(q0,p0)` | `(q1,p0)` | `(q0,p1)` |
|  | `(q0,p1)` | `(q1,p1)` | `(q0,p0)` |
| * | `(q1,p0)` | `(q1,p0)` | `(q0,p1)` |
| * | `(q1,p1)` | `(q1,p1)` | `(q0,p0)` |

Verificación con la cadena `abb`:

```
Automata 1      RECHAZADA   q0 → q1 → q0 → q0            (no termina en a)
Automata 2      ACEPTADA    p0 → p0 → p1 → p0            (dos b, cantidad par)
Automata union  ACEPTADA    (q0,p0) → (q1,p0) → (q0,p1) → (q0,p0)
```

El veredicto de la unión es `RECHAZADA o ACEPTADA = ACEPTADA`, y cada
componente del recorrido compuesto coincide exactamente con el recorrido
individual correspondiente.

---

## 7. Salidas y prueba de cadenas

### Visualización del autómata

- **Tabla de transiciones**: una fila por estado y una columna por símbolo,
  con marcador `→` para el estado inicial y `*` para los de aceptación. En
  consola los anchos de columna se calculan recorriendo los datos, para que la
  tabla quede alineada con nombres de cualquier longitud.
- **Listado explícito**: obtenido iterando las estructuras propias — conjunto
  total de estados, alfabeto, estado inicial y lista completa de estados de
  aceptación.
- **Diagrama de estados** (interfaz visual): círculos para los estados, doble
  anillo para los de aceptación, flecha de entrada para el inicial, arcos
  etiquetados para las transiciones y bucles para las transiciones a sí mismo.

### Prueba e inspección de cadenas

Se admite cualquier cadena sobre el alfabeto, incluida la cadena vacía. El
recorrido se hace paso a paso guardando cada estado visitado:

- **Trazabilidad**: se imprime la secuencia completa de estados desde el
  inicial hasta el alcanzado, indicando qué símbolo produjo cada salto.
- Si en algún punto no hay transición definida, el recorrido se detiene y se
  informa **en qué símbolo** se trabó.
- **Veredicto triple**: la misma cadena se evalúa de forma independiente sobre
  el Autómata 1, el Autómata 2 y la unión, y se reportan los tres resultados
  por separado.

---

## 8. Compilación y ejecución

**Requisitos:** CMake ≥ 3.16 y un compilador C++17 (MinGW g++, MSVC o clang).

```powershell
cmake -S . -B build
cmake --build build
```

Se generan dos ejecutables:

```powershell
.\build\dfa_system.exe    # menú de consola
.\build\dfa_server.exe    # interfaz visual en http://localhost:8080
```

Según el generador que use CMake, los binarios pueden quedar en
`build\Debug\`. El servidor abre el navegador automáticamente y se detiene con
`Ctrl+C`; el front se sirve desde `server/static`, así que editar el HTML, el
CSS o el JS no obliga a recompilar.

El proyecto compila sin advertencias con `-Wall -Wextra -Wpedantic`.

---

## 9. Guía de uso: menú de consola

```
===== Sistema de Automatas Finitos Deterministas =====
1. Crear automata
2. Listar automatas guardados
3. Validar un automata
4. Unir dos automatas
5. Probar una cadena
0. Salir
```

**Crear.** Pide nombre, cantidad de estados y sus nombres, cantidad de
símbolos y sus valores, estado inicial, estados de aceptación y transiciones
en formato `origen simbolo destino`. Al terminar, valida automáticamente: si
hay errores los lista y **no guarda** el autómata.

**Unir.** Pide los índices de dos autómatas guardados. Si los alfabetos no
coinciden, indica los símbolos que difieren. Si la unión resulta válida, la
imprime y la guarda como un autómata más, disponible para pruebas.

**Probar una cadena.** Pide el índice de la unión y los de los dos autómatas
originales, luego la cadena (`-` para la cadena vacía). Muestra el recorrido
paso a paso sobre la unión y el veredicto triple.

Ejemplo de sesión creando un autómata inválido:

```
--- Crear automata ---
...
El automata NO es valido. Errores encontrados:
  - el estado final 'zz' no pertenece al conjunto de estados
No se guarda ni queda disponible para la union.
```

---

## 10. Guía de uso: interfaz visual

`dfa_server` levanta un servidor local en `http://localhost:8080` con cuatro
pestañas:

- **Crear** — formulario con la matriz de δ generada a partir de los estados y
  el alfabeto definidos. El modo *Libre* permite cargar transiciones sueltas,
  repetir un par (estado, símbolo) o apuntar a estados inexistentes, para ver
  el validador en acción. Los botones separan *Validar sin guardar* de *Crear
  y guardar*.
- **Autómata** — diagrama, listado explícito y tabla de transiciones del
  autómata seleccionado.
- **Unión** — selección de los dos operandos y **construcción paso a paso**:
  definición formal de partida, producto cartesiano, estado inicial,
  derivación de cada transición δ((r₁,r₂), a) = (δ₁(r₁,a), δ₂(r₂,a)) y
  evaluación de la disyunción para cada par.
- **Probar cadena** — reproductor con avance paso a paso, retroceso y
  deslizador. Resalta el estado actual sobre los diagramas, muestra la cinta
  de entrada con el símbolo en curso, la secuencia de estados compuestos y el
  desarrollo formal de la función de transición extendida δ̂.

### API HTTP

| Método | Ruta | Función |
|---|---|---|
| `GET` | `/api/automatas` | Lista los autómatas guardados. |
| `GET` | `/api/automata?id=N` | Devuelve uno concreto. |
| `POST` | `/api/automatas` | Crea y valida; guarda solo si es válido. |
| `POST` | `/api/validar` | Revalida un autómata guardado. |
| `POST` | `/api/unir` | Une dos autómatas por índice. |
| `POST` | `/api/probar-cadena` | Recorrido y veredicto (simple o triple). |

Las respuestas son JSON construido por concatenación de cadenas, con escapado
propio de comillas, barras y caracteres de control.

Las transiciones viajan del front al servidor como
`origen + SEP + simbolo + SEP + destino`, donde `SEP` es el carácter de control
`0x1F`. Al no poder escribirse en un formulario, ningún nombre de estado ni
símbolo puede contenerlo, lo que hace el troceado inequívoco sin necesidad de
escapado ni de expresiones regulares.

---

## 11. Algoritmos propios y complejidad

Ninguna operación delega en el lenguaje. Sea *n* = |Q|, *m* = |Σ| y *t* = |δ|:

| Algoritmo | Técnica | Complejidad |
|---|---|---|
| `ArregloDinamico::agregar` | Duplicación de capacidad | O(1) amortizado |
| `ArregloDinamico::eliminarEnIndice` | Desplazamiento de la cola | O(n) |
| `existeEstado`, `existeSimbolo`, `esEstadoFinal` | Búsqueda lineal | O(n) |
| `obtenerTransicion` | Búsqueda lineal sobre δ | O(t) |
| Unicidad de estados / alfabeto | Doble bucle con lista de reportados | O(n²) |
| Integridad de destinos | Por transición, búsqueda en Q | O(t · n) |
| Totalidad y determinismo de δ | Por cada par (estado, símbolo), conteo sobre δ | O(n · m · t) |
| Compatibilidad de alfabetos | Doble recorrido cruzado | O(m²) |
| Construcción de la unión | Producto cartesiano y consulta doble por símbolo | O(n₁ · n₂ · m · t) |
| Simulación de una cadena *w* | Una transición por símbolo | O(&#124;w&#124; · t) |

Todas las cotas corresponden a búsqueda lineal, que es lo que impone la
restricción del proyecto: sin tablas hash ni contenedores ordenados, cada
consulta recorre el arreglo comparando uno a uno. Ninguna de estas operaciones
se apoya en una función de biblioteca; todas están escritas con bucles
explícitos sobre `ArregloDinamico`.

---

## 12. Pruebas

El proyecto incluye dos suites de integración en PowerShell que ejercitan la
API completa contra el servidor en ejecución: **635 comprobaciones**.

```powershell
.\build\dfa_server.exe          # en una terminal
.\server\pruebas_api.ps1        # en otra
.\server\pruebas_api2.ps1
```

Cobertura:

| Suite | Secciones |
|---|---|
| `pruebas_api.ps1` | Autómatas válidos, casos inválidos, entradas sucias, nombres y símbolos hostiles (Unicode, comillas, HTML, nombres de 300 caracteres), transiciones mal formadas, uniones, parámetros inválidos, prueba de cadenas simple y triple, rutas y métodos, carga y consistencia. |
| `pruebas_api2.ps1` | Barrido de los 94 caracteres ASCII imprimibles como símbolo, alfabetos grandes, verificación del lenguaje «binario divisible por 3» sobre 126 cadenas, comprobación exhaustiva de que la unión es exactamente el OR de los lenguajes, colisiones de nombres de par, combinaciones de errores, determinismo e idempotencia. |

Dos verificaciones destacan por lo que demuestran:

- **Lenguaje verificado**: se construye el DFA de los binarios divisibles por
  3 y se contrastan 126 cadenas contra el cálculo aritmético directo.
- **Unión = OR**: se enumeran todas las cadenas hasta cierta longitud y se
  comprueba que la unión acepta exactamente cuando lo hace M₁ o M₂.

---

## 13. Decisiones de diseño

**Inserción permisiva, validación centralizada.** `agregarEstado`,
`agregarSimbolo` y `agregarEstadoFinal` aceptan lo que reciben. La alternativa,
filtrar en la inserción, dejaría sin uso la mitad de las verificaciones del
Módulo 1: si un estado duplicado no llega a entrar en la estructura, el
validador no tiene nada que informar sobre él. Concentrando el criterio en un
solo punto, el validador es quien juzga y quien emite el diagnóstico
específico que pide el enunciado.

**δ como lista de tripletas, no como matriz.** Una matriz `estado × símbolo`
representa por construcción una función total y determinista, de modo que un
autómata incompleto o no determinista sería inexpresable y el Módulo 1 no
tendría nada que verificar. La lista plana permite representar esos casos y
que el validador los detecte y los nombre.

**Errores acumulados, no cortocircuito.** `validar` ejecuta las ocho
verificaciones siempre y devuelve todos los errores. Un autómata con cuatro
problemas los muestra los cuatro, en lugar de obligar a corregirlos de a uno.

**La unión también se valida.** El nombre de un estado compuesto se forma como
`(estado1,estado2)`, así que si los nombres originales contienen comas o
paréntesis dos pares distintos podrían producir el mismo nombre. El autómata
producto pasa por el validador antes de guardarse y, si no lo supera, la
operación se bloquea indicando el motivo. Es el mismo criterio del Módulo 1
aplicado a un autómata generado por el programa en vez de escrito por el
usuario.

**Una sola fuente de lógica.** La consola y el servidor consumen la misma
biblioteca `dfa_core`. Ninguna interfaz reimplementa una regla del autómata.

---

## 14. Alcance

**Símbolos de un carácter.** Σ se modela como un conjunto de `char`, que es la
representación directa del alfabeto en la definición formal: un símbolo es un
elemento atómico del alfabeto, no una cadena. Los símbolos reservados que
denotan la cadena vacía (ε, λ) se reconocen al leer la entrada y se rechazan
con un mensaje propio, tal como exige el Módulo 1.

**Modelo de sesión.** Los autómatas definidos se guardan en un
`ArregloDinamico<Automata>` y quedan disponibles para validar, unir y probar
cadenas durante toda la ejecución. Las dos interfaces comparten ese mismo
modelo.

**Búsqueda lineal en todas las estructuras.** Es consecuencia directa de la
restricción del proyecto: al no disponer de tablas hash ni de contenedores
ordenados, cada consulta se resuelve recorriendo el arreglo y comparando uno a
uno. Las cotas de la sección 11 son exactamente las de esa decisión, y cada
una de esas operaciones está escrita con bucles explícitos.
