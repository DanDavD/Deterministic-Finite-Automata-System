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
    // TODO(alumno): usar existeEstado(estado) para rechazar duplicados
    // antes de agregar (punto 3.1 - Unicidad). Por ahora se agrega sin
    // verificar, para que el proyecto compile y sirva de punto de partida.
    estados.agregar(estado);
    return true;
}

bool Automata::existeEstado(const std::string& estado) const {
    // TODO(alumno): implementar busqueda manual (recorrido con for)
    // sobre "estados" comparando cada elemento con "estado".
    (void)estado;
    return false;
}

const ArregloDinamico<std::string>& Automata::obtenerEstados() const {
    return estados;
}

bool Automata::agregarSimbolo(char simbolo) {
    // TODO(alumno): rechazar simbolos invalidos (epsilon, lambda, espacio,
    // guion) y duplicados usando existeSimbolo (punto 3.1).
    alfabeto.agregar(simbolo);
    return true;
}

bool Automata::existeSimbolo(char simbolo) const {
    // TODO(alumno): busqueda manual sobre "alfabeto".
    (void)simbolo;
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
    // TODO(alumno): validar que "estado" pertenezca a "estados" (punto 3.2)
    // antes de agregarlo a estadosFinales.
    estadosFinales.agregar(estado);
    return true;
}

bool Automata::esEstadoFinal(const std::string& estado) const {
    // TODO(alumno): busqueda manual sobre "estadosFinales".
    (void)estado;
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
    // TODO(alumno): recorrer "transiciones" manualmente buscando la
    // combinacion (origen, simbolo). Necesario para validar totalidad y
    // determinismo, para la union de automatas y para probar cadenas.
    (void)origen;
    (void)simbolo;
    (void)destinoResultado;
    return false;
}

const ArregloDinamico<Transicion>& Automata::obtenerTransiciones() const {
    return transiciones;
}

void Automata::imprimir() const {
    // TODO(alumno): mejorar el formato (tabla, marcar estado inicial y
    // estados de aceptacion) segun el punto 4.1 del enunciado.
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
