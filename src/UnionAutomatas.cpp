#include "UnionAutomatas.h"

ResultadoUnion UnionAutomatas::unir(const Automata& automataA, const Automata& automataB) {
    ResultadoUnion resultado;

    ArregloDinamico<char> simbolosNoCoincidentes;
    if (!alfabetosCoinciden(automataA, automataB, simbolosNoCoincidentes)) {
        resultado.exitoso = false;
        std::string mensaje = "Los alfabetos no coinciden. Simbolos no compartidos: ";
        for (int i = 0; i < simbolosNoCoincidentes.tamano(); i++) {
            mensaje += simbolosNoCoincidentes.obtener(i);
            if (i < simbolosNoCoincidentes.tamano() - 1) {
                mensaje += ", ";
            }
        }
        resultado.mensajeError = mensaje;
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
    const ArregloDinamico<char>& alfabetoA = automataA.obtenerAlfabeto();
    const ArregloDinamico<char>& alfabetoB = automataB.obtenerAlfabeto();

    for (int i = 0; i < alfabetoA.tamano(); i++) {
        char simbolo = alfabetoA.obtener(i);
        if (!automataB.existeSimbolo(simbolo)) {
            simbolosNoCoincidentes.agregar(simbolo);
        }
    }
    for (int i = 0; i < alfabetoB.tamano(); i++) {
        char simbolo = alfabetoB.obtener(i);
        if (!automataA.existeSimbolo(simbolo)) {
            simbolosNoCoincidentes.agregar(simbolo);
        }
    }

    return simbolosNoCoincidentes.estaVacio();
}

Automata UnionAutomatas::construirAutomataUnion(const Automata& automataA, const Automata& automataB) {
    Automata resultado;
    resultado.establecerNombre("Union(" + automataA.obtenerNombre() + ", " + automataB.obtenerNombre() + ")");

    const ArregloDinamico<std::string>& estadosA = automataA.obtenerEstados();
    const ArregloDinamico<std::string>& estadosB = automataB.obtenerEstados();
    const ArregloDinamico<char>& alfabeto = automataA.obtenerAlfabeto();

    for (int i = 0; i < alfabeto.tamano(); i++) {
        resultado.agregarSimbolo(alfabeto.obtener(i));
    }

    for (int i = 0; i < estadosA.tamano(); i++) {
        for (int j = 0; j < estadosB.tamano(); j++) {
            resultado.agregarEstado(nombreEstadoPar(estadosA.obtener(i), estadosB.obtener(j)));
        }
    }

    resultado.establecerEstadoInicial(nombreEstadoPar(automataA.obtenerEstadoInicial(), automataB.obtenerEstadoInicial()));

    for (int i = 0; i < estadosA.tamano(); i++) {
        for (int j = 0; j < estadosB.tamano(); j++) {
            const std::string& estadoA = estadosA.obtener(i);
            const std::string& estadoB = estadosB.obtener(j);
            std::string estadoPar = nombreEstadoPar(estadoA, estadoB);

            if (automataA.esEstadoFinal(estadoA) || automataB.esEstadoFinal(estadoB)) {
                resultado.agregarEstadoFinal(estadoPar);
            }

            for (int k = 0; k < alfabeto.tamano(); k++) {
                char simbolo = alfabeto.obtener(k);
                std::string destinoA, destinoB;
                if (automataA.obtenerTransicion(estadoA, simbolo, destinoA) &&
                    automataB.obtenerTransicion(estadoB, simbolo, destinoB)) {
                    resultado.agregarTransicion(estadoPar, simbolo, nombreEstadoPar(destinoA, destinoB));
                }
            }
        }
    }

    return resultado;
}
