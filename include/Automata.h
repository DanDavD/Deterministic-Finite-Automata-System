#ifndef AUTOMATA_H
#define AUTOMATA_H

#include <string>
#include "ArregloDinamico.h"

// Representa una transicion (estadoOrigen, simbolo) -> estadoDestino.
struct Transicion {
    std::string origen;
    char simbolo;
    std::string destino;

    Transicion() : origen(""), simbolo('\0'), destino("") {}
    Transicion(const std::string& o, char s, const std::string& d)
        : origen(o), simbolo(s), destino(d) {}
};

// Representa un Automata Finito Determinista (DFA) usando unicamente
// estructuras de datos propias (ArregloDinamico), sin STL containers.
//
// Esta clase solo se encarga de ALMACENAR la informacion del automata
// (estados, alfabeto, estado inicial, estados finales, transiciones) y de
// dar operaciones basicas de insercion/consulta. La logica de VALIDACION
// (unicidad, totalidad, determinismo, etc.) vive en ValidadorAutomata, y la
// logica de UNION vive en UnionAutomatas, tal como pide el enunciado.
//
// TODO(alumno): varios metodos de esta clase estan marcados como TODO.
// Debes implementarlos con tus propios algoritmos de busqueda/recorrido
// (sin usar funciones como std::find, .contains(), etc.), siguiendo el
// punto 3 del enunciado ("Verificacion de Alfabeto y Conjunto de Estados",
// "Verificacion de Estado Inicial...", etc.).
class Automata {
public:
    Automata();

    // --- Identificacion ---
    void establecerNombre(const std::string& nombre);
    std::string obtenerNombre() const;

    // --- Estados ---
    // Agrega un estado al arreglo de estados.
    // TODO(alumno): antes de agregar, verifica con un algoritmo propio
    // (recorrido manual) que el estado no exista ya (unicidad, punto 3.1).
    // Si ya existe, no debe agregarse de nuevo.
    bool agregarEstado(const std::string& estado);

    // TODO(alumno): implementar busqueda manual (recorrer el arreglo de
    // estados comparando uno a uno) que retorne true si "estado" existe.
    bool existeEstado(const std::string& estado) const;

    const ArregloDinamico<std::string>& obtenerEstados() const;

    // --- Alfabeto ---
    // TODO(alumno): validar que el simbolo no sea invalido (epsilon,
    // lambda, espacio en blanco, guion, etc. - punto 3.1 "Simbologia
    // valida") y que no este duplicado antes de agregarlo.
    bool agregarSimbolo(char simbolo);

    // TODO(alumno): busqueda manual sobre el arreglo de simbolos.
    bool existeSimbolo(char simbolo) const;

    const ArregloDinamico<char>& obtenerAlfabeto() const;

    // --- Estado inicial ---
    void establecerEstadoInicial(const std::string& estado);
    std::string obtenerEstadoInicial() const;
    bool tieneEstadoInicial() const;

    // --- Estados finales ---
    // TODO(alumno): validar (punto 3.2) que "estado" pertenezca al
    // conjunto de estados antes de marcarlo como final.
    bool agregarEstadoFinal(const std::string& estado);

    // TODO(alumno): busqueda manual sobre el arreglo de estados finales.
    bool esEstadoFinal(const std::string& estado) const;

    const ArregloDinamico<std::string>& obtenerEstadosFinales() const;

    // --- Transiciones ---
    // Agrega una transicion (origen, simbolo) -> destino a la estructura.
    bool agregarTransicion(const std::string& origen, char simbolo, const std::string& destino);

    // TODO(alumno): recorrer manualmente el arreglo de transiciones
    // buscando la que coincide con (origen, simbolo). Si se encuentra,
    // copiar el destino en "destinoResultado" y retornar true; si no,
    // retornar false. Esta funcion es clave para el punto 3.3
    // (completitud/determinismo) y para el Modulo 2 (union) y el modulo
    // de prueba de cadenas.
    bool obtenerTransicion(const std::string& origen, char simbolo, std::string& destinoResultado) const;

    const ArregloDinamico<Transicion>& obtenerTransiciones() const;

    // Imprime una tabla simple con estados, alfabeto, inicial, finales y
    // transiciones. Punto 4.1 del enunciado (Visualizacion).
    // TODO(alumno): dar formato de tabla / marcar visualmente el estado
    // inicial y los estados de aceptacion como pide el punto 4.1.
    void imprimir() const;

private:
    std::string nombre;
    ArregloDinamico<std::string> estados;
    ArregloDinamico<char> alfabeto;
    std::string estadoInicial;
    bool hayEstadoInicial;
    ArregloDinamico<std::string> estadosFinales;
    ArregloDinamico<Transicion> transiciones;
};

#endif // AUTOMATA_H
