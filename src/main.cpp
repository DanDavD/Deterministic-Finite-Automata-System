#include <iostream>
#include <string>
#include "ArregloDinamico.h"
#include "Automata.h"
#include "ValidadorAutomata.h"
#include "UnionAutomatas.h"

// Punto de entrada: menu de consola. La navegacion y la entrada/salida de
// datos ya estan implementadas para que puedas concentrarte en la logica
// pedida por el enunciado (ver los TODO en Automata.cpp, ValidadorAutomata.cpp
// y UnionAutomatas.cpp).

static void mostrarMenuPrincipal() {
    std::cout << "\n===== Sistema de Automatas Finitos Deterministas =====\n";
    std::cout << "1. Crear automata\n";
    std::cout << "2. Listar automatas guardados\n";
    std::cout << "3. Validar un automata\n";
    std::cout << "4. Unir dos automatas\n";
    std::cout << "5. Probar una cadena\n";
    std::cout << "0. Salir\n";
    std::cout << "Seleccione una opcion: ";
}

static int leerEntero() {
    int valor;
    while (!(std::cin >> valor)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Entrada invalida, intente de nuevo: ";
    }
    return valor;
}

static void crearAutomata(ArregloDinamico<Automata>& automatas) {
    Automata nuevo;

    std::cout << "\n--- Crear automata ---\n";
    std::cout << "Nombre del automata: ";
    std::string nombre;
    std::cin >> nombre;
    nuevo.establecerNombre(nombre);

    std::cout << "Cantidad de estados: ";
    int cantidadEstados = leerEntero();
    for (int i = 0; i < cantidadEstados; i++) {
        std::cout << "  Estado " << (i + 1) << ": ";
        std::string estado;
        std::cin >> estado;
        nuevo.agregarEstado(estado);
    }

    std::cout << "Cantidad de simbolos del alfabeto: ";
    int cantidadSimbolos = leerEntero();
    for (int i = 0; i < cantidadSimbolos; i++) {
        std::cout << "  Simbolo " << (i + 1) << ": ";
        char simbolo;
        std::cin >> simbolo;
        nuevo.agregarSimbolo(simbolo);
    }

    std::cout << "Estado inicial: ";
    std::string inicial;
    std::cin >> inicial;
    nuevo.establecerEstadoInicial(inicial);

    std::cout << "Cantidad de estados de aceptacion: ";
    int cantidadFinales = leerEntero();
    for (int i = 0; i < cantidadFinales; i++) {
        std::cout << "  Estado final " << (i + 1) << ": ";
        std::string estadoFinal;
        std::cin >> estadoFinal;
        nuevo.agregarEstadoFinal(estadoFinal);
    }

    std::cout << "Cantidad de transiciones: ";
    int cantidadTransiciones = leerEntero();
    for (int i = 0; i < cantidadTransiciones; i++) {
        std::cout << "  Transicion " << (i + 1) << " (origen simbolo destino): ";
        std::string origen, destino;
        char simbolo;
        std::cin >> origen >> simbolo >> destino;
        nuevo.agregarTransicion(origen, simbolo, destino);
    }

    automatas.agregar(nuevo);
    std::cout << "Automata '" << nombre << "' guardado (indice " << (automatas.tamano() - 1) << ").\n";
}

static void listarAutomatas(const ArregloDinamico<Automata>& automatas) {
    std::cout << "\n--- Automatas guardados ---\n";
    if (automatas.estaVacio()) {
        std::cout << "No hay automatas guardados todavia.\n";
        return;
    }
    for (int i = 0; i < automatas.tamano(); i++) {
        std::cout << "[" << i << "] " << automatas.obtener(i).obtenerNombre() << "\n";
    }
}

static int seleccionarAutomata(const ArregloDinamico<Automata>& automatas, const std::string& etiqueta) {
    listarAutomatas(automatas);
    if (automatas.estaVacio()) {
        return -1;
    }
    std::cout << "Indice del automata (" << etiqueta << "): ";
    int indice = leerEntero();
    if (indice < 0 || indice >= automatas.tamano()) {
        std::cout << "Indice fuera de rango.\n";
        return -1;
    }
    return indice;
}

static void imprimirResultadoValidacion(const ResultadoValidacion& resultado) {
    if (resultado.esValido) {
        std::cout << "El automata es VALIDO.\n";
        return;
    }
    std::cout << "El automata NO es valido. Errores encontrados:\n";
    for (int i = 0; i < resultado.errores.tamano(); i++) {
        std::cout << "  - " << resultado.errores.obtener(i) << "\n";
    }
}

static void validarAutomataMenu(const ArregloDinamico<Automata>& automatas) {
    std::cout << "\n--- Validar automata ---\n";
    int indice = seleccionarAutomata(automatas, "a validar");
    if (indice == -1) {
        return;
    }
    ResultadoValidacion resultado = ValidadorAutomata::validar(automatas.obtener(indice));
    imprimirResultadoValidacion(resultado);
}

static void unirAutomatasMenu(ArregloDinamico<Automata>& automatas) {
    std::cout << "\n--- Unir dos automatas ---\n";
    int indiceA = seleccionarAutomata(automatas, "primer automata (A)");
    if (indiceA == -1) {
        return;
    }
    int indiceB = seleccionarAutomata(automatas, "segundo automata (B)");
    if (indiceB == -1) {
        return;
    }

    ResultadoUnion resultado = UnionAutomatas::unir(automatas.obtener(indiceA), automatas.obtener(indiceB));
    if (!resultado.exitoso) {
        std::cout << "No se pudo unir: " << resultado.mensajeError << "\n";
        return;
    }

    std::cout << "Union generada correctamente:\n";
    resultado.automataResultante.imprimir();
    automatas.agregar(resultado.automataResultante);
    std::cout << "El automata union se guardo en el indice " << (automatas.tamano() - 1) << ".\n";
}

static void probarCadenaMenu(const ArregloDinamico<Automata>& automatas) {
    std::cout << "\n--- Probar cadena ---\n";
    int indice = seleccionarAutomata(automatas, "automata sobre el que se probara la cadena");
    if (indice == -1) {
        return;
    }

    std::cout << "Cadena a evaluar: ";
    std::string cadena;
    std::cin >> cadena;

    // TODO(alumno): implementar la simulacion paso a paso (Modulo de
    // Prueba e Inspeccion de Cadenas, punto 4.2):
    // 1) Partir del estado inicial del automata seleccionado.
    // 2) Por cada simbolo de "cadena", buscar la transicion con
    //    automata.obtenerTransicion(estadoActual, simbolo, siguiente) e
    //    imprimir el paso (estado actual -> estado siguiente).
    // 3) Al terminar, indicar si el estado alcanzado es final
    //    (automata.esEstadoFinal) => cadena aceptada o rechazada.
    // Si el automata seleccionado es el automata union, ademas deberias
    // mostrar el veredicto para los dos automatas originales (punto 4.2,
    // "Veredicto de Aceptacion Triple").
    (void)automatas;
    (void)indice;
    std::cout << "TODO: logica de simulacion de cadena aun no implementada.\n";
}

int main() {
    ArregloDinamico<Automata> automatas;

    bool salir = false;
    while (!salir) {
        mostrarMenuPrincipal();
        int opcion = leerEntero();

        switch (opcion) {
            case 1:
                crearAutomata(automatas);
                break;
            case 2:
                listarAutomatas(automatas);
                break;
            case 3:
                validarAutomataMenu(automatas);
                break;
            case 4:
                unirAutomatasMenu(automatas);
                break;
            case 5:
                probarCadenaMenu(automatas);
                break;
            case 0:
                salir = true;
                break;
            default:
                std::cout << "Opcion invalida.\n";
        }
    }

    std::cout << "Hasta luego.\n";
    return 0;
}
