#include "UnionAutomatas.h"

ResultadoUnion UnionAutomatas::unir(const Automata& automataA, const Automata& automataB) {
    ResultadoUnion resultado;

    ArregloDinamico<char> simbolosNoCoincidentes;
    if (!alfabetosCoinciden(automataA, automataB, simbolosNoCoincidentes)) {
        resultado.exitoso = false;
        resultado.mensajeError = "Los alfabetos de ambos automatas no coinciden.";
        return resultado;
    }

    resultado.automataResultante = construirAutomataUnion(automataA, automataB);
    resultado.exitoso = true;
    return resultado;
}

std::string UnionAutomatas::nombreEstadoPar(const std::string& estadoA, const std::string& estadoB) {
    return "(" + estadoA + "," + estadoB + ")";
}

bool UnionAutomatas::alfabetosCoinciden(const Automata& automataA, const Automata& automataB,
                                         ArregloDinamico<char>& simbolosNoCoincidentes) {
    (void)automataA;
    (void)automataB;
    (void)simbolosNoCoincidentes;
    return false;
}

Automata UnionAutomatas::construirAutomataUnion(const Automata& automataA, const Automata& automataB) {
    Automata resultado;
    resultado.establecerNombre("Union(" + automataA.obtenerNombre() + ", " + automataB.obtenerNombre() + ")");
    return resultado;
}
