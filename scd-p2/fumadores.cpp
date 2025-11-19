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
mutex mtx;

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   mtx.lock();
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;
   mtx.unlock();
   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   mtx.lock();
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;
   mtx.unlock();
   return num_ingrediente ;
}



//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
   mtx.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
   mtx.unlock();
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
   mtx.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
   mtx.unlock();
}

class Estanco : public HoareMonitor
{

   
   private:

      int ingredientes_en_mesa = -1;

      CondVar 
         producido[num_fumadores],
         mostrador;

   public:
      Estanco();
      void obtenerIngrediente(int num_fumador);
      void ponerIngrediente(int ingrediente);
      void esperarRecogidaIngrediente();

};

Estanco::Estanco(){
   
   for(int i = 0; i < num_fumadores; i++)
      producido[i] = newCondVar();

   mostrador = newCondVar();
}

void Estanco::obtenerIngrediente(int num_fumador){
   //Cuando se solicita un ingrediente este debe haberse producido
   if(ingredientes_en_mesa != num_fumador)
      producido[num_fumador].wait(); //Si no hay nada en la mesa esperamos 
   
   ingredientes_en_mesa = -1; //Ahora no hay nada
   mostrador.signal(); //Avisamos de que ya puede seguir trabajando
}

void Estanco::ponerIngrediente(int ingrediente){
   
   ingredientes_en_mesa = ingrediente;
   producido[ingrediente].signal(); //Ya lo puede coger el que esperaba

}

void Estanco::esperarRecogidaIngrediente(){
   //Tenemos que ver que no hay nada en la mesa
   if(ingredientes_en_mesa >= 0)
      mostrador.wait(); //Esperamos a que nos avisen de que lo quitan
   
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> monitor )
{
   while(true){

      int ingrediente = producir_ingrediente();
      mtx.lock();
      cout << "Estanquero produce el ingrendiente " << ingrediente << endl;
      mtx.unlock();
      monitor->ponerIngrediente(ingrediente);
      monitor->esperarRecogidaIngrediente();

   }
}


//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador(MRef<Estanco> monitor, int num_fumador )
{
   while( true )
   {
      
      monitor->obtenerIngrediente(num_fumador);
      mtx.lock();
      cout << "El fumador " << num_fumador << " retira su ingrediente" << endl; 
      mtx.unlock();
      fumar(num_fumador); //Varios pueden fumar a la vez

   }
}

//----------------------------------------------------------------------

int main()
{
   thread fumadores[num_fumadores], estanquero;
   MRef<Estanco> monitor = Create<Estanco>();

   for(int i = 0; i < num_fumadores; i++)
      fumadores[i] = thread(funcion_hebra_fumador, monitor, i);

   estanquero = thread(funcion_hebra_estanquero, monitor);

   for(int i = 0; i < num_fumadores; i++)
      fumadores[i].join();
   
   estanquero.join();
   
}
