#include "UnionAutomatas.h"

ResultadoUnion UnionAutomatas::unir(const Automata& automataA, const Automata& automataB) {
    ResultadoUnion resultado;

    ArregloDinamico<char> simbolosNoCoincidentes;
    if (!alfabetosCoinciden(automataA, automataB, simbolosNoCoincidentes)) {
        resultado.exitoso = false;
        // TODO(alumno): construir un mensaje mas detallado listando
        // "simbolosNoCoincidentes" (recorriendo el arreglo manualmente).
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
    // TODO(alumno): recorrer automataA.obtenerAlfabeto() y verificar (con
    // busqueda manual, por ejemplo usando automataB.existeSimbolo una vez
    // este implementado) que cada simbolo tambien este en el alfabeto de
    // B, y viceversa. Agregar a "simbolosNoCoincidentes" cada simbolo que
    // no coincida.
    (void)automataA;
    (void)automataB;
    (void)simbolosNoCoincidentes;
    return false;
}

Automata UnionAutomatas::construirAutomataUnion(const Automata& automataA, const Automata& automataB) {
    // TODO(alumno): ver los pasos detallados en UnionAutomatas.h.
    Automata resultado;
    resultado.establecerNombre("Union(" + automataA.obtenerNombre() + ", " + automataB.obtenerNombre() + ")");
    return resultado;
}
