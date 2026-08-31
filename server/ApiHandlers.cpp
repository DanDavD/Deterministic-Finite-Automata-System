#include "ApiHandlers.h"

#include <mutex>
#include <string>

#include "ArregloDinamico.h"
#include "Automata.h"
#include "ValidadorAutomata.h"
#include "UnionAutomatas.h"

namespace {

// los automatas viven aqui mientras el servidor este arriba, igual que en el
// menu de consola. el navegador abre varias conexiones a la vez, asi que cada
// handler toma el candado antes de tocar el almacen.
ArregloDinamico<Automata> almacen;
std::mutex candado;

// --- armado de json a mano ---

std::string escaparJson(const std::string& texto) {
    std::string salida;
    for (int i = 0; i < (int)texto.size(); i++) {
        char c = texto[i];
        if (c == '"') {
            salida += "\\\"";
        } else if (c == '\\') {
            salida += "\\\\";
        } else if (c == '\n') {
            salida += "\\n";
        } else if (c == '\r') {
            salida += "\\r";
        } else if (c == '\t') {
            salida += "\\t";
        } else if (c >= 0 && c < 32) {
            salida += ' ';
        } else {
            salida += c;
        }
    }
    return salida;
}

std::string texto(const std::string& valor) {
    return "\"" + escaparJson(valor) + "\"";
}

std::string desdeChar(char c) {
    std::string s;
    s += c;
    return s;
}

// sin sstream ni to_string, se arma digito por digito
std::string numero(int valor) {
    if (valor == 0) {
        return "0";
    }
    bool negativo = valor < 0;
    if (negativo) {
        valor = -valor;
    }
    std::string alReves;
    while (valor > 0) {
        alReves += (char)('0' + (valor % 10));
        valor /= 10;
    }
    std::string salida;
    if (negativo) {
        salida += '-';
    }
    for (int i = (int)alReves.size() - 1; i >= 0; i--) {
        salida += alReves[i];
    }
    return salida;
}

std::string booleano(bool valor) {
    return valor ? "true" : "false";
}

std::string listaDeTextos(const ArregloDinamico<std::string>& elementos) {
    std::string json = "[";
    for (int i = 0; i < elementos.tamano(); i++) {
        if (i > 0) {
            json += ",";
        }
        json += texto(elementos.obtener(i));
    }
    json += "]";
    return json;
}

std::string listaDeAlfabeto(const ArregloDinamico<char>& alfabeto) {
    std::string json = "[";
    for (int i = 0; i < alfabeto.tamano(); i++) {
        if (i > 0) {
            json += ",";
        }
        json += texto(desdeChar(alfabeto.obtener(i)));
    }
    json += "]";
    return json;
}

std::string jsonDeAutomata(const Automata& automata, int id) {
    std::string json = "{";
    json += "\"id\":" + numero(id);
    json += ",\"nombre\":" + texto(automata.obtenerNombre());
    json += ",\"estados\":" + listaDeTextos(automata.obtenerEstados());

    json += ",\"alfabeto\":" + listaDeAlfabeto(automata.obtenerAlfabeto());

    json += ",\"inicial\":";
    if (automata.tieneEstadoInicial()) {
        json += texto(automata.obtenerEstadoInicial());
    } else {
        json += "null";
    }

    json += ",\"finales\":" + listaDeTextos(automata.obtenerEstadosFinales());

    json += ",\"transiciones\":[";
    const ArregloDinamico<Transicion>& transiciones = automata.obtenerTransiciones();
    for (int i = 0; i < transiciones.tamano(); i++) {
        const Transicion& t = transiciones.obtener(i);
        if (i > 0) {
            json += ",";
        }
        json += "{\"origen\":" + texto(t.origen);
        json += ",\"simbolo\":" + texto(desdeChar(t.simbolo));
        json += ",\"destino\":" + texto(t.destino) + "}";
    }
    json += "]}";
    return json;
}

// rehace el recorrido de la construccion de la union para poder mostrarla
// paso a paso: mismos bucles y mismas consultas que hace construirAutomataUnion
std::string jsonDeConstruccion(const Automata& automataA, const Automata& automataB) {
    const ArregloDinamico<std::string>& estadosA = automataA.obtenerEstados();
    const ArregloDinamico<std::string>& estadosB = automataB.obtenerEstados();
    const ArregloDinamico<char>& alfabeto = automataA.obtenerAlfabeto();

    std::string json = "{";
    json += "\"nombreA\":" + texto(automataA.obtenerNombre());
    json += ",\"nombreB\":" + texto(automataB.obtenerNombre());
    json += ",\"estadosA\":" + listaDeTextos(estadosA);
    json += ",\"estadosB\":" + listaDeTextos(estadosB);
    json += ",\"alfabeto\":" + listaDeAlfabeto(alfabeto);
    json += ",\"finalesA\":" + listaDeTextos(automataA.obtenerEstadosFinales());
    json += ",\"finalesB\":" + listaDeTextos(automataB.obtenerEstadosFinales());
    json += ",\"inicialA\":" + texto(automataA.obtenerEstadoInicial());
    json += ",\"inicialB\":" + texto(automataB.obtenerEstadoInicial());
    json += ",\"inicialPar\":" + texto(UnionAutomatas::nombreEstadoPar(
        automataA.obtenerEstadoInicial(), automataB.obtenerEstadoInicial()));

    json += ",\"pasos\":[";
    bool primerPaso = true;
    for (int i = 0; i < estadosA.tamano(); i++) {
        for (int j = 0; j < estadosB.tamano(); j++) {
            const std::string& estadoA = estadosA.obtener(i);
            const std::string& estadoB = estadosB.obtener(j);
            bool finalA = automataA.esEstadoFinal(estadoA);
            bool finalB = automataB.esEstadoFinal(estadoB);

            if (!primerPaso) {
                json += ",";
            }
            primerPaso = false;

            json += "{\"par\":" + texto(UnionAutomatas::nombreEstadoPar(estadoA, estadoB));
            json += ",\"a\":" + texto(estadoA);
            json += ",\"b\":" + texto(estadoB);
            json += ",\"finalA\":" + booleano(finalA);
            json += ",\"finalB\":" + booleano(finalB);
            json += ",\"esFinal\":" + booleano(finalA || finalB);
            json += ",\"transiciones\":[";

            for (int k = 0; k < alfabeto.tamano(); k++) {
                char simbolo = alfabeto.obtener(k);
                std::string destinoA;
                std::string destinoB;
                bool hayA = automataA.obtenerTransicion(estadoA, simbolo, destinoA);
                bool hayB = automataB.obtenerTransicion(estadoB, simbolo, destinoB);

                if (k > 0) {
                    json += ",";
                }
                json += "{\"simbolo\":" + texto(desdeChar(simbolo));
                json += ",\"definida\":" + booleano(hayA && hayB);
                json += ",\"destinoA\":" + texto(destinoA);
                json += ",\"destinoB\":" + texto(destinoB);
                json += ",\"destino\":";
                if (hayA && hayB) {
                    json += texto(UnionAutomatas::nombreEstadoPar(destinoA, destinoB));
                } else {
                    json += "null";
                }
                json += "}";
            }
            json += "]}";
        }
    }
    json += "]}";
    return json;
}

// --- utilidades de entrada ---

bool aEntero(const std::string& valor, int& resultado) {
    if (valor.size() == 0) {
        return false;
    }
    int acumulado = 0;
    for (int i = 0; i < (int)valor.size(); i++) {
        if (valor[i] < '0' || valor[i] > '9') {
            return false;
        }
        acumulado = acumulado * 10 + (valor[i] - '0');
    }
    resultado = acumulado;
    return true;
}

bool indiceValido(int indice) {
    return indice >= 0 && indice < almacen.tamano();
}

// el front manda cada transicion como origen + SEP + simbolo + SEP + destino.
// el separador es un caracter de control que no se puede escribir en el
// formulario, asi que ningun nombre ni simbolo lo puede contener.
const char SEPARADOR = '\x1F';

bool partirTransicion(const std::string& crudo, std::string& origen, char& simbolo, std::string& destino) {
    int primera = -1;
    int segunda = -1;
    for (int i = 0; i < (int)crudo.size(); i++) {
        if (crudo[i] == SEPARADOR) {
            if (primera == -1) {
                primera = i;
            } else if (segunda == -1) {
                segunda = i;
            } else {
                return false;
            }
        }
    }
    if (primera == -1 || segunda == -1) {
        return false;
    }
    if (segunda - primera != 2) {
        return false;
    }
    origen = crudo.substr(0, primera);
    simbolo = crudo[primera + 1];
    destino = crudo.substr(segunda + 1);
    return origen.size() > 0 && destino.size() > 0;
}

// recorre la cadena guardando cada estado por el que pasa
bool simular(const Automata& automata, const std::string& cadena, ArregloDinamico<std::string>& traza) {
    if (!automata.tieneEstadoInicial()) {
        return false;
    }
    std::string actual = automata.obtenerEstadoInicial();
    traza.agregar(actual);
    for (int i = 0; i < (int)cadena.size(); i++) {
        std::string siguiente;
        if (!automata.obtenerTransicion(actual, cadena[i], siguiente)) {
            return false;
        }
        actual = siguiente;
        traza.agregar(actual);
    }
    return automata.esEstadoFinal(actual);
}

std::string jsonDeRecorrido(const Automata& automata, int id, const std::string& rol, const std::string& cadena) {
    ArregloDinamico<std::string> traza;
    bool acepta = simular(automata, cadena, traza);
    int consumidos = traza.tamano() > 0 ? traza.tamano() - 1 : 0;

    std::string json = "{";
    json += "\"id\":" + numero(id);
    json += ",\"rol\":" + texto(rol);
    json += ",\"nombre\":" + texto(automata.obtenerNombre());
    json += ",\"acepta\":" + booleano(acepta);
    json += ",\"traza\":" + listaDeTextos(traza);
    json += ",\"consumidos\":" + numero(consumidos);
    json += ",\"completa\":" + booleano(consumidos == (int)cadena.size());
    json += ",\"simboloTrabado\":";
    if (consumidos < (int)cadena.size()) {
        json += texto(desdeChar(cadena[consumidos]));
    } else {
        json += "null";
    }
    json += "}";
    return json;
}

void responder(httplib::Response& respuesta, const std::string& json) {
    // el charset explicito evita que los nombres con acentos o caracteres no
    // ascii se lean como latin-1 del otro lado
    respuesta.set_content(json, "application/json; charset=utf-8");
}

void error(httplib::Response& respuesta, int estado, const std::string& mensaje) {
    respuesta.status = estado;
    responder(respuesta, "{\"ok\":false,\"error\":" + texto(mensaje) + "}");
}

// --- rutas ---

void listarAutomatas(const httplib::Request&, httplib::Response& respuesta) {
    std::lock_guard<std::mutex> cierre(candado);
    std::string json = "{\"ok\":true,\"automatas\":[";
    for (int i = 0; i < almacen.tamano(); i++) {
        if (i > 0) {
            json += ",";
        }
        json += jsonDeAutomata(almacen.obtener(i), i);
    }
    json += "]}";
    responder(respuesta, json);
}

void obtenerAutomata(const httplib::Request& peticion, httplib::Response& respuesta) {
    std::lock_guard<std::mutex> cierre(candado);
    int indice = 0;
    if (!aEntero(peticion.get_param_value("id"), indice) || !indiceValido(indice)) {
        error(respuesta, 404, "No existe un automata con ese indice.");
        return;
    }
    responder(respuesta, "{\"ok\":true,\"automata\":" + jsonDeAutomata(almacen.obtener(indice), indice) + "}");
}

void crearAutomata(const httplib::Request& peticion, httplib::Response& respuesta) {
    std::lock_guard<std::mutex> cierre(candado);
    Automata nuevo;
    ArregloDinamico<std::string> avisos;

    nuevo.establecerNombre(peticion.get_param_value("nombre"));

    int totalEstados = (int)peticion.get_param_value_count("estado");
    for (int i = 0; i < totalEstados; i++) {
        std::string estado = peticion.get_param_value("estado", i);
        if (estado.size() == 0) {
            continue;
        }
        if (!nuevo.agregarEstado(estado)) {
            avisos.agregar("El estado '" + estado + "' estaba repetido en el formulario y solo se registro una vez.");
        }
    }

    int totalSimbolos = (int)peticion.get_param_value_count("simbolo");
    for (int i = 0; i < totalSimbolos; i++) {
        std::string simbolo = peticion.get_param_value("simbolo", i);
        if (simbolo.size() == 0) {
            continue;
        }
        if (simbolo.size() > 1) {
            avisos.agregar("El simbolo '" + simbolo + "' tiene mas de un caracter, se ignoro.");
            continue;
        }
        if (!nuevo.agregarSimbolo(simbolo[0])) {
            avisos.agregar("El simbolo '" + simbolo + "' se rechazo por repetido o por ser un caracter reservado.");
        }
    }

    std::string inicial = peticion.get_param_value("inicial");
    if (inicial.size() > 0) {
        nuevo.establecerEstadoInicial(inicial);
    }

    int totalFinales = (int)peticion.get_param_value_count("final");
    for (int i = 0; i < totalFinales; i++) {
        std::string estadoFinal = peticion.get_param_value("final", i);
        if (estadoFinal.size() == 0) {
            continue;
        }
        if (!nuevo.agregarEstadoFinal(estadoFinal)) {
            avisos.agregar("'" + estadoFinal + "' no se pudo marcar como estado de aceptacion porque no esta en el conjunto de estados.");
        }
    }

    int totalTransiciones = (int)peticion.get_param_value_count("transicion");
    for (int i = 0; i < totalTransiciones; i++) {
        std::string crudo = peticion.get_param_value("transicion", i);
        if (crudo.size() == 0) {
            continue;
        }
        std::string origen;
        std::string destino;
        char simbolo = '\0';
        if (!partirTransicion(crudo, origen, simbolo, destino)) {
            avisos.agregar("Una transicion llego incompleta y se ignoro.");
            continue;
        }
        nuevo.agregarTransicion(origen, simbolo, destino);
    }

    ResultadoValidacion validacion = ValidadorAutomata::validar(nuevo);

    // si no pasa la validacion no se guarda ni queda disponible para la union
    bool guardar = peticion.get_param_value("guardar") != "0";
    bool guardado = false;
    int id = -1;
    if (validacion.esValido && guardar) {
        almacen.agregar(nuevo);
        id = almacen.tamano() - 1;
        guardado = true;
    }

    std::string json = "{\"ok\":true";
    json += ",\"valido\":" + booleano(validacion.esValido);
    json += ",\"guardado\":" + booleano(guardado);
    json += ",\"id\":" + numero(id);
    json += ",\"errores\":" + listaDeTextos(validacion.errores);
    json += ",\"avisos\":" + listaDeTextos(avisos);
    json += ",\"automata\":" + jsonDeAutomata(nuevo, id);
    json += "}";
    responder(respuesta, json);
}

void validarGuardado(const httplib::Request& peticion, httplib::Response& respuesta) {
    std::lock_guard<std::mutex> cierre(candado);
    int indice = 0;
    if (!aEntero(peticion.get_param_value("id"), indice) || !indiceValido(indice)) {
        error(respuesta, 400, "Indice de automata invalido.");
        return;
    }
    ResultadoValidacion validacion = ValidadorAutomata::validar(almacen.obtener(indice));
    std::string json = "{\"ok\":true";
    json += ",\"valido\":" + booleano(validacion.esValido);
    json += ",\"errores\":" + listaDeTextos(validacion.errores);
    json += "}";
    responder(respuesta, json);
}

void unirAutomatas(const httplib::Request& peticion, httplib::Response& respuesta) {
    std::lock_guard<std::mutex> cierre(candado);
    int indiceA = 0;
    int indiceB = 0;
    if (!aEntero(peticion.get_param_value("idA"), indiceA) || !indiceValido(indiceA) ||
        !aEntero(peticion.get_param_value("idB"), indiceB) || !indiceValido(indiceB)) {
        error(respuesta, 400, "Hay que elegir dos automatas guardados.");
        return;
    }

    ResultadoUnion resultado = UnionAutomatas::unir(almacen.obtener(indiceA), almacen.obtener(indiceB));
    if (!resultado.exitoso) {
        responder(respuesta, "{\"ok\":true,\"exitoso\":false,\"error\":" + texto(resultado.mensajeError) + "}");
        return;
    }

    // el resultado tambien tiene que pasar la validacion antes de guardarse. si
    // los nombres de estado traen comas o parentesis, dos pares distintos pueden
    // terminar con el mismo nombre y el automata sale roto.
    ResultadoValidacion revision = ValidadorAutomata::validar(resultado.automataResultante);
    if (!revision.esValido) {
        std::string json = "{\"ok\":true,\"exitoso\":false";
        json += ",\"error\":" + texto("La union genero un automata invalido, asi que no se guarda. "
                                      "Suele pasar cuando los nombres de estado tienen comas o parentesis: "
                                      "dos pares distintos terminan con el mismo nombre.");
        json += ",\"errores\":" + listaDeTextos(revision.errores);
        json += "}";
        responder(respuesta, json);
        return;
    }

    almacen.agregar(resultado.automataResultante);
    int id = almacen.tamano() - 1;

    std::string json = "{\"ok\":true,\"exitoso\":true";
    json += ",\"idA\":" + numero(indiceA);
    json += ",\"idB\":" + numero(indiceB);
    json += ",\"id\":" + numero(id);
    json += ",\"automata\":" + jsonDeAutomata(almacen.obtener(id), id);
    json += ",\"construccion\":" + jsonDeConstruccion(almacen.obtener(indiceA), almacen.obtener(indiceB));
    json += "}";
    responder(respuesta, json);
}

// devuelve los recorridos como lista: uno solo cuando se prueba un automata
// suelto, o los tres del veredicto cuando se prueba una union
void probarCadena(const httplib::Request& peticion, httplib::Response& respuesta) {
    std::lock_guard<std::mutex> cierre(candado);
    std::string cadena = peticion.get_param_value("cadena");

    if (peticion.get_param_value("modo") == "simple") {
        int id = 0;
        if (!aEntero(peticion.get_param_value("id"), id) || !indiceValido(id)) {
            error(respuesta, 400, "Hay que elegir un automata guardado.");
            return;
        }
        std::string json = "{\"ok\":true,\"modo\":\"simple\"";
        json += ",\"cadena\":" + texto(cadena);
        json += ",\"principal\":0";
        json += ",\"recorridos\":[" + jsonDeRecorrido(almacen.obtener(id), id, "Automata", cadena) + "]";
        json += "}";
        responder(respuesta, json);
        return;
    }

    int idUnion = 0;
    int idA = 0;
    int idB = 0;
    if (!aEntero(peticion.get_param_value("idUnion"), idUnion) || !indiceValido(idUnion) ||
        !aEntero(peticion.get_param_value("idA"), idA) || !indiceValido(idA) ||
        !aEntero(peticion.get_param_value("idB"), idB) || !indiceValido(idB)) {
        error(respuesta, 400, "Hay que elegir el automata union y los dos originales.");
        return;
    }

    std::string json = "{\"ok\":true,\"modo\":\"union\"";
    json += ",\"cadena\":" + texto(cadena);
    json += ",\"principal\":2";
    json += ",\"recorridos\":[";
    json += jsonDeRecorrido(almacen.obtener(idA), idA, "Automata 1", cadena);
    json += "," + jsonDeRecorrido(almacen.obtener(idB), idB, "Automata 2", cadena);
    json += "," + jsonDeRecorrido(almacen.obtener(idUnion), idUnion, "Automata union", cadena);
    json += "]}";
    responder(respuesta, json);
}

} // namespace

void registrarRutas(httplib::Server& servidor) {
    servidor.Get("/api/automatas", listarAutomatas);
    servidor.Get("/api/automata", obtenerAutomata);
    servidor.Post("/api/automatas", crearAutomata);
    servidor.Post("/api/validar", validarGuardado);
    servidor.Post("/api/unir", unirAutomatas);
    servidor.Post("/api/probar-cadena", probarCadena);
}
