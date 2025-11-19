#include <iostream>
#include <iomanip>
#include <cassert>
#include <chrono>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

const int A = 4; //Hebras de diesel
const int B = 6; //Hebras de gasolina   //La suma de A y B debe ser 10 por el enunciado

const int C = 2; //Surtidores de diesel < A
const int D = 3; //Surtidores de gasolina < B

int surtidores_en_uso = 0;

Semaphore surtidor_libre_diesel = C;
Semaphore surtidor_libre_gasolina = D;

Semaphore seccion_critica = 1;
Semaphore mtx = 1;

void repostar(int matricula, string tipo){
    //sem_wait(mtx); //este ya no es necesario porque se pone justo antes de repostar
    cout << "Coche numero " << matricula << " de " << tipo << " comienza a repostar"<< endl;
    sem_signal(mtx); //Terminamos la impresion
    this_thread::sleep_for(chrono::milliseconds(aleatorio<1000,2000>()));
    sem_wait(mtx); //controlamos la salida por pantalla
    cout << "Coche numero " << matricula << " de " << tipo << " termina de repostar"<< endl<< endl;
    //sem_signal(mtx); //Este ya no es necesario porque se imprime justo despues de resportar
}

void funcion_hebra_diesel(int matricula){
    while(true){
        sem_wait(surtidor_libre_diesel);
        seccion_critica.sem_wait(); surtidores_en_uso++; seccion_critica.sem_signal();
        sem_wait(mtx); //controlamos la salida por pantalla
        cout << "El numero de surtidores en uso ahora es " << surtidores_en_uso << endl; 
        repostar(matricula, "diesel");
        seccion_critica.sem_wait(); surtidores_en_uso--; seccion_critica.sem_signal();
        cout << "El numero de surtidores en uso ahora es " << surtidores_en_uso << endl; 
        sem_signal(mtx); //Terminamos la impresion  
        sem_signal(surtidor_libre_diesel);
    }
}

void funcion_hebra_gasolina(int matricula){
    while(true){
        sem_wait(surtidor_libre_gasolina);

        seccion_critica.sem_wait(); surtidores_en_uso++; seccion_critica.sem_signal();
        sem_wait(mtx); //controlamos la salida por pantalla
        cout << "El numero de surtidores en uso ahora es " << surtidores_en_uso << endl; 
        repostar(matricula, "gasolina");
        seccion_critica.sem_wait(); surtidores_en_uso--; seccion_critica.sem_signal();
        cout << "El numero de surtidores en uso ahora es " << surtidores_en_uso << endl; 
        sem_signal(mtx); //Terminamos la impresion  
        sem_signal(surtidor_libre_gasolina);
    }
}

int main(){


    thread diesel[A], gasolina[B];

    for(int i = 0; i < A; i++)
        diesel[i] = thread(funcion_hebra_diesel,i);

    for(int i = 0; i < B; i++)
        gasolina[i] = thread(funcion_hebra_gasolina, i);
    
    for(int i = 0; i < A; i++)
        diesel[i].join();

    for(int i = 0; i < B; i++)
        gasolina[i].join();
    

}