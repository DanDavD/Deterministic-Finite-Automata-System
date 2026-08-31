#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include "httplib.h"

// cuelga las rutas /api/... sobre el servidor ya creado
void registrarRutas(httplib::Server& servidor);

#endif // API_HANDLERS_H
