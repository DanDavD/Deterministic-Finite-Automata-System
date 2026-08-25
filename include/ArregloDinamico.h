#ifndef ARREGLO_DINAMICO_H
#define ARREGLO_DINAMICO_H

// Arreglo dinamico implementado desde cero, en lugar de usar std::vector.
// Solo ofrece las operaciones primitivas de un arreglo (agregar al final,
// obtener por indice, eliminar por indice, tamano, vaciar).
template <typename T>
class ArregloDinamico {
public:
    ArregloDinamico()
        : datos(nullptr), capacidad(0), cantidad(0) {
        redimensionar(4);
    }

    ArregloDinamico(const ArregloDinamico<T>& otro)
        : datos(nullptr), capacidad(0), cantidad(0) {
        copiarDesde(otro);
    }

    ArregloDinamico<T>& operator=(const ArregloDinamico<T>& otro) {
        if (this != &otro) {
            liberar();
            copiarDesde(otro);
        }
        return *this;
    }

    ~ArregloDinamico() {
        liberar();
    }

    // Agrega un elemento al final del arreglo, redimensionando si hace falta.
    void agregar(const T& valor) {
        if (cantidad == capacidad) {
            redimensionar(capacidad == 0 ? 4 : capacidad * 2);
        }
        datos[cantidad] = valor;
        cantidad++;
    }

    // Elimina el elemento en la posicion "indice", recorriendo manualmente
    // el arreglo para desplazar los elementos restantes una posicion.
    bool eliminarEnIndice(int indice) {
        if (indice < 0 || indice >= cantidad) {
            return false;
        }
        for (int i = indice; i < cantidad - 1; i++) {
            datos[i] = datos[i + 1];
        }
        cantidad--;
        return true;
    }

    T& obtener(int indice) {
        return datos[indice];
    }

    const T& obtener(int indice) const {
        return datos[indice];
    }

    int tamano() const {
        return cantidad;
    }

    bool estaVacio() const {
        return cantidad == 0;
    }

    void vaciar() {
        cantidad = 0;
    }

private:
    T* datos;
    int capacidad;
    int cantidad;

    void redimensionar(int nuevaCapacidad) {
        T* datosNuevos = new T[nuevaCapacidad];
        for (int i = 0; i < cantidad; i++) {
            datosNuevos[i] = datos[i];
        }
        delete[] datos;
        datos = datosNuevos;
        capacidad = nuevaCapacidad;
    }

    void copiarDesde(const ArregloDinamico<T>& otro) {
        capacidad = otro.capacidad;
        cantidad = otro.cantidad;
        datos = new T[capacidad];
        for (int i = 0; i < cantidad; i++) {
            datos[i] = otro.datos[i];
        }
    }

    void liberar() {
        delete[] datos;
        datos = nullptr;
        capacidad = 0;
        cantidad = 0;
    }
};

#endif // ARREGLO_DINAMICO_H
