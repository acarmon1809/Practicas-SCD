#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// numero de fumadores 

const int num_fumadores = 3 ;

Semaphore 
   producido[num_fumadores] = {0,0,0}, //No se ha producido ninguno
   mostrador = 1; //Permitimos que de primeras se coloque algo
   

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   while(true){

      int ingrediente = producir_ingrediente();
      sem_wait(mostrador); //Semaforo que espera a que cojan de la mesa el ingrediente
      cout << "Estanquero produce el ingrendiente " << ingrediente << endl;
      sem_signal(producido[ingrediente]); //Desbloqueamos el ingrediente

   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
      
      sem_wait(producido[num_fumador]); //Esperamos a que se haya producido
      cout << "El fumador " << num_fumador << " retira su ingrediente" << endl;  //Nos toca fumar
      sem_signal(mostrador); //Indicamos que ya puede poner otra cosa
      fumar(num_fumador); //Varios pueden fumar a la vez

   }
}

//----------------------------------------------------------------------

int main()
{
   thread fumadores[num_fumadores], estanquero;

   for(int i = 0; i < num_fumadores; i++)
      fumadores[i] = thread(funcion_hebra_fumador, i);

   estanquero = thread(funcion_hebra_estanquero);

   for(int i = 0; i < num_fumadores; i++)
      fumadores[i].join();
   
   estanquero.join();
   
}
