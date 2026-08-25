#include "ValidadorAutomata.h"

ResultadoValidacion ValidadorAutomata::validar(const Automata& automata) {
    ResultadoValidacion resultado;

    verificarUnicidadEstados(automata, resultado.errores);
    verificarUnicidadAlfabeto(automata, resultado.errores);
    verificarSimbologiaValida(automata, resultado.errores);
    verificarEstadoInicial(automata, resultado.errores);
    verificarEstadosFinales(automata, resultado.errores);
    verificarTransiciones(automata, resultado.errores);

    resultado.esValido = resultado.errores.estaVacio();
    return resultado;
}

void ValidadorAutomata::verificarUnicidadEstados(const Automata& automata, ArregloDinamico<std::string>& errores) {
    (void)automata;
    (void)errores;
}

void ValidadorAutomata::verificarUnicidadAlfabeto(const Automata& automata, ArregloDinamico<std::string>& errores) {
    (void)automata;
    (void)errores;
}

void ValidadorAutomata::verificarSimbologiaValida(const Automata& automata, ArregloDinamico<std::string>& errores) {
    (void)automata;
    (void)errores;
}

void ValidadorAutomata::verificarEstadoInicial(const Automata& automata, ArregloDinamico<std::string>& errores) {
    (void)automata;
    (void)errores;
}

void ValidadorAutomata::verificarEstadosFinales(const Automata& automata, ArregloDinamico<std::string>& errores) {
    (void)automata;
    (void)errores;
}

void ValidadorAutomata::verificarTransiciones(const Automata& automata, ArregloDinamico<std::string>& errores) {
    (void)automata;
    (void)errores;
}
