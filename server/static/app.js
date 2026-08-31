/* Front del sistema de automatas.
   Igual que en el codigo c++, aca no se usan mapas, conjuntos ni los metodos
   de alto nivel de los arreglos: solo arreglos planos recorridos con for y
   comparaciones una por una. El diagrama se dibuja generando el svg a mano. */

var SVG_NS = 'http://www.w3.org/2000/svg';

// separa los campos de una transicion al mandarla. es un caracter de control
// que no se puede tipear, asi que ningun nombre ni simbolo lo puede contener.
var SEPARADOR = '\u001F';

/* ============ ayudantes basicos ============ */

function porId(id) {
  return document.getElementById(id);
}

function crear(etiqueta, clase) {
  var elemento = document.createElement(etiqueta);
  if (clase) {
    elemento.className = clase;
  }
  return elemento;
}

function svgCrear(etiqueta) {
  return document.createElementNS(SVG_NS, etiqueta);
}

function limpiar(nodo) {
  while (nodo.firstChild) {
    nodo.removeChild(nodo.firstChild);
  }
}

function texto(etiqueta, clase, contenido) {
  var elemento = crear(etiqueta, clase);
  elemento.textContent = contenido;
  return elemento;
}

/* busquedas manuales, sin indexOf ni includes */

function contieneTexto(arreglo, valor) {
  for (var i = 0; i < arreglo.length; i++) {
    if (arreglo[i] === valor) {
      return true;
    }
  }
  return false;
}

function posicionDeTexto(arreglo, valor) {
  for (var i = 0; i < arreglo.length; i++) {
    if (arreglo[i] === valor) {
      return i;
    }
  }
  return -1;
}

function indiceDeNodo(nodos, nombre) {
  for (var i = 0; i < nodos.length; i++) {
    if (nodos[i].nombre === nombre) {
      return i;
    }
  }
  return -1;
}

function buscarDestino(automata, origen, simbolo) {
  var transiciones = automata.transiciones;
  for (var i = 0; i < transiciones.length; i++) {
    if (transiciones[i].origen === origen && transiciones[i].simbolo === simbolo) {
      return transiciones[i].destino;
    }
  }
  return null;
}

function quitarEnIndice(arreglo, indice) {
  var copia = [];
  for (var i = 0; i < arreglo.length; i++) {
    if (i !== indice) {
      copia.push(arreglo[i]);
    }
  }
  return copia;
}

/* ============ red ============ */

function agregarPar(cuerpo, clave, valor) {
  var pieza = encodeURIComponent(clave) + '=' + encodeURIComponent(valor);
  return cuerpo.length > 0 ? cuerpo + '&' + pieza : pieza;
}

function pedirJson(ruta) {
  return fetch(ruta).then(function (respuesta) {
    return respuesta.json();
  });
}

function enviarForm(ruta, cuerpo) {
  return fetch(ruta, {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: cuerpo
  }).then(function (respuesta) {
    return respuesta.json();
  });
}

/* ============ vista de grafo ============ */

function radioNodo(nombre) {
  var r = 18 + nombre.length * 3.2;
  if (r < 24) { r = 24; }
  if (r > 48) { r = 48; }
  return r;
}

function tamanoTexto(nombre) {
  if (nombre.length > 12) { return 9; }
  if (nombre.length > 8) { return 11; }
  return 13;
}

// agrupa las transiciones que van del mismo origen al mismo destino
function agruparAristas(transiciones) {
  var aristas = [];
  for (var i = 0; i < transiciones.length; i++) {
    var t = transiciones[i];
    var encontrada = -1;
    for (var j = 0; j < aristas.length; j++) {
      if (aristas[j].origen === t.origen && aristas[j].destino === t.destino) {
        encontrada = j;
        break;
      }
    }
    if (encontrada === -1) {
      aristas.push({ origen: t.origen, destino: t.destino, etiqueta: t.simbolo, simbolos: [t.simbolo] });
    } else {
      var repetido = false;
      for (var k = 0; k < aristas[encontrada].simbolos.length; k++) {
        if (aristas[encontrada].simbolos[k] === t.simbolo) {
          repetido = true;
          break;
        }
      }
      if (!repetido) {
        aristas[encontrada].simbolos.push(t.simbolo);
        aristas[encontrada].etiqueta = aristas[encontrada].etiqueta + ', ' + t.simbolo;
      }
    }
  }
  return aristas;
}

function haciaPunto(nodo, px, py) {
  var dx = px - nodo.x;
  var dy = py - nodo.y;
  var d = Math.sqrt(dx * dx + dy * dy);
  if (d < 0.001) { d = 0.001; }
  return { x: nodo.x + (dx / d) * nodo.r, y: nodo.y + (dy / d) * nodo.r };
}

function puntosPunta(punta, dx, dy, tamano) {
  var d = Math.sqrt(dx * dx + dy * dy);
  if (d < 0.001) { d = 0.001; }
  var ux = dx / d;
  var uy = dy / d;
  var px = -uy;
  var py = ux;
  var baseX = punta.x - ux * tamano;
  var baseY = punta.y - uy * tamano;
  var ala = tamano * 0.42;
  return punta.x + ',' + punta.y + ' ' +
    (baseX + px * ala) + ',' + (baseY + py * ala) + ' ' +
    (baseX - px * ala) + ',' + (baseY - py * ala);
}

function crearVistaGrafo(contenedor) {
  limpiar(contenedor);

  var svg = svgCrear('svg');
  svg.setAttribute('preserveAspectRatio', 'xMidYMid meet');
  contenedor.appendChild(svg);

  var herramientas = crear('div', 'herramientas-lienzo');
  var botonMas = texto('button', null, '+');
  var botonMenos = texto('button', null, '−');
  var botonCentro = texto('button', null, '⌗');
  botonMas.title = 'Acercar';
  botonMenos.title = 'Alejar';
  botonCentro.title = 'Centrar';
  herramientas.appendChild(botonMas);
  herramientas.appendChild(botonMenos);
  herramientas.appendChild(botonCentro);
  contenedor.appendChild(herramientas);

  var pista = texto('div', 'pista-lienzo', 'arrastra los nodos o el fondo, rueda para acercar');
  contenedor.appendChild(pista);

  var automata = null;
  var nodos = [];
  var aristas = [];
  var vista = { x: -200, y: -150, w: 400, h: 300 };
  var resaltado = { estado: null, origen: null, destino: null, simbolo: null, clase: 'nodo-activo' };
  var arrastreNodo = null;
  var arrastreVista = null;
  var tocado = false;

  function aplicarVista() {
    svg.setAttribute('viewBox', vista.x + ' ' + vista.y + ' ' + vista.w + ' ' + vista.h);
  }

  function relacionCaja() {
    var caja = svg.getBoundingClientRect();
    if (caja.width < 1 || caja.height < 1) {
      return 0.62;
    }
    return caja.height / caja.width;
  }

  function encuadrar(minX, minY, maxX, maxY) {
    var relacion = relacionCaja();
    var anchoContenido = maxX - minX;
    var altoContenido = maxY - minY;
    if (anchoContenido < 80) { anchoContenido = 80; }
    if (altoContenido < 80) { altoContenido = 80; }
    var ancho = anchoContenido;
    if (altoContenido / ancho > relacion) {
      ancho = altoContenido / relacion;
    }
    var alto = ancho * relacion;
    var cx = (minX + maxX) / 2;
    var cy = (minY + maxY) / 2;
    vista = { x: cx - ancho / 2, y: cy - alto / 2, w: ancho, h: alto };
    aplicarVista();
  }

  function centrar() {
    if (nodos.length === 0) {
      encuadrar(-160, -120, 160, 120);
      return;
    }
    var minX = nodos[0].x;
    var maxX = nodos[0].x;
    var minY = nodos[0].y;
    var maxY = nodos[0].y;
    for (var i = 0; i < nodos.length; i++) {
      var n = nodos[i];
      if (n.x - n.r < minX) { minX = n.x - n.r; }
      if (n.x + n.r > maxX) { maxX = n.x + n.r; }
      if (n.y - n.r < minY) { minY = n.y - n.r; }
      if (n.y + n.r > maxY) { maxY = n.y + n.r; }
    }
    var margen = 92;
    encuadrar(minX - margen, minY - margen, maxX + margen, maxY + margen);
    tocado = false;
  }

  function centroide() {
    if (nodos.length === 0) {
      return { x: 0, y: 0 };
    }
    var sx = 0;
    var sy = 0;
    for (var i = 0; i < nodos.length; i++) {
      sx += nodos[i].x;
      sy += nodos[i].y;
    }
    return { x: sx / nodos.length, y: sy / nodos.length };
  }

  function direccionExterior(nodo, centro) {
    var dx = nodo.x - centro.x;
    var dy = nodo.y - centro.y;
    var d = Math.sqrt(dx * dx + dy * dy);
    if (d < 1) {
      return { x: Math.cos(nodo.ang), y: Math.sin(nodo.ang) };
    }
    return { x: dx / d, y: dy / d };
  }

  function calcularLayout() {
    nodos = [];
    if (automata === null) {
      return;
    }
    var estados = automata.estados;
    var total = estados.length;
    var radio = 150;
    if (total > 4) {
      radio = 108 + total * 26;
    }
    for (var i = 0; i < total; i++) {
      var nombre = estados[i];
      var ang = -Math.PI / 2 + (2 * Math.PI * i) / total;
      var x = total === 1 ? 0 : radio * Math.cos(ang);
      var y = total === 1 ? 0 : radio * Math.sin(ang);
      nodos.push({
        nombre: nombre,
        x: x,
        y: y,
        ang: ang,
        r: radioNodo(nombre),
        esFinal: contieneTexto(automata.finales, nombre),
        esInicial: automata.inicial === nombre
      });
    }
  }

  function aristaActiva(arista) {
    if (resaltado.origen === null || resaltado.destino === null) {
      return false;
    }
    if (arista.origen !== resaltado.origen || arista.destino !== resaltado.destino) {
      return false;
    }
    for (var i = 0; i < arista.simbolos.length; i++) {
      if (arista.simbolos[i] === resaltado.simbolo) {
        return true;
      }
    }
    return false;
  }

  function dibujarEtiqueta(capa, x, y, contenido, activa) {
    var ancho = contenido.length * 7.4 + 12;
    var fondo = svgCrear('rect');
    fondo.setAttribute('x', x - ancho / 2);
    fondo.setAttribute('y', y - 9.5);
    fondo.setAttribute('width', ancho);
    fondo.setAttribute('height', 19);
    fondo.setAttribute('rx', 6);
    fondo.setAttribute('class', 'arista-fondo');
    capa.appendChild(fondo);

    var rotulo = svgCrear('text');
    rotulo.setAttribute('x', x);
    rotulo.setAttribute('y', y);
    rotulo.setAttribute('class', activa ? 'arista-etiqueta arista-etiqueta-activa' : 'arista-etiqueta');
    rotulo.textContent = contenido;
    capa.appendChild(rotulo);
  }

  function dibujarPunta(capa, punta, dx, dy, activa) {
    var poligono = svgCrear('polygon');
    poligono.setAttribute('points', puntosPunta(punta, dx, dy, 11));
    poligono.setAttribute('class', activa ? 'arista-punta arista-activa-punta' : 'arista-punta');
    capa.appendChild(poligono);
  }

  function dibujarAutolazo(capa, nodo, arista, centro, activa) {
    var u = direccionExterior(nodo, centro);
    var px = -u.y;
    var py = u.x;
    var r = nodo.r;

    var ini = { x: nodo.x + u.x * r * 0.42 + px * r * 0.9, y: nodo.y + u.y * r * 0.42 + py * r * 0.9 };
    var fin = { x: nodo.x + u.x * r * 0.42 - px * r * 0.9, y: nodo.y + u.y * r * 0.42 - py * r * 0.9 };
    var c1 = { x: nodo.x + u.x * (r + 58) + px * (r + 24), y: nodo.y + u.y * (r + 58) + py * (r + 24) };
    var c2 = { x: nodo.x + u.x * (r + 58) - px * (r + 24), y: nodo.y + u.y * (r + 58) - py * (r + 24) };

    var camino = svgCrear('path');
    camino.setAttribute('d', 'M ' + ini.x + ' ' + ini.y +
      ' C ' + c1.x + ' ' + c1.y + ' ' + c2.x + ' ' + c2.y + ' ' + fin.x + ' ' + fin.y);
    camino.setAttribute('class', activa ? 'arista arista-activa' : 'arista');
    capa.appendChild(camino);

    dibujarPunta(capa, fin, fin.x - c2.x, fin.y - c2.y, activa);
    dibujarEtiqueta(capa, nodo.x + u.x * (r + 66), nodo.y + u.y * (r + 66), arista.etiqueta, activa);
  }

  function dibujarArista(capa, arista, centro) {
    var ia = indiceDeNodo(nodos, arista.origen);
    var ib = indiceDeNodo(nodos, arista.destino);
    if (ia === -1 || ib === -1) {
      return;
    }
    var activa = aristaActiva(arista);
    var a = nodos[ia];

    if (arista.origen === arista.destino) {
      dibujarAutolazo(capa, a, arista, centro, activa);
      return;
    }

    var b = nodos[ib];
    var dx = b.x - a.x;
    var dy = b.y - a.y;
    var d = Math.sqrt(dx * dx + dy * dy);
    if (d < 0.001) { d = 0.001; }

    // el control se corre perpendicular al segmento; como la perpendicular
    // cambia de signo al invertir el sentido, la ida y la vuelta no se pisan
    var nx = -dy / d;
    var ny = dx / d;
    var curva = 34;
    var cx = (a.x + b.x) / 2 + nx * curva;
    var cy = (a.y + b.y) / 2 + ny * curva;

    var ini = haciaPunto(a, cx, cy);
    var fin = haciaPunto(b, cx, cy);

    var camino = svgCrear('path');
    camino.setAttribute('d', 'M ' + ini.x + ' ' + ini.y + ' Q ' + cx + ' ' + cy + ' ' + fin.x + ' ' + fin.y);
    camino.setAttribute('class', activa ? 'arista arista-activa' : 'arista');
    capa.appendChild(camino);

    dibujarPunta(capa, fin, fin.x - cx, fin.y - cy, activa);

    var ex = 0.25 * ini.x + 0.5 * cx + 0.25 * fin.x;
    var ey = 0.25 * ini.y + 0.5 * cy + 0.25 * fin.y;
    dibujarEtiqueta(capa, ex + nx * 12, ey + ny * 12, arista.etiqueta, activa);
  }

  function dibujarFlechaInicial(capa, nodo, centro) {
    var u = direccionExterior(nodo, centro);
    var desde = { x: nodo.x + u.x * (nodo.r + 46), y: nodo.y + u.y * (nodo.r + 46) };
    var hasta = { x: nodo.x + u.x * (nodo.r + 3), y: nodo.y + u.y * (nodo.r + 3) };

    var linea = svgCrear('line');
    linea.setAttribute('x1', desde.x);
    linea.setAttribute('y1', desde.y);
    linea.setAttribute('x2', hasta.x);
    linea.setAttribute('y2', hasta.y);
    linea.setAttribute('class', 'flecha-inicial');
    capa.appendChild(linea);

    var poligono = svgCrear('polygon');
    poligono.setAttribute('points', puntosPunta(hasta, -u.x, -u.y, 12));
    poligono.setAttribute('class', 'flecha-inicial-punta');
    capa.appendChild(poligono);
  }

  function dibujarNodo(capa, nodo, indice, centro) {
    var grupo = svgCrear('g');
    var clases = 'nodo-grupo';
    if (nodo.esFinal) {
      clases += ' nodo-final';
    }
    if (resaltado.estado === nodo.nombre) {
      clases += ' ' + resaltado.clase;
    }
    grupo.setAttribute('class', clases);

    if (nodo.esInicial) {
      dibujarFlechaInicial(grupo, nodo, centro);
    }

    var circulo = svgCrear('circle');
    circulo.setAttribute('cx', nodo.x);
    circulo.setAttribute('cy', nodo.y);
    circulo.setAttribute('r', nodo.r);
    circulo.setAttribute('class', 'nodo-circulo');
    grupo.appendChild(circulo);

    if (nodo.esFinal) {
      var anillo = svgCrear('circle');
      anillo.setAttribute('cx', nodo.x);
      anillo.setAttribute('cy', nodo.y);
      anillo.setAttribute('r', nodo.r - 5);
      anillo.setAttribute('class', 'nodo-anillo');
      grupo.appendChild(anillo);
    }

    var rotulo = svgCrear('text');
    rotulo.setAttribute('x', nodo.x);
    rotulo.setAttribute('y', nodo.y);
    rotulo.setAttribute('class', 'nodo-texto');
    rotulo.setAttribute('font-size', tamanoTexto(nodo.nombre));
    rotulo.textContent = nodo.nombre;
    grupo.appendChild(rotulo);

    grupo.addEventListener('pointerdown', function (evento) {
      evento.stopPropagation();
      var p = puntoSvg(evento);
      arrastreNodo = { indice: indice, dx: nodos[indice].x - p.x, dy: nodos[indice].y - p.y };
      tocado = true;
      svg.setPointerCapture(evento.pointerId);
    });

    capa.appendChild(grupo);
  }

  function redibujar() {
    limpiar(svg);
    if (automata === null) {
      return;
    }
    var centro = centroide();

    var capaAristas = svgCrear('g');
    var capaNodos = svgCrear('g');
    svg.appendChild(capaAristas);
    svg.appendChild(capaNodos);

    for (var i = 0; i < aristas.length; i++) {
      dibujarArista(capaAristas, aristas[i], centro);
    }
    for (var j = 0; j < nodos.length; j++) {
      dibujarNodo(capaNodos, nodos[j], j, centro);
    }
  }

  function puntoSvg(evento) {
    var caja = svg.getBoundingClientRect();
    if (caja.width < 1 || caja.height < 1) {
      return { x: 0, y: 0 };
    }
    var fx = (evento.clientX - caja.left) / caja.width;
    var fy = (evento.clientY - caja.top) / caja.height;
    return { x: vista.x + fx * vista.w, y: vista.y + fy * vista.h };
  }

  svg.addEventListener('pointerdown', function (evento) {
    var p = puntoSvg(evento);
    arrastreVista = { px: p.x, py: p.y };
    svg.classList.add('arrastrando');
    svg.setPointerCapture(evento.pointerId);
  });

  svg.addEventListener('pointermove', function (evento) {
    if (arrastreNodo !== null) {
      var p = puntoSvg(evento);
      nodos[arrastreNodo.indice].x = p.x + arrastreNodo.dx;
      nodos[arrastreNodo.indice].y = p.y + arrastreNodo.dy;
      redibujar();
      return;
    }
    if (arrastreVista !== null) {
      var q = puntoSvg(evento);
      vista.x -= q.x - arrastreVista.px;
      vista.y -= q.y - arrastreVista.py;
      aplicarVista();
      tocado = true;
    }
  });

  function soltar(evento) {
    arrastreNodo = null;
    arrastreVista = null;
    svg.classList.remove('arrastrando');
    if (evento && evento.pointerId !== undefined && svg.hasPointerCapture(evento.pointerId)) {
      svg.releasePointerCapture(evento.pointerId);
    }
  }

  svg.addEventListener('pointerup', soltar);
  svg.addEventListener('pointercancel', soltar);

  svg.addEventListener('wheel', function (evento) {
    evento.preventDefault();
    var factor = evento.deltaY > 0 ? 1.14 : 1 / 1.14;
    var p = puntoSvg(evento);
    vista.x = p.x - (p.x - vista.x) * factor;
    vista.y = p.y - (p.y - vista.y) * factor;
    vista.w *= factor;
    vista.h *= factor;
    aplicarVista();
    tocado = true;
  }, { passive: false });

  function zoom(factor) {
    var cx = vista.x + vista.w / 2;
    var cy = vista.y + vista.h / 2;
    vista.w *= factor;
    vista.h *= factor;
    vista.x = cx - vista.w / 2;
    vista.y = cy - vista.h / 2;
    aplicarVista();
    tocado = true;
  }

  botonMas.addEventListener('click', function () { zoom(1 / 1.25); });
  botonMenos.addEventListener('click', function () { zoom(1.25); });
  botonCentro.addEventListener('click', centrar);

  if (typeof ResizeObserver !== 'undefined') {
    var observador = new ResizeObserver(function () {
      if (!tocado) {
        centrar();
      }
    });
    observador.observe(contenedor);
  }

  return {
    cargar: function (nuevo) {
      automata = nuevo;
      aristas = agruparAristas(nuevo.transiciones);
      resaltado = { estado: null, origen: null, destino: null, simbolo: null, clase: 'nodo-activo' };
      calcularLayout();
      redibujar();
      centrar();
    },
    resaltar: function (estado, origen, simbolo, destino, clase) {
      resaltado = {
        estado: estado,
        origen: origen === undefined ? null : origen,
        simbolo: simbolo === undefined ? null : simbolo,
        destino: destino === undefined ? null : destino,
        clase: clase || 'nodo-activo'
      };
      redibujar();
    },
    refrescar: function () {
      if (!tocado) {
        centrar();
      }
    }
  };
}

/* ============ estado de la aplicacion ============ */

var automatas = [];
var seleccionado = -1;

var estadosForm = ['q0', 'q1'];
var simbolosForm = ['a', 'b'];
var finalesForm = [];
var inicialForm = '';
var matriz = [];
var transicionesLibres = [];
var modoTransiciones = 'matriz';
var contadorEjemplos = 0;

var grafoDetalle = null;
var grafoUnion = null;
var grafosCorrida = [];
var modoPrueba = 'simple';

var corrida = null;
var pasoActual = 0;
var temporizador = null;

/* ============ piezas visuales compartidas ============ */

function fichas(valores, clase, vacio) {
  var caja = crear('div', 'fichas');
  if (valores.length === 0) {
    caja.appendChild(texto('span', 'ficha ficha-vacia', vacio));
    return caja;
  }
  for (var i = 0; i < valores.length; i++) {
    caja.appendChild(texto('span', clase ? 'ficha ' + clase : 'ficha', valores[i]));
  }
  return caja;
}

function itemResumen(etiqueta, contenido) {
  var item = crear('div', 'resumen-item');
  item.appendChild(texto('div', 'resumen-etiqueta', etiqueta));
  item.appendChild(contenido);
  return item;
}

function construirResumen(automata) {
  var caja = crear('div', 'resumen');
  caja.appendChild(itemResumen('Estados (' + automata.estados.length + ')',
    fichas(automata.estados, null, 'ninguno')));
  caja.appendChild(itemResumen('Alfabeto (' + automata.alfabeto.length + ')',
    fichas(automata.alfabeto, null, 'ninguno')));
  caja.appendChild(itemResumen('Estado inicial',
    fichas(automata.inicial === null ? [] : [automata.inicial], 'ficha-inicial', 'sin definir')));
  caja.appendChild(itemResumen('Estados de aceptacion (' + automata.finales.length + ')',
    fichas(automata.finales, 'ficha-final', 'ninguno')));
  return caja;
}

function construirTabla(automata) {
  var tabla = crear('table');

  var cabecera = crear('thead');
  var filaCabecera = crear('tr');
  filaCabecera.appendChild(texto('th', null, 'Estado'));
  for (var i = 0; i < automata.alfabeto.length; i++) {
    filaCabecera.appendChild(texto('th', null, automata.alfabeto[i]));
  }
  cabecera.appendChild(filaCabecera);
  tabla.appendChild(cabecera);

  var cuerpo = crear('tbody');
  for (var j = 0; j < automata.estados.length; j++) {
    var estado = automata.estados[j];
    var fila = crear('tr');

    var celdaEstado = crear('td');
    if (automata.inicial === estado) {
      celdaEstado.appendChild(texto('span', 'marcador-inicial', '→'));
    }
    if (contieneTexto(automata.finales, estado)) {
      celdaEstado.appendChild(texto('span', 'marcador-final', '✱'));
    }
    celdaEstado.appendChild(document.createTextNode(estado));
    fila.appendChild(celdaEstado);

    for (var k = 0; k < automata.alfabeto.length; k++) {
      var destino = buscarDestino(automata, estado, automata.alfabeto[k]);
      if (destino === null) {
        fila.appendChild(texto('td', 'faltante', '—'));
      } else {
        fila.appendChild(texto('td', null, destino));
      }
    }
    cuerpo.appendChild(fila);
  }
  tabla.appendChild(cuerpo);
  return tabla;
}

function bloqueReporte(clase, titulo, lineas) {
  var caja = crear('div', 'reporte ' + clase);
  caja.appendChild(texto('h4', null, titulo));
  if (lineas && lineas.length > 0) {
    var lista = crear('ul');
    for (var i = 0; i < lineas.length; i++) {
      lista.appendChild(texto('li', null, lineas[i]));
    }
    caja.appendChild(lista);
  }
  return caja;
}

// se llama cuando el fetch ni siquiera llega al servidor
function avisarSinConexion(caja) {
  limpiar(caja);
  caja.appendChild(bloqueReporte('reporte-mal',
    'No hay conexion con el servidor. Revisa que dfa_server.exe siga corriendo y volve a intentar.', null));
}

/* ============ lista lateral ============ */

function refrescarLista() {
  return pedirJson('/api/automatas').then(function (datos) {
    automatas = datos.automatas;
    dibujarLista();
    llenarSelectores();
  }).catch(function () {
    var lista = porId('listaAutomatas');
    limpiar(lista);
    lista.appendChild(texto('p', 'nota',
      'Sin conexion con el servidor. Revisa que dfa_server.exe siga corriendo.'));
  });
}

function dibujarLista() {
  var lista = porId('listaAutomatas');
  limpiar(lista);
  porId('contadorAutomatas').textContent = automatas.length;

  if (automatas.length === 0) {
    lista.appendChild(texto('p', 'nota', 'Todavia no hay ninguno.'));
    return;
  }

  for (var i = 0; i < automatas.length; i++) {
    var automata = automatas[i];
    var tarjeta = crear('button', 'tarjeta-automata' + (automata.id === seleccionado ? ' activa' : ''));
    tarjeta.appendChild(texto('div', 'tarjeta-nombre', automata.nombre));
    tarjeta.appendChild(texto('div', 'tarjeta-meta',
      automata.estados.length + ' estados · ' + automata.alfabeto.length + ' simbolos · ' +
      automata.finales.length + ' finales'));
    (function (id) {
      tarjeta.addEventListener('click', function () {
        seleccionar(id);
      });
    })(automata.id);
    lista.appendChild(tarjeta);
  }
}

function buscarAutomata(id) {
  for (var i = 0; i < automatas.length; i++) {
    if (automatas[i].id === id) {
      return automatas[i];
    }
  }
  return null;
}

function seleccionar(id) {
  seleccionado = id;
  dibujarLista();
  porId('probarSolo').value = id;
  mostrarPestana('detalle');
  mostrarDetalle();
}

/* ============ detalle ============ */

function mostrarDetalle() {
  var automata = buscarAutomata(seleccionado);
  if (automata === null) {
    porId('detalleVacio').classList.remove('oculto');
    porId('detalleContenido').classList.add('oculto');
    return;
  }
  porId('detalleVacio').classList.add('oculto');
  porId('detalleContenido').classList.remove('oculto');
  porId('detalleNombre').textContent = automata.nombre;

  if (grafoDetalle === null) {
    grafoDetalle = crearVistaGrafo(porId('lienzoDetalle'));
  }
  grafoDetalle.cargar(automata);

  var resumen = porId('resumenDetalle');
  limpiar(resumen);
  resumen.appendChild(construirResumen(automata));

  var tabla = porId('tablaDetalle');
  limpiar(tabla);
  tabla.appendChild(construirTabla(automata));
}

/* ============ formulario de creacion ============ */

function estadosValidos() {
  var salida = [];
  for (var i = 0; i < estadosForm.length; i++) {
    var valor = estadosForm[i].trim();
    if (valor.length > 0 && !contieneTexto(salida, valor)) {
      salida.push(valor);
    }
  }
  return salida;
}

function simbolosValidos() {
  var salida = [];
  for (var i = 0; i < simbolosForm.length; i++) {
    var valor = simbolosForm[i].trim();
    if (valor.length > 0 && !contieneTexto(salida, valor)) {
      salida.push(valor);
    }
  }
  return salida;
}

function destinoMatriz(origen, simbolo) {
  for (var i = 0; i < matriz.length; i++) {
    if (matriz[i].origen === origen && matriz[i].simbolo === simbolo) {
      return matriz[i].destino;
    }
  }
  return '';
}

function fijarMatriz(origen, simbolo, destino) {
  for (var i = 0; i < matriz.length; i++) {
    if (matriz[i].origen === origen && matriz[i].simbolo === simbolo) {
      matriz[i].destino = destino;
      return;
    }
  }
  matriz.push({ origen: origen, simbolo: simbolo, destino: destino });
}

function filaTexto(valor, indice, marcador, alQuitar, alCambiar, estrecho) {
  var fila = crear('div', 'fila');
  fila.appendChild(texto('span', 'indice', (indice + 1) + '.'));

  var entrada = crear('input', estrecho ? 'estrecho' : null);
  entrada.type = 'text';
  entrada.value = valor;
  entrada.placeholder = marcador;
  entrada.autocomplete = 'off';
  if (estrecho) {
    entrada.maxLength = 1;
  }
  entrada.addEventListener('input', function () {
    alCambiar(entrada.value);
  });
  fila.appendChild(entrada);

  var quitar = texto('button', 'boton-quitar', '×');
  quitar.type = 'button';
  quitar.title = 'Quitar';
  quitar.addEventListener('click', alQuitar);
  fila.appendChild(quitar);

  return fila;
}

function dibujarEstados() {
  var caja = porId('filasEstados');
  limpiar(caja);
  for (var i = 0; i < estadosForm.length; i++) {
    (function (indice) {
      caja.appendChild(filaTexto(estadosForm[indice], indice, 'q' + indice,
        function () {
          estadosForm = quitarEnIndice(estadosForm, indice);
          if (estadosForm.length === 0) { estadosForm = ['']; }
          dibujarEstados();
          sincronizarDependientes();
        },
        function (valor) {
          estadosForm[indice] = valor;
          sincronizarDependientes();
        }, false));
    })(i);
  }
}

function dibujarSimbolos() {
  var caja = porId('filasSimbolos');
  limpiar(caja);
  for (var i = 0; i < simbolosForm.length; i++) {
    (function (indice) {
      caja.appendChild(filaTexto(simbolosForm[indice], indice, 'a',
        function () {
          simbolosForm = quitarEnIndice(simbolosForm, indice);
          if (simbolosForm.length === 0) { simbolosForm = ['']; }
          dibujarSimbolos();
          sincronizarDependientes();
        },
        function (valor) {
          simbolosForm[indice] = valor;
          sincronizarDependientes();
        }, true));
    })(i);
  }
}

function dibujarInicial() {
  var seleccion = porId('campoInicial');
  limpiar(seleccion);
  var estados = estadosValidos();

  var vacia = crear('option');
  vacia.value = '';
  vacia.textContent = '-- sin definir --';
  seleccion.appendChild(vacia);

  for (var i = 0; i < estados.length; i++) {
    var opcion = crear('option');
    opcion.value = estados[i];
    opcion.textContent = estados[i];
    seleccion.appendChild(opcion);
  }
  seleccion.value = contieneTexto(estados, inicialForm) ? inicialForm : '';
}

function dibujarFinales() {
  var caja = porId('cajaFinales');
  limpiar(caja);
  var estados = estadosValidos();

  // los marcados que ya no existen se descartan
  var vigentes = [];
  for (var i = 0; i < finalesForm.length; i++) {
    if (contieneTexto(estados, finalesForm[i])) {
      vigentes.push(finalesForm[i]);
    }
  }
  finalesForm = vigentes;

  if (estados.length === 0) {
    caja.appendChild(texto('p', 'nota', 'Primero defini los estados.'));
    return;
  }

  for (var j = 0; j < estados.length; j++) {
    (function (nombre) {
      var marcado = contieneTexto(finalesForm, nombre);
      var item = crear('label', 'marca-item' + (marcado ? ' marcada' : ''));
      var casilla = crear('input');
      casilla.type = 'checkbox';
      casilla.checked = marcado;
      casilla.addEventListener('change', function () {
        var posicion = posicionDeTexto(finalesForm, nombre);
        if (casilla.checked && posicion === -1) {
          finalesForm.push(nombre);
        } else if (!casilla.checked && posicion !== -1) {
          finalesForm = quitarEnIndice(finalesForm, posicion);
        }
        dibujarFinales();
      });
      item.appendChild(casilla);
      item.appendChild(document.createTextNode(nombre));
      caja.appendChild(item);
    })(estados[j]);
  }
}

function dibujarMatriz() {
  var caja = porId('matrizTransiciones');
  limpiar(caja);
  var estados = estadosValidos();
  var simbolos = simbolosValidos();

  if (estados.length === 0 || simbolos.length === 0) {
    caja.appendChild(texto('p', 'nota', 'Defini estados y alfabeto para armar la tabla.'));
    return;
  }

  var tabla = crear('table');
  var cabecera = crear('thead');
  var filaCabecera = crear('tr');
  filaCabecera.appendChild(texto('th', null, 'δ'));
  for (var i = 0; i < simbolos.length; i++) {
    filaCabecera.appendChild(texto('th', null, simbolos[i]));
  }
  cabecera.appendChild(filaCabecera);
  tabla.appendChild(cabecera);

  var cuerpo = crear('tbody');
  for (var j = 0; j < estados.length; j++) {
    var fila = crear('tr');
    fila.appendChild(texto('td', null, estados[j]));
    for (var k = 0; k < simbolos.length; k++) {
      var celda = crear('td');
      celda.appendChild(celdaDestino(estados[j], simbolos[k], estados));
      fila.appendChild(celda);
    }
    cuerpo.appendChild(fila);
  }
  tabla.appendChild(cuerpo);
  caja.appendChild(tabla);
}

function celdaDestino(origen, simbolo, estados) {
  var seleccion = crear('select', 'mono');
  var vacia = crear('option');
  vacia.value = '';
  vacia.textContent = '—';
  seleccion.appendChild(vacia);

  for (var i = 0; i < estados.length; i++) {
    var opcion = crear('option');
    opcion.value = estados[i];
    opcion.textContent = estados[i];
    seleccion.appendChild(opcion);
  }

  seleccion.value = destinoMatriz(origen, simbolo);
  seleccion.addEventListener('change', function () {
    fijarMatriz(origen, simbolo, seleccion.value);
  });
  return seleccion;
}

function dibujarTransicionesLibres() {
  var caja = porId('filasTransiciones');
  limpiar(caja);

  for (var i = 0; i < transicionesLibres.length; i++) {
    (function (indice) {
      var t = transicionesLibres[indice];
      var fila = crear('div', 'fila');
      fila.appendChild(texto('span', 'indice', (indice + 1) + '.'));

      var origen = crear('input');
      origen.type = 'text';
      origen.value = t.origen;
      origen.placeholder = 'origen';
      origen.autocomplete = 'off';
      origen.addEventListener('input', function () { t.origen = origen.value; });
      fila.appendChild(origen);

      var simbolo = crear('input', 'estrecho');
      simbolo.type = 'text';
      simbolo.value = t.simbolo;
      simbolo.placeholder = 'a';
      simbolo.maxLength = 1;
      simbolo.autocomplete = 'off';
      simbolo.addEventListener('input', function () { t.simbolo = simbolo.value; });
      fila.appendChild(simbolo);

      var destino = crear('input');
      destino.type = 'text';
      destino.value = t.destino;
      destino.placeholder = 'destino';
      destino.autocomplete = 'off';
      destino.addEventListener('input', function () { t.destino = destino.value; });
      fila.appendChild(destino);

      var quitar = texto('button', 'boton-quitar', '×');
      quitar.type = 'button';
      quitar.addEventListener('click', function () {
        transicionesLibres = quitarEnIndice(transicionesLibres, indice);
        dibujarTransicionesLibres();
      });
      fila.appendChild(quitar);

      caja.appendChild(fila);
    })(i);
  }

  if (transicionesLibres.length === 0) {
    caja.appendChild(texto('p', 'nota', 'Sin transiciones cargadas.'));
  }
}

function sincronizarDependientes() {
  dibujarInicial();
  dibujarFinales();
  dibujarMatriz();
}

function dibujarFormulario() {
  dibujarEstados();
  dibujarSimbolos();
  dibujarTransicionesLibres();
  sincronizarDependientes();
}

function cambiarModoTransiciones(modo) {
  modoTransiciones = modo;
  var esMatriz = modo === 'matriz';
  porId('modoMatriz').classList.toggle('activa', esMatriz);
  porId('modoLibre').classList.toggle('activa', !esMatriz);
  porId('matrizTransiciones').classList.toggle('oculto', !esMatriz);
  porId('libreTransiciones').classList.toggle('oculto', esMatriz);
  porId('notaTransiciones').textContent = esMatriz
    ? 'Una celda por cada estado y simbolo. Dejar una en blanco deja la funcion incompleta.'
    : 'Cada fila es una transicion suelta. Sirve para provocar errores a proposito.';
}

function cuerpoDelFormulario(guardar) {
  var cuerpo = '';
  cuerpo = agregarPar(cuerpo, 'nombre', porId('campoNombre').value.trim());

  var estados = estadosValidos();
  for (var i = 0; i < estados.length; i++) {
    cuerpo = agregarPar(cuerpo, 'estado', estados[i]);
  }

  var simbolos = simbolosValidos();
  for (var j = 0; j < simbolos.length; j++) {
    cuerpo = agregarPar(cuerpo, 'simbolo', simbolos[j]);
  }

  cuerpo = agregarPar(cuerpo, 'inicial', inicialForm);

  for (var k = 0; k < finalesForm.length; k++) {
    cuerpo = agregarPar(cuerpo, 'final', finalesForm[k]);
  }

  if (modoTransiciones === 'matriz') {
    for (var m = 0; m < estados.length; m++) {
      for (var n = 0; n < simbolos.length; n++) {
        var destino = destinoMatriz(estados[m], simbolos[n]);
        if (destino.length > 0) {
          cuerpo = agregarPar(cuerpo, 'transicion', estados[m] + SEPARADOR + simbolos[n] + SEPARADOR + destino);
        }
      }
    }
  } else {
    for (var p = 0; p < transicionesLibres.length; p++) {
      var t = transicionesLibres[p];
      if (t.origen.trim().length > 0 && t.simbolo.trim().length > 0 && t.destino.trim().length > 0) {
        cuerpo = agregarPar(cuerpo, 'transicion',
          t.origen.trim() + SEPARADOR + t.simbolo.trim() + SEPARADOR + t.destino.trim());
      }
    }
  }

  cuerpo = agregarPar(cuerpo, 'guardar', guardar ? '1' : '0');
  return cuerpo;
}

function mostrarReporteCreacion(respuesta) {
  var caja = porId('reporteCrear');
  limpiar(caja);

  if (respuesta.valido) {
    var titulo = respuesta.guardado
      ? 'El automata es VALIDO y quedo guardado con el indice ' + respuesta.id + '.'
      : 'El automata es VALIDO. No se guardo porque solo pediste validarlo.';
    caja.appendChild(bloqueReporte('reporte-ok', titulo, null));
  } else {
    caja.appendChild(bloqueReporte('reporte-mal',
      'El automata NO es valido, por eso no se guarda ni queda disponible para la union:',
      respuesta.errores));
  }

  if (respuesta.avisos && respuesta.avisos.length > 0) {
    caja.appendChild(bloqueReporte('reporte-aviso', 'Avisos sobre lo que se cargo:', respuesta.avisos));
  }
}

function enviarFormulario(guardar) {
  enviarForm('/api/automatas', cuerpoDelFormulario(guardar)).then(function (respuesta) {
    mostrarReporteCreacion(respuesta);
    if (respuesta.guardado) {
      refrescarLista().then(function () {
        seleccionar(respuesta.id);
      });
    }
  }).catch(function () {
    avisarSinConexion(porId('reporteCrear'));
  });
}

// conAndamio deja las filas de arranque (q0/q1, a/b) como al abrir la pagina;
// sin andamio deja el formulario totalmente en blanco
function reiniciarFormulario(conAndamio) {
  porId('campoNombre').value = '';
  estadosForm = conAndamio ? ['q0', 'q1'] : [''];
  simbolosForm = conAndamio ? ['a', 'b'] : [''];
  finalesForm = [];
  inicialForm = '';
  matriz = [];
  transicionesLibres = [];
  limpiar(porId('reporteCrear'));
  cambiarModoTransiciones('matriz');
  dibujarFormulario();
  porId('campoNombre').focus();
}

function cargarEjemplo() {
  var par = contadorEjemplos % 2 === 0;
  contadorEjemplos++;

  if (par) {
    porId('campoNombre').value = 'TerminaEnA';
    estadosForm = ['q0', 'q1'];
    simbolosForm = ['a', 'b'];
    inicialForm = 'q0';
    finalesForm = ['q1'];
    matriz = [
      { origen: 'q0', simbolo: 'a', destino: 'q1' },
      { origen: 'q0', simbolo: 'b', destino: 'q0' },
      { origen: 'q1', simbolo: 'a', destino: 'q1' },
      { origen: 'q1', simbolo: 'b', destino: 'q0' }
    ];
  } else {
    porId('campoNombre').value = 'ParesDeB';
    estadosForm = ['p0', 'p1'];
    simbolosForm = ['a', 'b'];
    inicialForm = 'p0';
    finalesForm = ['p0'];
    matriz = [
      { origen: 'p0', simbolo: 'a', destino: 'p0' },
      { origen: 'p0', simbolo: 'b', destino: 'p1' },
      { origen: 'p1', simbolo: 'a', destino: 'p1' },
      { origen: 'p1', simbolo: 'b', destino: 'p0' }
    ];
  }

  cambiarModoTransiciones('matriz');
  limpiar(porId('reporteCrear'));
  dibujarFormulario();
}

/* ============ selectores ============ */

function llenarSelector(seleccion, valorPrevio) {
  limpiar(seleccion);
  if (automatas.length === 0) {
    var vacia = crear('option');
    vacia.value = '';
    vacia.textContent = '-- no hay automatas --';
    seleccion.appendChild(vacia);
    return;
  }
  for (var i = 0; i < automatas.length; i++) {
    var opcion = crear('option');
    opcion.value = automatas[i].id;
    opcion.textContent = '[' + automatas[i].id + '] ' + automatas[i].nombre;
    seleccion.appendChild(opcion);
  }
  if (valorPrevio !== '' && valorPrevio !== null && buscarAutomata(parseInt(valorPrevio, 10)) !== null) {
    seleccion.value = valorPrevio;
  }
}

function llenarSelectores() {
  var ids = ['unionA', 'unionB', 'probarSolo', 'probarUnion', 'probarA', 'probarB'];
  for (var i = 0; i < ids.length; i++) {
    var seleccion = porId(ids[i]);
    llenarSelector(seleccion, seleccion.value);
  }
}

/* ============ construccion paso a paso de la union ============ */

function conjunto(valores) {
  if (valores.length === 0) {
    return '∅';
  }
  var salida = '';
  for (var i = 0; i < valores.length; i++) {
    if (i > 0) {
      salida += ', ';
    }
    salida += valores[i];
  }
  return '{' + salida + '}';
}

function bloqueConTitulo(titulo) {
  var bloque = crear('div', 'bloque');
  var cabecera = crear('div', 'bloque-titulo');
  cabecera.appendChild(texto('h4', null, titulo));
  bloque.appendChild(cabecera);
  var cuerpo = crear('div');
  bloque.appendChild(cuerpo);
  return { bloque: bloque, cuerpo: cuerpo };
}

function lineaFormal(sub, nombre, estados, alfabeto, inicial, finales) {
  var caja = crear('div', 'formal-item');
  caja.appendChild(texto('div', 'formal-simbolica',
    'M' + sub + ' = (Q' + sub + ', Σ, δ' + sub + ', q' + sub + ', F' + sub + ')   ·   ' + nombre));
  caja.appendChild(texto('div', 'formal-concreta',
    'Q' + sub + ' = ' + conjunto(estados) +
    '     Σ = ' + conjunto(alfabeto) +
    '     q' + sub + ' = ' + inicial +
    '     F' + sub + ' = ' + conjunto(finales)));
  return caja;
}

// arma la linea  δ((r1,r2), a) = (δ1(r1,a), δ2(r2,a)) = (s1,s2)
function lineaDerivacion(paso, transicion) {
  var linea = crear('div', 'derivacion');
  var simbolo = transicion.simbolo;

  linea.appendChild(texto('span', null, 'δ('));
  linea.appendChild(texto('span', 'res-par', paso.par));
  linea.appendChild(texto('span', null, ', '));
  linea.appendChild(texto('span', 'res-simbolo', simbolo));
  linea.appendChild(texto('span', null, ') = (δ₁(' + paso.a + ',' + simbolo +
    '), δ₂(' + paso.b + ',' + simbolo + ')) = '));

  if (transicion.definida) {
    linea.appendChild(texto('span', null, '('));
    linea.appendChild(texto('span', 'res-parte', transicion.destinoA));
    linea.appendChild(texto('span', null, ','));
    linea.appendChild(texto('span', 'res-parte', transicion.destinoB));
    linea.appendChild(texto('span', null, ') = '));
    linea.appendChild(texto('span', 'res-final', transicion.destino));
  } else {
    linea.appendChild(texto('span', 'res-indefinida', 'no definida'));
  }
  return linea;
}

function lineaAceptacion(paso) {
  var linea = crear('div', 'derivacion');
  linea.appendChild(texto('span', 'res-par', paso.par));
  linea.appendChild(texto('span', null, ' : '));
  linea.appendChild(texto('span', null, paso.a + ' ∈ F₁ ? '));
  linea.appendChild(texto('span', paso.finalA ? 'res-si' : 'res-no', paso.finalA ? 'si' : 'no'));
  linea.appendChild(texto('span', null, '   o   ' + paso.b + ' ∈ F₂ ? '));
  linea.appendChild(texto('span', paso.finalB ? 'res-si' : 'res-no', paso.finalB ? 'si' : 'no'));
  linea.appendChild(texto('span', null, '   ⇒   '));
  linea.appendChild(texto('span', paso.esFinal ? 'res-final' : 'res-indefinida',
    paso.esFinal ? 'es de aceptacion' : 'no es de aceptacion'));
  return linea;
}

function construirConstruccion(c) {
  var caja = crear('div');

  var uno = bloqueConTitulo('1 · Definicion formal de partida');
  var formal = crear('div', 'formal');
  formal.appendChild(lineaFormal('₁', c.nombreA, c.estadosA, c.alfabeto, c.inicialA, c.finalesA));
  formal.appendChild(lineaFormal('₂', c.nombreB, c.estadosB, c.alfabeto, c.inicialB, c.finalesB));
  var destacada = crear('div', 'formal-item formal-destacada');
  destacada.appendChild(texto('div', 'formal-simbolica', 'M = (Q₁ × Q₂, Σ, δ, (q₁,q₂), F)'));
  destacada.appendChild(texto('div', 'formal-concreta',
    'δ((r₁,r₂), a) = (δ₁(r₁,a), δ₂(r₂,a))      F = { (r₁,r₂) : r₁ ∈ F₁  o  r₂ ∈ F₂ }'));
  formal.appendChild(destacada);
  uno.cuerpo.appendChild(formal);
  caja.appendChild(uno.bloque);

  var dos = bloqueConTitulo('2 · Producto cartesiano  Q₁ × Q₂');
  dos.cuerpo.appendChild(texto('p', 'nota',
    c.estadosA.length + ' × ' + c.estadosB.length + ' = ' + c.pasos.length + ' estados compuestos'));
  var pares = [];
  for (var i = 0; i < c.pasos.length; i++) {
    pares.push(c.pasos[i].par);
  }
  dos.cuerpo.appendChild(fichas(pares, null, 'ninguno'));
  caja.appendChild(dos.bloque);

  var tres = bloqueConTitulo('3 · Estado inicial');
  var lineaInicial = crear('div', 'derivacion');
  lineaInicial.appendChild(texto('span', null, '(q₁, q₂) = ('));
  lineaInicial.appendChild(texto('span', 'res-parte', c.inicialA));
  lineaInicial.appendChild(texto('span', null, ', '));
  lineaInicial.appendChild(texto('span', 'res-parte', c.inicialB));
  lineaInicial.appendChild(texto('span', null, ') = '));
  lineaInicial.appendChild(texto('span', 'res-final', c.inicialPar));
  tres.cuerpo.appendChild(lineaInicial);
  caja.appendChild(tres.bloque);

  var cuatro = bloqueConTitulo('4 · Funcion de transicion δ');
  var rejilla = crear('div', 'rejilla-pasos');
  for (var j = 0; j < c.pasos.length; j++) {
    var paso = c.pasos[j];
    var tarjeta = crear('div', 'paso-par');
    tarjeta.appendChild(texto('div', 'paso-par-titulo', paso.par));
    for (var k = 0; k < paso.transiciones.length; k++) {
      tarjeta.appendChild(lineaDerivacion(paso, paso.transiciones[k]));
    }
    rejilla.appendChild(tarjeta);
  }
  cuatro.cuerpo.appendChild(rejilla);
  caja.appendChild(cuatro.bloque);

  var cinco = bloqueConTitulo('5 · Estados de aceptacion  (F₁ o F₂)');
  for (var m = 0; m < c.pasos.length; m++) {
    cinco.cuerpo.appendChild(lineaAceptacion(c.pasos[m]));
  }
  caja.appendChild(cinco.bloque);

  return caja;
}

/* ============ union ============ */

function ejecutarUnion() {
  var idA = porId('unionA').value;
  var idB = porId('unionB').value;
  var caja = porId('reporteUnion');
  limpiar(caja);

  if (idA === '' || idB === '') {
    caja.appendChild(bloqueReporte('reporte-mal', 'Hacen falta dos automatas guardados.', null));
    return;
  }

  var cuerpo = agregarPar(agregarPar('', 'idA', idA), 'idB', idB);
  enviarForm('/api/unir', cuerpo).then(function (respuesta) {
    limpiar(caja);

    if (!respuesta.ok) {
      caja.appendChild(bloqueReporte('reporte-mal', respuesta.error, null));
      return;
    }
    if (!respuesta.exitoso) {
      porId('unionResultado').classList.add('oculto');
      var motivos = [respuesta.error];
      if (respuesta.errores) {
        for (var i = 0; i < respuesta.errores.length; i++) {
          motivos.push(respuesta.errores[i]);
        }
      }
      caja.appendChild(bloqueReporte('reporte-mal', 'No se pudo unir.', motivos));
      return;
    }

    caja.appendChild(bloqueReporte('reporte-ok',
      'Union generada y guardada con el indice ' + respuesta.id + '.', null));

    var automata = respuesta.automata;
    porId('unionResultado').classList.remove('oculto');
    porId('unionNombre').textContent = automata.nombre;

    var construccion = porId('construccionUnion');
    limpiar(construccion);
    construccion.appendChild(construirConstruccion(respuesta.construccion));

    if (grafoUnion === null) {
      grafoUnion = crearVistaGrafo(porId('lienzoUnion'));
    }
    grafoUnion.cargar(automata);

    var resumen = porId('resumenUnion');
    limpiar(resumen);
    resumen.appendChild(construirResumen(automata));

    var tabla = porId('tablaUnion');
    limpiar(tabla);
    tabla.appendChild(construirTabla(automata));

    // deja lista la prueba del veredicto triple con esta union
    refrescarLista().then(function () {
      cambiarModoPrueba('union');
      porId('probarUnion').value = respuesta.id;
      porId('probarA').value = respuesta.idA;
      porId('probarB').value = respuesta.idB;
      porId('probarSolo').value = respuesta.id;
    });
  }).catch(function () {
    avisarSinConexion(caja);
  });
}

/* ============ probar cadena ============ */

function cambiarModoPrueba(modo) {
  modoPrueba = modo;
  var esSimple = modo === 'simple';
  porId('modoSimple').classList.toggle('activa', esSimple);
  porId('modoUnion').classList.toggle('activa', !esSimple);
  porId('selectoresSimple').classList.toggle('oculto', !esSimple);
  porId('selectoresUnion').classList.toggle('oculto', esSimple);
}

function ejecutarPrueba() {
  var caja = porId('reporteProbar');
  limpiar(caja);

  var cuerpo = agregarPar('', 'modo', modoPrueba);

  if (modoPrueba === 'simple') {
    var id = porId('probarSolo').value;
    if (id === '') {
      caja.appendChild(bloqueReporte('reporte-mal', 'Elegi un automata guardado.', null));
      return;
    }
    cuerpo = agregarPar(cuerpo, 'id', id);
  } else {
    var idUnion = porId('probarUnion').value;
    var idA = porId('probarA').value;
    var idB = porId('probarB').value;
    if (idUnion === '' || idA === '' || idB === '') {
      caja.appendChild(bloqueReporte('reporte-mal', 'Hacen falta los tres automatas.', null));
      return;
    }
    cuerpo = agregarPar(cuerpo, 'idUnion', idUnion);
    cuerpo = agregarPar(cuerpo, 'idA', idA);
    cuerpo = agregarPar(cuerpo, 'idB', idB);
  }

  cuerpo = agregarPar(cuerpo, 'cadena', porId('campoCadena').value.trim());

  enviarForm('/api/probar-cadena', cuerpo).then(function (respuesta) {
    if (!respuesta.ok) {
      caja.appendChild(bloqueReporte('reporte-mal', respuesta.error, null));
      return;
    }
    corrida = respuesta;
    prepararCorrida();
  }).catch(function () {
    avisarSinConexion(caja);
  });
}

function recorridoPrincipal() {
  return corrida.recorridos[corrida.principal];
}

function bloqueGrafo(titulo, grande) {
  var bloque = crear('div', 'bloque');
  var cabecera = crear('div', 'bloque-titulo');
  cabecera.appendChild(texto('h4', null, titulo));
  bloque.appendChild(cabecera);
  var lienzo = crear('div', grande ? 'lienzo lienzo-alto' : 'lienzo');
  bloque.appendChild(lienzo);
  return { bloque: bloque, lienzo: lienzo };
}

function prepararCorrida() {
  detenerReproduccion();
  porId('probarResultado').classList.remove('oculto');

  dibujarVeredictos();

  var cajaPrincipal = porId('grafoPrincipal');
  var cajaSecundarios = porId('grafosSecundarios');
  limpiar(cajaPrincipal);
  limpiar(cajaSecundarios);

  var recorridos = corrida.recorridos;
  grafosCorrida = [];
  for (var i = 0; i < recorridos.length; i++) {
    grafosCorrida.push(null);
  }

  // el principal va grande arriba; si hay mas, van abajo en dos columnas
  var indice = corrida.principal;
  var pieza = bloqueGrafo(recorridos[indice].rol + ' · ' + recorridos[indice].nombre, true);
  cajaPrincipal.appendChild(pieza.bloque);
  grafosCorrida[indice] = crearVistaGrafo(pieza.lienzo);

  for (var j = 0; j < recorridos.length; j++) {
    if (j === indice) {
      continue;
    }
    var otra = bloqueGrafo(recorridos[j].rol + ' · ' + recorridos[j].nombre, false);
    cajaSecundarios.appendChild(otra.bloque);
    grafosCorrida[j] = crearVistaGrafo(otra.lienzo);
  }

  for (var k = 0; k < recorridos.length; k++) {
    var automata = buscarAutomata(recorridos[k].id);
    if (automata !== null) {
      grafosCorrida[k].cargar(automata);
    }
  }

  porId('tituloTraza').textContent = corrida.modo === 'union'
    ? 'Secuencia de estados compuestos'
    : 'Secuencia de estados';

  var gorro = porId('deltaGorro');
  limpiar(gorro);
  gorro.appendChild(construirDeltaGorro());

  var deslizador = porId('deslizador');
  deslizador.min = 0;
  deslizador.max = corrida.cadena.length;
  deslizador.value = 0;

  irAlPaso(0);
}

function tarjetaVeredicto(recorrido) {
  var acepta = recorrido.acepta;
  var tarjeta = crear('div', 'veredicto ' + (acepta ? 'aceptada' : 'rechazada'));
  tarjeta.appendChild(texto('div', 'veredicto-titulo', recorrido.rol));
  tarjeta.appendChild(texto('div', 'veredicto-nombre', recorrido.nombre));
  tarjeta.appendChild(texto('div', 'veredicto-estado', acepta ? 'ACEPTADA' : 'RECHAZADA'));
  if (!recorrido.completa) {
    tarjeta.appendChild(texto('p', 'nota',
      'Se traba en el simbolo «' + recorrido.simboloTrabado + '», no hay transicion definida.'));
  }
  return tarjeta;
}

function dibujarVeredictos() {
  var caja = porId('veredictos');
  limpiar(caja);
  for (var i = 0; i < corrida.recorridos.length; i++) {
    caja.appendChild(tarjetaVeredicto(corrida.recorridos[i]));
  }
}

function dibujarCinta(paso) {
  var caja = porId('cintaCadena');
  limpiar(caja);

  if (corrida.cadena.length === 0) {
    caja.appendChild(texto('span', 'cinta-vacia', 'cadena vacia (ε)'));
    return;
  }

  var principal = recorridoPrincipal();
  for (var i = 0; i < corrida.cadena.length; i++) {
    var clase = 'celda-cinta';
    if (!principal.completa && i === principal.consumidos) {
      clase += ' trabada';
    } else if (i < paso) {
      clase += ' consumida';
    } else if (i === paso) {
      clase += ' actual';
    }
    caja.appendChild(texto('div', clase, corrida.cadena[i]));
  }
}

function dibujarTraza(paso) {
  var caja = porId('listaTraza');
  limpiar(caja);
  var principal = recorridoPrincipal();
  var traza = principal.traza;

  if (traza.length === 0) {
    caja.appendChild(texto('span', 'nota', 'El automata no tiene estado inicial.'));
    return;
  }

  for (var i = 0; i < traza.length; i++) {
    if (i > 0) {
      caja.appendChild(texto('span', 'flecha-traza', '--' + corrida.cadena[i - 1] + '→'));
    }
    var ficha = texto('button', 'paso-traza' + (i === paso ? ' actual' : ''), traza[i]);
    (function (indice) {
      ficha.addEventListener('click', function () {
        irAlPaso(indice);
      });
    })(i);
    caja.appendChild(ficha);
  }

  if (!principal.completa) {
    caja.appendChild(texto('span', 'flecha-traza',
      '--' + principal.simboloTrabado + '→  sin transicion, el recorrido se detiene'));
  }
}

function resaltarRecorrido(grafo, recorrido, paso) {
  var traza = recorrido.traza;
  if (traza.length === 0) {
    return;
  }

  var trabado = paso >= traza.length;
  var indice = trabado ? traza.length - 1 : paso;
  var estado = traza[indice];

  var clase = 'nodo-activo';
  var enElFinal = paso >= corrida.cadena.length;
  if (trabado) {
    clase = 'nodo-rechazado';
  } else if (enElFinal && recorrido.completa) {
    clase = recorrido.acepta ? 'nodo-aceptado' : 'nodo-rechazado';
  }

  if (indice > 0 && !trabado) {
    grafo.resaltar(estado, traza[indice - 1], corrida.cadena[indice - 1], estado, clase);
  } else {
    grafo.resaltar(estado, null, null, null, clase);
  }
}

function irAlPaso(paso) {
  if (corrida === null) {
    return;
  }
  if (paso < 0) { paso = 0; }
  if (paso > corrida.cadena.length) { paso = corrida.cadena.length; }
  pasoActual = paso;

  porId('deslizador').value = paso;
  porId('marcadorPaso').textContent = 'paso ' + paso + ' de ' + corrida.cadena.length;

  dibujarCinta(paso);
  dibujarTraza(paso);

  for (var i = 0; i < corrida.recorridos.length; i++) {
    if (grafosCorrida[i] !== null && grafosCorrida[i] !== undefined) {
      resaltarRecorrido(grafosCorrida[i], corrida.recorridos[i], paso);
    }
  }
}

/* ---- desarrollo de la funcion de transicion extendida ---- */

function lineaResolucion(recorrido, inicial, indice) {
  var cadena = corrida.cadena;
  var linea = crear('div', 'derivacion');
  linea.appendChild(texto('span', null, 'δ̂(' + inicial + ', '));
  linea.appendChild(texto('span', 'res-par', indice === 0 ? 'ε' : cadena.substring(0, indice)));
  linea.appendChild(texto('span', null, ') = '));

  if (indice === 0) {
    linea.appendChild(texto('span', 'res-final', recorrido.traza[0]));
    linea.appendChild(texto('span', null, '     (caso base)'));
    return linea;
  }

  var simbolo = cadena[indice - 1];
  var previo = indice === 1 ? 'ε' : cadena.substring(0, indice - 1);

  // primero se aplica la regla recursiva, recien despues se resuelve el interior
  linea.appendChild(texto('span', null, 'δ(δ̂(' + inicial + ', '));
  linea.appendChild(texto('span', 'res-par', previo));
  linea.appendChild(texto('span', null, '), '));
  linea.appendChild(texto('span', 'res-simbolo', simbolo));
  linea.appendChild(texto('span', null, ') = δ('));
  linea.appendChild(texto('span', 'res-parte', recorrido.traza[indice - 1]));
  linea.appendChild(texto('span', null, ', '));
  linea.appendChild(texto('span', 'res-simbolo', simbolo));
  linea.appendChild(texto('span', null, ') = '));
  linea.appendChild(texto('span', 'res-final', recorrido.traza[indice]));
  return linea;
}

function lineaVeredictoFormal(recorrido, automata, inicial) {
  var cadena = corrida.cadena;
  var w = cadena.length === 0 ? 'ε' : cadena;
  var linea = crear('div', 'derivacion');

  if (!recorrido.completa) {
    linea.appendChild(texto('span', null, 'δ̂(' + inicial + ', ' + w + ') no esta definida   ⇒   '));
    linea.appendChild(texto('span', 'res-no', w + ' ∉ L(M)'));
    return linea;
  }

  var ultimo = recorrido.traza[recorrido.traza.length - 1];
  linea.appendChild(texto('span', null, 'δ̂(' + inicial + ', '));
  linea.appendChild(texto('span', 'res-par', w));
  linea.appendChild(texto('span', null, ') = '));
  linea.appendChild(texto('span', 'res-final', ultimo));
  linea.appendChild(texto('span', null, '     ' + ultimo + ' ∈ F = ' + conjunto(automata.finales) + ' ? '));
  linea.appendChild(texto('span', recorrido.acepta ? 'res-si' : 'res-no', recorrido.acepta ? 'si' : 'no'));
  linea.appendChild(texto('span', null, '   ⇒   ' + w + (recorrido.acepta ? ' ∈ ' : ' ∉ ') + 'L(M)'));
  return linea;
}

function bloqueDeltaGorro(recorrido) {
  var pieza = bloqueConTitulo(recorrido.rol + ' · ' + recorrido.nombre);
  var automata = buscarAutomata(recorrido.id);
  if (automata === null || recorrido.traza.length === 0) {
    pieza.cuerpo.appendChild(texto('p', 'nota', 'Sin estado inicial, no hay recorrido posible.'));
    return pieza.bloque;
  }

  var inicial = automata.inicial;

  var ficha = crear('div', 'formal-item');
  ficha.appendChild(texto('div', 'formal-concreta',
    'Q = ' + conjunto(automata.estados) +
    '     Σ = ' + conjunto(automata.alfabeto) +
    '     q₀ = ' + inicial +
    '     F = ' + conjunto(automata.finales)));
  pieza.cuerpo.appendChild(ficha);

  pieza.cuerpo.appendChild(texto('div', 'sub-titulo', 'Resolucion desde el caso base'));
  for (var i = 0; i < recorrido.traza.length; i++) {
    pieza.cuerpo.appendChild(lineaResolucion(recorrido, inicial, i));
  }

  if (!recorrido.completa) {
    var aviso = crear('div', 'derivacion');
    if (!contieneTexto(automata.alfabeto, recorrido.simboloTrabado)) {
      aviso.appendChild(texto('span', 'res-indefinida',
        'El simbolo ' + recorrido.simboloTrabado + ' no pertenece a Σ, la cadena no esta sobre el alfabeto de este automata.'));
    } else {
      aviso.appendChild(texto('span', 'res-indefinida',
        'δ(' + recorrido.traza[recorrido.consumidos] + ', ' + recorrido.simboloTrabado + ') no esta definida.'));
    }
    pieza.cuerpo.appendChild(aviso);
  }

  pieza.cuerpo.appendChild(texto('div', 'sub-titulo', 'Criterio de aceptacion'));
  pieza.cuerpo.appendChild(lineaVeredictoFormal(recorrido, automata, inicial));

  return pieza.bloque;
}

function construirDeltaGorro() {
  var caja = crear('div');

  var definicion = bloqueConTitulo('Definicion de la transicion extendida δ̂');
  var formal = crear('div', 'formal');
  var item = crear('div', 'formal-item formal-destacada');
  item.appendChild(texto('div', 'formal-simbolica', 'δ̂(q, ε) = q'));
  item.appendChild(texto('div', 'formal-simbolica', 'δ̂(q, wa) = δ(δ̂(q, w), a)'));
  item.appendChild(texto('div', 'formal-concreta',
    'Criterio de aceptacion:     w ∈ L(M)   ⟺   δ̂(q₀, w) ∈ F'));
  formal.appendChild(item);
  definicion.cuerpo.appendChild(formal);
  caja.appendChild(definicion.bloque);

  for (var i = 0; i < corrida.recorridos.length; i++) {
    caja.appendChild(bloqueDeltaGorro(corrida.recorridos[i]));
  }
  return caja;
}

function detenerReproduccion() {
  if (temporizador !== null) {
    clearInterval(temporizador);
    temporizador = null;
  }
  porId('pasoPlay').textContent = 'Reproducir';
}

function alternarReproduccion() {
  if (temporizador !== null) {
    detenerReproduccion();
    return;
  }
  if (corrida === null) {
    return;
  }
  if (pasoActual >= corrida.cadena.length) {
    irAlPaso(0);
  }
  porId('pasoPlay').textContent = 'Pausar';
  temporizador = setInterval(function () {
    if (pasoActual >= corrida.cadena.length) {
      detenerReproduccion();
      return;
    }
    irAlPaso(pasoActual + 1);
  }, 850);
}

/* ============ pestanas y arranque ============ */

function mostrarPestana(nombre) {
  var botones = document.querySelectorAll('.pestana');
  for (var i = 0; i < botones.length; i++) {
    botones[i].classList.toggle('activa', botones[i].getAttribute('data-vista') === nombre);
  }

  var vistas = document.querySelectorAll('.vista');
  for (var j = 0; j < vistas.length; j++) {
    vistas[j].classList.remove('activa');
  }

  var destino = 'vistaCrear';
  if (nombre === 'detalle') { destino = 'vistaDetalle'; }
  if (nombre === 'union') { destino = 'vistaUnion'; }
  if (nombre === 'probar') { destino = 'vistaProbar'; }
  porId(destino).classList.add('activa');

  // los lienzos ocultos miden cero, hay que reencuadrar al mostrarlos
  if (nombre === 'detalle' && grafoDetalle !== null) { grafoDetalle.refrescar(); }
  if (nombre === 'union' && grafoUnion !== null) { grafoUnion.refrescar(); }
  if (nombre === 'probar') {
    for (var k = 0; k < grafosCorrida.length; k++) {
      if (grafosCorrida[k] !== null && grafosCorrida[k] !== undefined) {
        grafosCorrida[k].refrescar();
      }
    }
  }
}

function iniciar() {
  var pestanas = document.querySelectorAll('.pestana');
  for (var i = 0; i < pestanas.length; i++) {
    (function (boton) {
      boton.addEventListener('click', function () {
        mostrarPestana(boton.getAttribute('data-vista'));
      });
    })(pestanas[i]);
  }

  porId('botonNuevo').addEventListener('click', function () {
    mostrarPestana('crear');
    reiniciarFormulario(true);
  });

  porId('botonAgregarEstado').addEventListener('click', function () {
    estadosForm.push('');
    dibujarEstados();
  });

  porId('botonAgregarSimbolo').addEventListener('click', function () {
    simbolosForm.push('');
    dibujarSimbolos();
  });

  porId('botonAgregarTransicion').addEventListener('click', function () {
    transicionesLibres.push({ origen: '', simbolo: '', destino: '' });
    dibujarTransicionesLibres();
  });

  porId('campoInicial').addEventListener('change', function () {
    inicialForm = porId('campoInicial').value;
  });

  porId('modoMatriz').addEventListener('click', function () { cambiarModoTransiciones('matriz'); });
  porId('modoLibre').addEventListener('click', function () { cambiarModoTransiciones('libre'); });

  porId('modoSimple').addEventListener('click', function () { cambiarModoPrueba('simple'); });
  porId('modoUnion').addEventListener('click', function () { cambiarModoPrueba('union'); });

  porId('botonValidar').addEventListener('click', function () { enviarFormulario(false); });
  porId('botonGuardar').addEventListener('click', function () { enviarFormulario(true); });
  porId('botonEjemplo').addEventListener('click', cargarEjemplo);
  porId('botonLimpiar').addEventListener('click', function () {
    reiniciarFormulario(false);
  });

  porId('botonUnir').addEventListener('click', ejecutarUnion);
  porId('botonProbar').addEventListener('click', ejecutarPrueba);

  porId('campoCadena').addEventListener('keydown', function (evento) {
    if (evento.key === 'Enter') {
      ejecutarPrueba();
    }
  });

  porId('pasoInicio').addEventListener('click', function () { detenerReproduccion(); irAlPaso(0); });
  porId('pasoAtras').addEventListener('click', function () { detenerReproduccion(); irAlPaso(pasoActual - 1); });
  porId('pasoAdelante').addEventListener('click', function () { detenerReproduccion(); irAlPaso(pasoActual + 1); });
  porId('pasoFin').addEventListener('click', function () {
    detenerReproduccion();
    irAlPaso(corrida === null ? 0 : corrida.cadena.length);
  });
  porId('pasoPlay').addEventListener('click', alternarReproduccion);
  porId('deslizador').addEventListener('input', function () {
    detenerReproduccion();
    irAlPaso(parseInt(porId('deslizador').value, 10));
  });

  cambiarModoPrueba('simple');
  reiniciarFormulario(true);
  refrescarLista();
}

iniciar();
