#include "Automata.h"
#include <iostream>

Automata::Automata()
    : nombre(""), estadoInicial(""), hayEstadoInicial(false) {
}

void Automata::establecerNombre(const std::string& nombreNuevo) {
    nombre = nombreNuevo;
}

std::string Automata::obtenerNombre() const {
    return nombre;
}

bool Automata::agregarEstado(const std::string& estado) {
    if (existeEstado(estado)) {
        return false;
    }
    estados.agregar(estado);
    return true;
}

bool Automata::existeEstado(const std::string& estado) const {
    for (int i = 0; i < estados.tamano(); i++) {
        if (estados.obtener(i) == estado) {
            return true;
        }
    }
    return false;
}

const ArregloDinamico<std::string>& Automata::obtenerEstados() const {
    return estados;
}

bool Automata::agregarSimbolo(char simbolo) {
    // char es de un solo byte, por lo que epsilon/lambda (multibyte en
    // UTF-8) nunca llegan aqui completos; se rechazan los reservados que si
    // son representables como char: espacios en blanco y guion.
    if (simbolo == ' ' || simbolo == '\t' || simbolo == '\n' || simbolo == '\r' || simbolo == '-') {
        return false;
    }
    if (existeSimbolo(simbolo)) {
        return false;
    }
    alfabeto.agregar(simbolo);
    return true;
}

bool Automata::existeSimbolo(char simbolo) const {
    for (int i = 0; i < alfabeto.tamano(); i++) {
        if (alfabeto.obtener(i) == simbolo) {
            return true;
        }
    }
    return false;
}

const ArregloDinamico<char>& Automata::obtenerAlfabeto() const {
    return alfabeto;
}

void Automata::establecerEstadoInicial(const std::string& estado) {
    estadoInicial = estado;
    hayEstadoInicial = true;
}

std::string Automata::obtenerEstadoInicial() const {
    return estadoInicial;
}

bool Automata::tieneEstadoInicial() const {
    return hayEstadoInicial;
}

bool Automata::agregarEstadoFinal(const std::string& estado) {
    if (!existeEstado(estado)) {
        return false;
    }
    estadosFinales.agregar(estado);
    return true;
}

bool Automata::esEstadoFinal(const std::string& estado) const {
    for (int i = 0; i < estadosFinales.tamano(); i++) {
        if (estadosFinales.obtener(i) == estado) {
            return true;
        }
    }
    return false;
}

const ArregloDinamico<std::string>& Automata::obtenerEstadosFinales() const {
    return estadosFinales;
}

bool Automata::agregarTransicion(const std::string& origen, char simbolo, const std::string& destino) {
    transiciones.agregar(Transicion(origen, simbolo, destino));
    return true;
}

bool Automata::obtenerTransicion(const std::string& origen, char simbolo, std::string& destinoResultado) const {
    for (int i = 0; i < transiciones.tamano(); i++) {
        const Transicion& t = transiciones.obtener(i);
        if (t.origen == origen && t.simbolo == simbolo) {
            destinoResultado = t.destino;
            return true;
        }
    }
    return false;
}

const ArregloDinamico<Transicion>& Automata::obtenerTransiciones() const {
    return transiciones;
}

void Automata::imprimir() const {
    std::cout << "Automata: " << nombre << "\n";

    std::cout << "Estados: ";
    for (int i = 0; i < estados.tamano(); i++) {
        std::cout << estados.obtener(i) << " ";
    }
    std::cout << "\n";

    std::cout << "Alfabeto: ";
    for (int i = 0; i < alfabeto.tamano(); i++) {
        std::cout << alfabeto.obtener(i) << " ";
    }
    std::cout << "\n";

    std::cout << "Estado inicial: " << (hayEstadoInicial ? estadoInicial : std::string("(sin definir)")) << "\n";

    std::cout << "Estados finales: ";
    for (int i = 0; i < estadosFinales.tamano(); i++) {
        std::cout << estadosFinales.obtener(i) << " ";
    }
    std::cout << "\n";

    std::cout << "Transiciones:\n";
    for (int i = 0; i < transiciones.tamano(); i++) {
        const Transicion& t = transiciones.obtener(i);
        std::cout << "  (" << t.origen << ", " << t.simbolo << ") -> " << t.destino << "\n";
    }
}
