#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"
//#define LIFO //Para compilar el código lifo
using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 40 ,   // número de items
	tam_vec   = 10 ,   // tamaño del buffer
   n_p = 4, //Nº hebras productoras
   n_c = 4, //Nº hebras consumidoras
   p = num_items/n_p, //Cantidad que cada productor produce
   c = num_items/n_c; //Cantidad que cada consumidor consume

unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   item_producidos[n_p] = {0};

Semaphore
   libres = tam_vec,
   ocupadas = 0,
   modificando = 1;

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato(int n_hebra)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   int k = item_producidos[n_hebra]; //Cuantos items ha producido esta hebra
   const unsigned dato_producido = n_hebra*p+k ;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << flush ;
   item_producidos[n_hebra]++;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato , int n_hebra)
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

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

#ifdef LIFO

int
   primera_libre = 0,
   vec[tam_vec] = {0};

void  funcion_hebra_productora( int n_hebra )
{
   for( unsigned i = 0 ; i < p ; i++ )
   {
      int dato = producir_dato(n_hebra) ;
      sem_wait(libres);
      sem_wait(modificando); //Esperamos a ver si lo están tocando
      vec[primera_libre] = dato; //Aniadimos a la lista
      primera_libre++; //Aumenta pues ahora leemos el siguiente
      sem_signal(modificando); //Sumamos 1 para que otro entre
      sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora( int n_hebra )
{
   for( unsigned i = 0 ; i < c ; i++ )
   {
      int dato ;
      sem_wait(ocupadas);
      sem_wait(modificando);
      primera_libre--;
      dato = vec[primera_libre];
      sem_signal(modificando);
      sem_signal(libres);
      consumir_dato( dato , n_hebra) ;
    }
}
//----------------------------------------------------------------------

#else //Si no es LIFO usamos FIFO

int
   primera_libre = 0,
   primera_ocupada = 0,
   vec[tam_vec] = {0};

void  funcion_hebra_productora( int n_hebra )
{
   for( unsigned i = 0 ; i < p ; i++ )
   {
      int dato = producir_dato(n_hebra) ;
      sem_wait(libres);
      sem_wait(modificando);
      vec[primera_libre] = dato; //Aniadimos a la lista
      primera_libre = (primera_libre + 1) % tam_vec; //Aumenta pues ahora leemos el siguiente
      sem_signal(modificando);
      sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora( int n_hebra )
{
   for( unsigned i = 0 ; i < c ; i++ )
   {
      int dato ;
      sem_wait(ocupadas);
      sem_wait(modificando);
      dato = vec[primera_ocupada];   
      primera_ocupada = (primera_ocupada + 1) % tam_vec; 
      sem_signal(modificando);
      sem_signal(libres);
      consumir_dato( dato , n_hebra) ;
    }
}
//----------------------------------------------------------------------
#endif
int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO o FIFO ?)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebras_consumidoras[n_c];
   thread hebras_productoras[n_p];

   for(int i = 0; i < n_p; i++)
      hebras_productoras[i] = thread(funcion_hebra_productora, i);
   
   for(int j = 0; j < n_c; j++)
      hebras_consumidoras[j] = thread(funcion_hebra_consumidora, j);
   
   for(int i = 0; i < n_p; i++)
      hebras_productoras[i].join();
   
   for(int j = 0; j < n_c; j++)
      hebras_consumidoras[j].join();

   test_contadores();
}
