#include "ValidadorAutomata.h"

ResultadoValidacion ValidadorAutomata::validar(const Automata& automata) {
    ResultadoValidacion resultado;

    verificarNoVacuidad(automata, resultado.errores);
    verificarUnicidadEstados(automata, resultado.errores);
    verificarUnicidadAlfabeto(automata, resultado.errores);
    verificarSimbologiaValida(automata, resultado.errores);
    verificarEstadoInicial(automata, resultado.errores);
    verificarEstadosFinales(automata, resultado.errores);
    verificarTransiciones(automata, resultado.errores);

    resultado.esValido = resultado.errores.estaVacio();
    return resultado;
}

void ValidadorAutomata::verificarNoVacuidad(const Automata& automata, ArregloDinamico<std::string>& errores) {
    if (automata.obtenerEstados().tamano() == 0) {
        errores.agregar("el conjunto de estados esta vacio");
    }
    if (automata.obtenerAlfabeto().tamano() == 0) {
        errores.agregar("el alfabeto esta vacio");
    }
}

void ValidadorAutomata::verificarUnicidadEstados(const Automata& automata, ArregloDinamico<std::string>& errores) {
    const ArregloDinamico<std::string>& estados = automata.obtenerEstados();
    ArregloDinamico<std::string> yaReportados;

    for (int i = 0; i < estados.tamano(); i++) {
        for (int j = i + 1; j < estados.tamano(); j++) {
            if (estados.obtener(i) != estados.obtener(j)) {
                continue;
            }
            bool reportado = false;
            for (int k = 0; k < yaReportados.tamano(); k++) {
                if (yaReportados.obtener(k) == estados.obtener(i)) {
                    reportado = true;
                    break;
                }
            }
            if (!reportado) {
                errores.agregar("el estado '" + estados.obtener(i) + "' esta duplicado en el conjunto de estados");
                yaReportados.agregar(estados.obtener(i));
            }
            break;
        }
    }
}

void ValidadorAutomata::verificarUnicidadAlfabeto(const Automata& automata, ArregloDinamico<std::string>& errores) {
    const ArregloDinamico<char>& alfabeto = automata.obtenerAlfabeto();
    ArregloDinamico<char> yaReportados;

    for (int i = 0; i < alfabeto.tamano(); i++) {
        for (int j = i + 1; j < alfabeto.tamano(); j++) {
            if (alfabeto.obtener(i) != alfabeto.obtener(j)) {
                continue;
            }
            bool reportado = false;
            for (int k = 0; k < yaReportados.tamano(); k++) {
                if (yaReportados.obtener(k) == alfabeto.obtener(i)) {
                    reportado = true;
                    break;
                }
            }
            if (!reportado) {
                errores.agregar(std::string("el simbolo '") + alfabeto.obtener(i) + "' esta duplicado en el alfabeto");
                yaReportados.agregar(alfabeto.obtener(i));
            }
            break;
        }
    }
}

void ValidadorAutomata::verificarSimbologiaValida(const Automata& automata, ArregloDinamico<std::string>& errores) {
    // char es de 1 byte asi que epsilon/lambda (utf-8 multibyte) no llegan
    // completos aca; se frenan antes, al leer la entrada
    const ArregloDinamico<char>& alfabeto = automata.obtenerAlfabeto();

    for (int i = 0; i < alfabeto.tamano(); i++) {
        char simbolo = alfabeto.obtener(i);
        std::string descripcion;
        if (simbolo == ' ') {
            descripcion = "un espacio en blanco";
        } else if (simbolo == '\t') {
            descripcion = "una tabulacion";
        } else if (simbolo == '\n') {
            descripcion = "un salto de linea";
        } else if (simbolo == '\r') {
            descripcion = "un retorno de carro";
        } else if (simbolo == '-') {
            descripcion = "el guion '-'";
        } else if (simbolo == '\0') {
            descripcion = "un caracter nulo";
        } else {
            continue;
        }
        errores.agregar("el alfabeto contiene " + descripcion + ", que es un simbolo reservado y no puede usarse");
    }
}

void ValidadorAutomata::verificarEstadoInicial(const Automata& automata, ArregloDinamico<std::string>& errores) {
    if (!automata.tieneEstadoInicial()) {
        errores.agregar("no se definio un estado inicial");
        return;
    }
    if (!automata.existeEstado(automata.obtenerEstadoInicial())) {
        errores.agregar("el estado inicial '" + automata.obtenerEstadoInicial() + "' no pertenece al conjunto de estados");
    }
}

void ValidadorAutomata::verificarEstadosFinales(const Automata& automata, ArregloDinamico<std::string>& errores) {
    const ArregloDinamico<std::string>& finales = automata.obtenerEstadosFinales();

    for (int i = 0; i < finales.tamano(); i++) {
        if (!automata.existeEstado(finales.obtener(i))) {
            errores.agregar("el estado final '" + finales.obtener(i) + "' no pertenece al conjunto de estados");
        }
    }
}

void ValidadorAutomata::verificarTransiciones(const Automata& automata, ArregloDinamico<std::string>& errores) {
    const ArregloDinamico<std::string>& estados = automata.obtenerEstados();
    const ArregloDinamico<char>& alfabeto = automata.obtenerAlfabeto();
    const ArregloDinamico<Transicion>& transiciones = automata.obtenerTransiciones();

    for (int i = 0; i < transiciones.tamano(); i++) {
        const Transicion& t = transiciones.obtener(i);
        if (!automata.existeEstado(t.destino)) {
            errores.agregar("el estado de destino '" + t.destino + "' no esta registrado en el conjunto de estados");
        }
    }

    for (int i = 0; i < estados.tamano(); i++) {
        const std::string& estado = estados.obtener(i);
        for (int j = 0; j < alfabeto.tamano(); j++) {
            char simbolo = alfabeto.obtener(j);
            int cantidadDestinos = 0;
            for (int k = 0; k < transiciones.tamano(); k++) {
                const Transicion& t = transiciones.obtener(k);
                if (t.origen == estado && t.simbolo == simbolo) {
                    cantidadDestinos++;
                }
            }
            if (cantidadDestinos == 0) {
                errores.agregar("el estado '" + estado + "' carece de transicion para el simbolo '" + std::string(1, simbolo) + "'");
            } else if (cantidadDestinos > 1) {
                errores.agregar("el estado '" + estado + "' tiene mas de una transicion definida para el simbolo '" + std::string(1, simbolo) + "'");
            }
        }
    }
}
