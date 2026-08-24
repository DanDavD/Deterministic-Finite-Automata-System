#ifndef UNION_AUTOMATAS_H
#define UNION_AUTOMATAS_H

#include <string>
#include "ArregloDinamico.h"
#include "Automata.h"

// Resultado de intentar unir dos automatas (Modulo 2 del enunciado).
struct ResultadoUnion {
    bool exitoso;
    std::string mensajeError;
    Automata automataResultante;

    ResultadoUnion() : exitoso(false) {}
};

// Modulo 2: Union de dos DFA previamente validados (punto 3, Modulo 2).
class UnionAutomatas {
public:
    // Valida compatibilidad de alfabetos y, si son compatibles, construye
    // el automata union por producto cartesiano de estados.
    static ResultadoUnion unir(const Automata& automataA, const Automata& automataB);

    // Nombre visible para un estado compuesto (qA, qB), ej: "(q0,p1)".
    // Provisto como utilidad de formato: no es logica de negocio.
    static std::string nombreEstadoPar(const std::string& estadoA, const std::string& estadoB);

private:
    // Recorre ambos alfabetos manualmente y verifica que contengan
    // exactamente los mismos simbolos. Si difieren, agrega a
    // "simbolosNoCoincidentes" los simbolos que sobran/faltan.
    // TODO(alumno): implementar (punto "Coincidencia de Alfabeto").
    static bool alfabetosCoinciden(const Automata& automataA, const Automata& automataB,
                                    ArregloDinamico<char>& simbolosNoCoincidentes);

    // TODO(alumno): construir el automata resultante:
    // 1) Generar todos los estados compuestos (estado_A, estado_B).
    // 2) Para cada estado compuesto y cada simbolo, calcular la transicion
    //    combinada consultando automataA.obtenerTransicion y
    //    automataB.obtenerTransicion.
    // 3) Marcar como final todo estado compuesto (qA, qB) donde qA sea
    //    final en A O qB sea final en B (criterio de disyuncion).
    // 4) El estado inicial del resultado es el par de estados iniciales.
    static Automata construirAutomataUnion(const Automata& automataA, const Automata& automataB);
};

#endif // UNION_AUTOMATAS_H
