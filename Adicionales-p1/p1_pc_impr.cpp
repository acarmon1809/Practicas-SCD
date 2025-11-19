#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"
using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 20 ,   // número de items
	tam_vec   = 10 ;   // tamaño del buffer
   
   unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ,  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)
   celdas_ocupadas = 0; //Elemenetos Aniadidos al vector pero no consumidos todavía
   
mutex
   mtx;

Semaphore
   libres = tam_vec,
   ocupadas = 0, //Numero entre 0 y tam_vec
   exclusion_mutua = 1,
   impresora = 0, //Tienen que desbloquearla primero
   productor = 0; //Se bloquea mientras se imprime

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   mtx.lock();
   cout << "producido: " << dato_producido << endl << flush ;
   mtx.unlock();
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx.unlock();
}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

int
   primera_libre = 0,
   primera_ocupada = 0,
   vec[tam_vec] = {0};

void  funcion_hebra_productora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;
      sem_wait(libres);
      exclusion_mutua.sem_wait();
      vec[primera_libre] = dato; //Aniadimos a la lista
      celdas_ocupadas++;
      primera_libre = (primera_libre + 1) % tam_vec; //Aumenta pues ahora leemos el siguiente
      if(dato%5 == 0){
         sem_signal(impresora);
         sem_wait(productor);
      }
      exclusion_mutua.sem_signal();
      sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;
      sem_wait(ocupadas);
      exclusion_mutua.sem_wait();
      dato = vec[primera_ocupada];   
      primera_ocupada = (primera_ocupada + 1) % tam_vec; 
      celdas_ocupadas--;
      exclusion_mutua.sem_signal(); 
      sem_signal(libres);

      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

void funcion_hebra_impresora(){
   
   for(int i = 0; i < num_items/5; i++) {

      sem_wait(impresora);
      mtx.lock();
      cout << "Celdas ocupadas: " << celdas_ocupadas << endl;
      mtx.unlock();
      sem_signal(productor);
   }
}

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO o FIFO ?)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora ),
          impresora( funcion_hebra_impresora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;
   impresora.join();

   test_contadores();
}
