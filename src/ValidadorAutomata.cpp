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
    // TODO(alumno): recorrer automata.obtenerEstados() con dos indices
    // (i, j) comparando manualmente cada par para detectar nombres
    // repetidos. Agregar un error por cada duplicado encontrado.
    (void)automata;
    (void)errores;
}

void ValidadorAutomata::verificarUnicidadAlfabeto(const Automata& automata, ArregloDinamico<std::string>& errores) {
    // TODO(alumno): igual que arriba pero sobre automata.obtenerAlfabeto().
    (void)automata;
    (void)errores;
}

void ValidadorAutomata::verificarSimbologiaValida(const Automata& automata, ArregloDinamico<std::string>& errores) {
    // TODO(alumno): recorrer el alfabeto y rechazar simbolos reservados o
    // nulos (epsilon, lambda, espacio en blanco, guion...).
    (void)automata;
    (void)errores;
}

void ValidadorAutomata::verificarEstadoInicial(const Automata& automata, ArregloDinamico<std::string>& errores) {
    // TODO(alumno): validar que automata.tieneEstadoInicial() sea true y
    // que automata.obtenerEstadoInicial() exista dentro de
    // automata.obtenerEstados() (busqueda manual).
    (void)automata;
    (void)errores;
}

void ValidadorAutomata::verificarEstadosFinales(const Automata& automata, ArregloDinamico<std::string>& errores) {
    // TODO(alumno): recorrer automata.obtenerEstadosFinales() y verificar,
    // uno a uno, que cada estado pertenezca a automata.obtenerEstados().
    // Un conjunto de estados finales vacio es valido.
    (void)automata;
    (void)errores;
}

void ValidadorAutomata::verificarTransiciones(const Automata& automata, ArregloDinamico<std::string>& errores) {
    // TODO(alumno):
    // 1) Completitud: para cada estado y cada simbolo del alfabeto,
    //    verificar que exista una transicion definida (recorriendo
    //    automata.obtenerTransiciones() manualmente, o usando
    //    automata.obtenerTransicion una vez este implementado).
    // 2) Integridad de destino: el estado destino de cada transicion debe
    //    pertenecer a automata.obtenerEstados().
    // 3) Determinismo: no debe haber mas de una transicion definida para
    //    el mismo par (estado, simbolo).
    (void)automata;
    (void)errores;
}
