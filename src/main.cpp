#include <iostream>
#include <string>
#include "ArregloDinamico.h"
#include "Automata.h"
#include "ValidadorAutomata.h"
#include "UnionAutomatas.h"

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

// epsilon y lambda ocupan dos bytes en utf-8, no entran en un char
static bool esReservadoMultibyte(const std::string& simbolo, std::string& nombre) {
    if (simbolo.size() != 2 || (unsigned char)simbolo[0] != 0xCE) {
        return false;
    }
    unsigned char segundo = (unsigned char)simbolo[1];
    if (segundo == 0xB5 || segundo == 0x95) {
        nombre = "epsilon";
        return true;
    }
    if (segundo == 0xBB || segundo == 0x9B) {
        nombre = "lambda";
        return true;
    }
    return false;
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
        // se lee como cadena y no como char: asi un caracter multibyte entra
        // entero y no deja bytes sueltos ensuciando la lectura siguiente
        std::string simbolo;
        std::cin >> simbolo;
        if (simbolo.size() == 1) {
            nuevo.agregarSimbolo(simbolo[0]);
            continue;
        }
        std::string reservado;
        if (esReservadoMultibyte(simbolo, reservado)) {
            std::cout << "    '" << simbolo << "' (" << reservado << ") representa la cadena vacia: "
                      << "es un simbolo reservado y no se agrega al alfabeto.\n";
        } else {
            std::cout << "    '" << simbolo << "' tiene mas de un caracter, se ignora.\n";
        }
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

    std::cout << "\n";
    ResultadoValidacion revision = ValidadorAutomata::validar(nuevo);
    imprimirResultadoValidacion(revision);
    if (!revision.esValido) {
        std::cout << "No se guarda ni queda disponible para la union.\n";
        return;
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

    // si los nombres de estado traen comas o parentesis, dos pares distintos
    // pueden terminar con el mismo nombre y el automata sale roto
    ResultadoValidacion revision = ValidadorAutomata::validar(resultado.automataResultante);
    if (!revision.esValido) {
        std::cout << "La union genero un automata invalido, no se guarda:\n";
        for (int i = 0; i < revision.errores.tamano(); i++) {
            std::cout << "  - " << revision.errores.obtener(i) << "\n";
        }
        return;
    }

    std::cout << "Union generada correctamente:\n";
    resultado.automataResultante.imprimir();
    automatas.agregar(resultado.automataResultante);
    std::cout << "El automata union se guardo en el indice " << (automatas.tamano() - 1) << ".\n";
}

// corre la cadena paso a paso; si traza no es nullptr guarda cada estado
static bool simularCadena(const Automata& automata, const std::string& cadena,
                          ArregloDinamico<std::string>* traza) {
    if (!automata.tieneEstadoInicial()) {
        return false;
    }
    std::string actual = automata.obtenerEstadoInicial();
    if (traza != nullptr) {
        traza->agregar(actual);
    }
    for (int i = 0; i < (int)cadena.size(); i++) {
        std::string siguiente;
        if (!automata.obtenerTransicion(actual, cadena[i], siguiente)) {
            return false;
        }
        actual = siguiente;
        if (traza != nullptr) {
            traza->agregar(actual);
        }
    }
    return automata.esEstadoFinal(actual);
}

static void probarCadenaMenu(const ArregloDinamico<Automata>& automatas) {
    std::cout << "\n--- Probar cadena ---\n";
    std::cout << "Se prueba sobre el automata union y sus dos originales por separado.\n";

    int indiceUnion = seleccionarAutomata(automatas, "automata union");
    if (indiceUnion == -1) {
        return;
    }
    int indiceA = seleccionarAutomata(automatas, "automata 1 original");
    if (indiceA == -1) {
        return;
    }
    int indiceB = seleccionarAutomata(automatas, "automata 2 original");
    if (indiceB == -1) {
        return;
    }

    std::cout << "Cadena a evaluar (- para cadena vacia): ";
    std::string cadena;
    std::cin >> cadena;
    if (cadena == "-") {
        cadena = "";
    }

    const Automata& automataUnion = automatas.obtener(indiceUnion);
    const Automata& automata1 = automatas.obtener(indiceA);
    const Automata& automata2 = automatas.obtener(indiceB);

    ArregloDinamico<std::string> traza;
    bool aceptaUnion = simularCadena(automataUnion, cadena, &traza);

    std::cout << "\nRecorrido sobre el automata union:\n";
    if (traza.estaVacio()) {
        std::cout << "  el automata union no tiene estado inicial\n";
    } else {
        std::cout << "  " << traza.obtener(0) << "\n";
        for (int i = 1; i < traza.tamano(); i++) {
            std::cout << "  --" << cadena[i - 1] << "--> " << traza.obtener(i) << "\n";
        }
        int consumidos = traza.tamano() - 1;
        if (consumidos < (int)cadena.size()) {
            std::cout << "  no hay transicion para '" << cadena[consumidos] << "', el recorrido se detiene\n";
        }
    }

    bool acepta1 = simularCadena(automata1, cadena, nullptr);
    bool acepta2 = simularCadena(automata2, cadena, nullptr);

    std::cout << "\nVeredicto para la cadena \"" << cadena << "\":\n";
    std::cout << "  Automata 1 (" << automata1.obtenerNombre() << "): " << (acepta1 ? "ACEPTADA" : "RECHAZADA") << "\n";
    std::cout << "  Automata 2 (" << automata2.obtenerNombre() << "): " << (acepta2 ? "ACEPTADA" : "RECHAZADA") << "\n";
    std::cout << "  Automata union (" << automataUnion.obtenerNombre() << "): " << (aceptaUnion ? "ACEPTADA" : "RECHAZADA") << "\n";
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
