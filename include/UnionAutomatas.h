#ifndef UNION_AUTOMATAS_H
#define UNION_AUTOMATAS_H

#include <string>
#include "ArregloDinamico.h"
#include "Automata.h"

// Resultado de intentar unir dos automatas.
struct ResultadoUnion {
    bool exitoso;
    std::string mensajeError;
    Automata automataResultante;

    ResultadoUnion() : exitoso(false) {}
};

// Union de dos DFA previamente validados.
class UnionAutomatas {
public:
    static ResultadoUnion unir(const Automata& automataA, const Automata& automataB);

    // Nombre visible para un estado compuesto (qA, qB), ej: "(q0,p1)".
    static std::string nombreEstadoPar(const std::string& estadoA, const std::string& estadoB);

private:
    static bool alfabetosCoinciden(const Automata& automataA, const Automata& automataB,
                                    ArregloDinamico<char>& simbolosNoCoincidentes);

    static Automata construirAutomataUnion(const Automata& automataA, const Automata& automataB);
};

#endif // UNION_AUTOMATAS_H
