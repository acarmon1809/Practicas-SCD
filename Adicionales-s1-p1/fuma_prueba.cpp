/*
Posible solución fuma_prueba.cpp:
Cada fumador fuma hasta 4 veces, mostrando cuántas fumadas lleva. 
Cuando uno alcanza las 4 fumadas, se muestra un mensaje final y 
el programa termina: los demás fumadores y el estanquero dejan de ejecutar.
*/

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <vector>
#include "scd.h"

using namespace std;
using namespace scd;

constexpr int             num_fumadores = 3;         // número de fumadores
Semaphore                 mostrador_libre = 1;       // 1 si el mostrador está libre, 0 si no
mutex                     mutex_cout;                // mutex para que no se mezclen las salidas en pantalla
std::vector<Semaphore>    ingrediente_en_mostrador ; // vector de semáforos de fumadores
// vale: 1 si el i-ésimo fumador tiene su ingrediente disponible en el mostrador, 0 si no.

//--- nuevas variables globales ---
constexpr int             max_fumadas = 4;           // número máximo de fumadas
bool                      terminar = false;          // control global de finalización
Semaphore                 sem_fin = 0;               // liberado por el primer fumador que llega a 4
Semaphore                 sem_terminar = 0;          // aviso global de terminación para todos los fumadores y el estanquero

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)    
   chrono::milliseconds duracion_produ(aleatorio<10,100>());
   
   // informa de que comienza a producir
   mutex_cout.lock();
	 cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;
   mutex_cout.unlock();
   
   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos	
   this_thread::sleep_for(duracion_produ);

   const int num_ingrediente = aleatorio<0,num_fumadores-1>();

   // informa de que ha terminado de producir
    mutex_cout.lock();  
      cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;
    mutex_cout.unlock();
	return num_ingrediente;
}

//-------------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero()
{
    while (!terminar)
    {
        // esperar hasta que el mostrador esté libre
        mostrador_libre.sem_wait();
		
        // chequeo inmediato de finalización
		if (terminar) 
		{
			mostrador_libre.sem_signal(); // liberar mostrador para no bloquear
            break;
		}	
		
        // selecciona aleatoriamente un fumador
	    int num_fumador = producir_ingrediente();
        
		// informa del ingrediente que ha puesto
		mutex_cout.lock();
          cout << "Estanquero : pone en mostrador ingrediente para fumador " << num_fumador << "." << endl;
        mutex_cout.unlock();
		
        // señala que está disponible el ingrediente para el fumador número 'num_fumador'
        ingrediente_en_mostrador[num_fumador].sem_signal();			
	}
//---Mensaje de finalización---
mutex_cout.lock();
cout << "Estanquero : termina su ejecución." << endl;
mutex_cout.unlock();
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar(int num_fumador)
{
    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_fumar(aleatorio<20,200>());

    // informa de que comienza a fumar
    mutex_cout.lock();
      cout << "Fumador " << num_fumador << " : empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
    mutex_cout.unlock();
    
	// espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
    this_thread::sleep_for(duracion_fumar);
 
    // informa de que ha terminado de fumar
    mutex_cout.lock();
      cout << "Fumador " << num_fumador << " : termina de fumar, comienza espera de ingrediente." << endl;
    mutex_cout.unlock();
}

//-------------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void funcion_hebra_fumador(int num_fumador)
{
    int contador = 0;

    while (!terminar) 
    {
        ingrediente_en_mostrador[num_fumador].sem_wait();
		      	
		if (terminar) break; // chequeo de seguridad después de desbloquear
    
     	mostrador_libre.sem_signal(); //liberar el mostrador
		
		if (terminar) break; // chequeo adicional si otro fumador alcanzó max_fumadas, no fumar

		fumar(num_fumador);

        contador++;
        mutex_cout.lock();
          cout << "Fumador " << num_fumador << " ha fumado: " << contador << " veces." << endl;
        mutex_cout.unlock();

        // Primer fumador que alcanza max_fumadas avisa a main y detiene todo
        if (contador == max_fumadas) 
		{	
		    terminar = true;       // detener globalmente
			sem_fin.sem_signal();  // notifica a main
			break;                 // termina el fumador con max_fumadas 
		}
	}	
//---Mensaje de finalización---
mutex_cout.lock();
cout << "Fumador " << num_fumador << " termina su ejecución." << endl;
mutex_cout.unlock();
}

//-------------------------------------------------------------------------

int main()
{
    for (int i = 0; i < num_fumadores; i++)
        ingrediente_en_mostrador.push_back(Semaphore(0));
	
    thread hebra_estanquero(funcion_hebra_estanquero),
           hebra_fumador[num_fumadores];

    for (unsigned long i = 0; i < num_fumadores; i++)
        hebra_fumador[i] = thread(funcion_hebra_fumador, i);

    // Esperar a que un fumador llegue a max_fumadas
    sem_fin.sem_wait();
	
    // Avisar a todos los fumadores y al estanquero que deben terminar
    terminar = true;
	
    // Desbloquear posibles bloqueos en estanquero y fumadores
    mostrador_libre.sem_signal();
    for (int i = 0; i < num_fumadores; i++)
        ingrediente_en_mostrador[i].sem_signal();
		
	// Esperar que terminen todas las hebras
	hebra_estanquero.join();
	for (int i = 0; i < num_fumadores; i++)
        hebra_fumador[i].join();
 
    cout << "\n=== Fin del ejercicio ===" << endl;
}
