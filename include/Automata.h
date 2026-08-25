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
class Automata {
public:
    Automata();

    // --- Identificacion ---
    void establecerNombre(const std::string& nombre);
    std::string obtenerNombre() const;

    // --- Estados ---
    bool agregarEstado(const std::string& estado);
    bool existeEstado(const std::string& estado) const;
    const ArregloDinamico<std::string>& obtenerEstados() const;

    // --- Alfabeto ---
    bool agregarSimbolo(char simbolo);
    bool existeSimbolo(char simbolo) const;
    const ArregloDinamico<char>& obtenerAlfabeto() const;

    // --- Estado inicial ---
    void establecerEstadoInicial(const std::string& estado);
    std::string obtenerEstadoInicial() const;
    bool tieneEstadoInicial() const;

    // --- Estados finales ---
    bool agregarEstadoFinal(const std::string& estado);
    bool esEstadoFinal(const std::string& estado) const;
    const ArregloDinamico<std::string>& obtenerEstadosFinales() const;

    // --- Transiciones ---
    bool agregarTransicion(const std::string& origen, char simbolo, const std::string& destino);
    bool obtenerTransicion(const std::string& origen, char simbolo, std::string& destinoResultado) const;
    const ArregloDinamico<Transicion>& obtenerTransiciones() const;

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
