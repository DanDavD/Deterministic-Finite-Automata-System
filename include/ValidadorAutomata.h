#ifndef VALIDADOR_AUTOMATA_H
#define VALIDADOR_AUTOMATA_H

#include <string>
#include "ArregloDinamico.h"
#include "Automata.h"

// resultado de validar un automata: si es valido, y en caso contrario, el
// listado detallado de errores encontrados
struct ResultadoValidacion {
    bool esValido;
    ArregloDinamico<std::string> errores;

    ResultadoValidacion() : esValido(false) {}
};

// validacion de dfa
class ValidadorAutomata {
public:
    static ResultadoValidacion validar(const Automata& automata);

private:
    static void verificarUnicidadEstados(const Automata& automata, ArregloDinamico<std::string>& errores);
    static void verificarUnicidadAlfabeto(const Automata& automata, ArregloDinamico<std::string>& errores);
    static void verificarSimbologiaValida(const Automata& automata, ArregloDinamico<std::string>& errores);
    static void verificarEstadoInicial(const Automata& automata, ArregloDinamico<std::string>& errores);
    static void verificarEstadosFinales(const Automata& automata, ArregloDinamico<std::string>& errores);
    static void verificarTransiciones(const Automata& automata, ArregloDinamico<std::string>& errores);
};

#endif // VALIDADOR_AUTOMATA_H
