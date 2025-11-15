// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 2. Casos prácticos de monitores en C++11.
//
// Archivo: prodcons_mu.cpp 
//
// Múltiples productores y consumidores. Opcion FIFO.
// MODIFICADO: Añadidos mensajes de inicio de hebra.
//
// -----------------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

// Constantes para múltiples productores/consumidores
constexpr int
    num_prod = 5,                // Número de hebras productoras (np)
    num_cons = 3;                // Número de hebras consumidoras (nc)

constexpr int
    num_items = 15 ;             // Número total de items (m)
    
static_assert( num_items % num_prod == 0, "num_items debe ser divisible por num_prod" );
static_assert( num_items % num_cons == 0, "num_items debe ser divisible por num_cons" );

constexpr int
    items_prod = num_items / num_prod,   // Items por productor (p)
    items_cons = num_items / num_cons;   // Items por consumidor (c)

    
constexpr int           
    min_ms      = 5,   // tiempo minimo de espera en sleep_for
    max_ms      = 20 ; // tiempo máximo de espera en sleep_for


mutex
    mtx ;                // mutex de escritura en pantalla
unsigned
    cont_prod[num_items] = {0}, // contadores de verificación: producidos
    cont_cons[num_items] = {0}; // contadores de verificación: consumidos

int 
    productora_siguiente_item[num_prod] = {0};

//**********************************************************************
// funciones comunes
//----------------------------------------------------------------------

int producir_dato( int id_hebra )
{
    this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
    
    const int valor_producido = (id_hebra * items_prod) + productora_siguiente_item[id_hebra];
    productora_siguiente_item[id_hebra]++ ; 

    mtx.lock();
    cout << "hebra productora " << id_hebra << ", produce " << valor_producido << endl << flush ;
    mtx.unlock();
    cont_prod[valor_producido]++ ;
    return valor_producido ;
}
//----------------------------------------------------------------------

// MODIFICADO: Acepta id_hebra_cons para trazabilidad
void consumir_dato( unsigned valor_consumir, int id_hebra_cons )
{
    if ( num_items <= valor_consumir )
    {
       cout << " valor a consumir === " << valor_consumir << ", num_items == " << num_items << endl ;
       assert( valor_consumir < num_items );
    }
    cont_cons[valor_consumir] ++ ;
    this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
    mtx.lock();
    // MODIFICADO: Mensaje de consumo mejorado
    cout << "                       hebra consumidora " << id_hebra_cons << ", consume: " << valor_consumir << endl ;
    mtx.unlock();
}
//----------------------------------------------------------------------

void test_contadores()
{
    bool ok = true ;
    cout << "comprobando contadores ...." << endl ;

    for( unsigned i = 0 ; i < num_items ; i++ )
    {
       if ( cont_prod[i] != 1 )
       {
          cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
          ok = false ;
       }
       if ( cont_cons[i] != 1 )
       {
          cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
          ok = false ;
       }
    }
    if (ok)
       cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// Clase para monitor buffer, version FIFO, semántica SC, multiples prod/cons

class ProdConsMu : public HoareMonitor
{
 private:
 static const int          
    num_celdas_total = 10; 
 int                       
    buffer[num_celdas_total], 
    primera_ocupada ,     
    primera_libre ,       
    num_celdas_ocupadas ; 

 CondVar                   
    ocupadas,                
    libres ;                 

 public:                   
    ProdConsMu() ;           
    int  leer();             
    void escribir( int valor ); 
} ;
// -----------------------------------------------------------------------------

ProdConsMu::ProdConsMu(  )
{
    primera_ocupada = 0 ;
    primera_libre = 0 ;
    num_celdas_ocupadas = 0 ; 
    ocupadas      = newCondVar();
    libres        = newCondVar();
}
// -----------------------------------------------------------------------------
int ProdConsMu::leer(  )
{
    if ( num_celdas_ocupadas == 0 )
       ocupadas.wait();

    assert( 0 < num_celdas_ocupadas  );

    const int valor = buffer[primera_ocupada] ;
    primera_ocupada = (primera_ocupada + 1) % num_celdas_total;
    num_celdas_ocupadas-- ;
    
    libres.signal();
    return valor ;
}
// -----------------------------------------------------------------------------
void ProdConsMu::escribir( int valor )
{
    if ( num_celdas_ocupadas == num_celdas_total )
       libres.wait();

    assert( num_celdas_ocupadas < num_celdas_total );

    buffer[primera_libre] = valor ;
    primera_libre = (primera_libre + 1) % num_celdas_total;
    num_celdas_ocupadas++ ;

    ocupadas.signal();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( MRef<ProdConsMu> monitor, int id_hebra )
{
    // Mensaje de inicio de hebra ***************************
    mtx.lock();
    cout << "Hebra productora " << id_hebra << " INICIADA." << endl;
    mtx.unlock();

    for( unsigned i = 0 ; i < items_prod ; i++ )
    {
       int valor = producir_dato( id_hebra ) ;
       monitor->escribir( valor );
    }
}
// -----------------------------------------------------------------------------

// MODIFICADO: Acepta id_hebra
void funcion_hebra_consumidora( MRef<ProdConsMu>  monitor, int id_hebra )
{
    // Mensaje de inicio de hebra ***************************
    mtx.lock();
    cout << "Hebra consumidora " << id_hebra << " INICIADA." << endl;
    mtx.unlock();

    for( unsigned i = 0 ; i < items_cons ; i++ )
    {
       int valor = monitor->leer();
       consumir_dato( valor, id_hebra ) ; // MODIFICADO: Pasa el id
    }
}
// -----------------------------------------------------------------------------

int main()
{
    cout << "--------------------------------------------------------------------" << endl
         << "Problema del productor-consumidor MÚLTIPLES (Monitor SU, buffer FIFO). " << endl
         << "--------------------------------------------------------------------" << endl
         << flush ;

    MRef<ProdConsMu> monitor = Create<ProdConsMu>() ;

    thread hebras_prod[num_prod];
    thread hebras_cons[num_cons];

    // Lanzar hebras productoras
    for(int i = 0; i < num_prod; i++)
        hebras_prod[i] = thread( funcion_hebra_productora, monitor, i );

    // Lanzar hebras consumidoras
    for(int i = 0; i < num_cons; i++)
        hebras_cons[i] = thread( funcion_hebra_consumidora, monitor, i ); // MODIFICADO: Pasa el id

    // Esperar a que terminen las hebras productoras
    for(int i = 0; i < num_prod; i++)
        hebras_prod[i].join();

    // Esperar a que terminen las hebras consumidoras
    for(int i = 0; i < num_cons; i++)
        hebras_cons[i].join();


    test_contadores() ;
}