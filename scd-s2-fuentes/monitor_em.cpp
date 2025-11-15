// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: monitor_em.cpp
// Ejemplo de monitores en C++11 sin variables condición
// (solo con encapsulamiento y exclusión mutua)
//
// -- MContador1 : sin E.M., únicamente encapsulamiento // Usado para probar la condición de carrera
// -- MContador2 : con E.M. mediante clase base 'HoareMonitor' y MRef // Usado para sincronización
//
// Historial:
// Julio 2017: creado
// Sept 2022 : se quita MContador3 antiguo y se adapta MContador2 para usar HoareMonitor
// --------------------------------------------------------------------------------------

#include <iostream>      // Para operaciones de entrada/salida (cout)
#include <cassert>       // Para aserciones de debug
#include <thread>        // Para manejar hebras (threads)
#include <mutex>         // Para exclusión mutua (aunque HoareMonitor lo gestiona)
#include <random>        // Para números aleatorios (no usado directamente)
#include "scd.h"         // Librería que contiene la implementación de HoareMonitor y MRef

#include <chrono>        // Para medir tiempos de ejecución
using namespace std;     // Usa el espacio de nombres estándar
using namespace scd;     // Usa el espacio de nombres de la librería SCD
using namespace std::chrono; // Para simplificar el acceso a las clases de tiempo (steady_clock, time_point, duration)

const int num_incrementos = 10000; // Número de incrementos que cada hebra realizará

// *****************************************************************************
// clase contador, sin exclusión mutua

class MContador1
{
    private:
    int cont; // Contador interno, NO protegido por un mutex.

    public:
    MContador1( int valor_ini ); // Constructor
    void incrementa();           // Método público para incrementar
    int leer_valor();            // Método público para leer
};
// -----------------------------------------------------------------------------

MContador1::MContador1( int valor_ini )
{
    cont = valor_ini; // Inicializa el contador
}
// -----------------------------------------------------------------------------

void MContador1::incrementa()
{
    cont++; // Operación de incremento vulnerable a la condición de carrera
}
// -----------------------------------------------------------------------------

int MContador1::leer_valor()
{
    return cont; // Devuelve el valor
}

// *****************************************************************************
// clase contador, con exclusión mutua mediante herencia de 'HoareMonitor'

class MContador2 : public HoareMonitor // Hereda de HoareMonitor, adquiriendo su mecanismo de EM
{
    private:
    int cont; // Contador interno, protegido por el monitor

    public:
    MContador2( int valor_ini ); // Constructor
    void incrementa();           // Protegido por el monitor
    int leer_valor();            // Protegido por el monitor
};

// -----------------------------------------------------------------------------

MContador2::MContador2( int valor_ini )
{
    cont = valor_ini; // Inicializa el contador
}
// -----------------------------------------------------------------------------

void MContador2::incrementa()
{
    cont++; // El acceso a 'cont' está serializado (un solo hilo a la vez)
}
// -----------------------------------------------------------------------------

int MContador2::leer_valor()
{
    return cont;
}

// *****************************************************************************

void funcion_hebra_M1( MContador1 & monitor ) // Función que ejecuta las hebras para MContador1
{
    for( int i = 0 ; i < num_incrementos ; i++ )
        monitor.incrementa(); // Llama a la función no sincronizada
}
// -----------------------------------------------------------------------------

void test_1() // Caso de prueba SIN Exclusión Mutua
{
    MContador1 monitor(0); // Crea una instancia de contador no sincronizado

    // Inicia la medición de tiempo justo antes de lanzar las hebras
    time_point<steady_clock> inicio = steady_clock::now();
    // Lanza dos hebras, pasando el monitor por referencia ('ref')
    thread hebra1( funcion_hebra_M1, ref(monitor) ),
           hebra2( funcion_hebra_M1, ref(monitor) );

    hebra1.join(); // Espera a que la hebra 1 termine
    hebra2.join(); // Espera a que la hebra 2 termine

    // Finaliza la medición de tiempo
    time_point<steady_clock> fin = steady_clock::now();
    chrono::duration<double, milli> duracion = fin - inicio; // Calcula la duración en milisegundos

    cout << "Monitor contador (sin exclusión mutua):" << endl
         << endl
         << "   valor esperado == " << 2*num_incrementos << endl // Valor esperado = 20000
         << "   valor obtenido == " << monitor.leer_valor() << endl // Valor real (probablemente incorrecto)
         << " TIEMPO: " << duracion.count() << endl // Muestra el tiempo total
         << endl;
}
// *****************************************************************************

void funcion_hebra_M2( MRef<MContador2> monitor) // Función que ejecuta las hebras para MContador2
{
    for( int i = 0 ; i < num_incrementos ; i++ )
        monitor->incrementa(); // Llama al método a través de MRef, que garantiza la EM
}
// -----------------------------------------------------------------------------

void test_2() // Caso de prueba CON Exclusión Mutua (Monitor de Hoare)
{
    // Crea el monitor de Hoare, inicializado a 0
    MRef<MContador2> monitor = Create<MContador2>(0);

    // Inicia la medición de tiempo
    time_point<steady_clock> inicio = steady_clock::now();
    // Lanza dos hebras, pasando el objeto MRef
    thread hebra1( funcion_hebra_M2, monitor);
    thread hebra2( funcion_hebra_M2, monitor);

    hebra1.join(); // Espera a que la hebra 1 termine
    hebra2.join(); // Espera a que la hebra 2 termine

    // Finaliza la medición de tiempo
    time_point<steady_clock> fin = steady_clock::now();
    chrono::duration<double, milli> duracion = fin - inicio; // Calcula la duración en milisegundos

    cout << "Monitor contador (EM usando clase derivada de HoareMonitor):" << endl
         << endl
         << "   valor esperado == " << 2*num_incrementos << endl // Valor esperado = 20000
         << "   valor obtenido == " << monitor->leer_valor() << endl // Valor real (debería ser correcto)
         << " TIEMPO: " << duracion.count() << endl // Muestra el tiempo total
         << endl;
}
// *****************************************************************************

int main()
{
    test_1(); // Ejecuta el caso sin sincronización
    test_2(); // Ejecuta el caso con sincronización (monitor)
}