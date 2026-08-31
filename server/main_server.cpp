#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "httplib.h"
#include "ApiHandlers.h"

namespace {

const char* HOST = "127.0.0.1";
const int PUERTO = 8080;

// busca la carpeta del front. RUTA_ESTATICOS la define cmake con la ruta del
// repo, asi editar el html o el js no obliga a recompilar.
bool montarEstaticos(httplib::Server& servidor, const std::string& rutaPedida) {
    if (rutaPedida.size() > 0 && servidor.set_mount_point("/", rutaPedida)) {
        std::cout << "Front servido desde: " << rutaPedida << "\n";
        return true;
    }
#ifdef RUTA_ESTATICOS
    if (servidor.set_mount_point("/", RUTA_ESTATICOS)) {
        std::cout << "Front servido desde: " << RUTA_ESTATICOS << "\n";
        return true;
    }
#endif
    const char* candidatas[] = { "server/static", "../server/static", "../../server/static", "static" };
    for (int i = 0; i < 4; i++) {
        if (servidor.set_mount_point("/", candidatas[i])) {
            std::cout << "Front servido desde: " << candidatas[i] << "\n";
            return true;
        }
    }
    return false;
}

void abrirNavegador(const std::string& url) {
#ifdef _WIN32
    std::string comando = "start \"\" \"" + url + "\"";
    std::system(comando.c_str());
#else
    std::string comando = "xdg-open \"" + url + "\" >/dev/null 2>&1 &";
    std::system(comando.c_str());
#endif
}

} // namespace

int main(int argc, char* argv[]) {
    httplib::Server servidor;

    // por defecto httplib pone SO_REUSEADDR, y en windows eso deja que un
    // segundo proceso se ate al mismo puerto sin avisar: quedan dos servidores
    // con almacenes distintos repartiendose las peticiones. con exclusiva el
    // segundo falla y se entera.
    servidor.set_socket_options([](socket_t sock) {
        int activado = 1;
#ifdef _WIN32
        setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                   reinterpret_cast<const char*>(&activado), sizeof(activado));
#else
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
#endif
    });

    std::string rutaPedida = (argc > 1) ? argv[1] : "";
    if (!montarEstaticos(servidor, rutaPedida)) {
        std::cout << "No se encontro la carpeta server/static.\n";
        std::cout << "Podes pasarla como argumento: dfa_server.exe <ruta>\n";
        return 1;
    }

    registrarRutas(servidor);

    std::string url = "http://localhost:" + std::string("8080");
    std::cout << "\n=== Servidor de Automatas Finitos Deterministas ===\n";
    std::cout << "Escuchando en " << url << "\n";
    std::cout << "Ctrl+C para detener.\n\n";

    std::thread lanzador([&servidor, url] {
        servidor.wait_until_ready();
        abrirNavegador(url);
    });
    lanzador.detach();

    if (!servidor.listen(HOST, PUERTO)) {
        std::cout << "No se pudo abrir el puerto " << PUERTO << ", puede que ya este ocupado.\n";
        return 1;
    }
    return 0;
}
