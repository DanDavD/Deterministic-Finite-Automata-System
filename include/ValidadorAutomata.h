#ifndef VALIDADOR_AUTOMATA_H
#define VALIDADOR_AUTOMATA_H

#include <string>
#include "ArregloDinamico.h"
#include "Automata.h"

// Resultado de validar un automata: si es valido, y en caso contrario, el
// listado detallado de errores encontrados (punto 3.4 del enunciado).
struct ResultadoValidacion {
    bool esValido;
    ArregloDinamico<std::string> errores;

    ResultadoValidacion() : esValido(false) {}
};

// Modulo 1: Validacion estricta de un DFA (punto 3 del enunciado).
//
// TODO(alumno): implementar cada verificacion usando UNICAMENTE recorridos
// y busquedas manuales sobre las estructuras de Automata (nada de
// std::find, std::set, expresiones regulares, etc.). Cada verificacion
// debe agregar un mensaje de error especifico y legible a "errores" cuando
// encuentre un problema (ej: "El estado 'q2' carece de transicion para el
// simbolo 'b'").
class ValidadorAutomata {
public:
    // Ejecuta todas las verificaciones y arma el ResultadoValidacion final.
    static ResultadoValidacion validar(const Automata& automata);

private:
    // 3.1 Unicidad de nombres de estados.
    static void verificarUnicidadEstados(const Automata& automata, ArregloDinamico<std::string>& errores);

    // 3.1 Unicidad de simbolos del alfabeto.
    static void verificarUnicidadAlfabeto(const Automata& automata, ArregloDinamico<std::string>& errores);

    // 3.1 Simbologia valida (rechazar epsilon, lambda, espacios, guiones...).
    static void verificarSimbologiaValida(const Automata& automata, ArregloDinamico<std::string>& errores);

    // 3.2 Debe existir exactamente un estado inicial y pertenecer a estados.
    static void verificarEstadoInicial(const Automata& automata, ArregloDinamico<std::string>& errores);

    // 3.2 Todo estado final debe pertenecer al conjunto de estados.
    static void verificarEstadosFinales(const Automata& automata, ArregloDinamico<std::string>& errores);

    // 3.3 Completitud (delta total), integridad de destino, y
    // prohibicion de no-determinismo.
    static void verificarTransiciones(const Automata& automata, ArregloDinamico<std::string>& errores);
};

#endif // VALIDADOR_AUTOMATA_H
