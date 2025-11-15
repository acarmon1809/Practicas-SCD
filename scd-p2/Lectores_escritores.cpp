#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

const int   num_lectores = 2,
            num_escritores = 1;

class Lec_Esc : public HoareMonitor{

    private:
        int n_lec; //Numero de lectores leyendo
        bool escrib; //True si hay algún escritor escribiendo
        CondVar lectura, escritura;
    
    public:
        void init_lectura(); //Para lectores
        void fin_lectura(); //Para lectores

        void init_escritura(); //Para escritores
        void fin_escritura();   //Para escritores
};

Lec_Esc::Lec_Esc(){
    lectura = newCondVar();
    escritura = newCondVar();
    escrib = false;
    n_lec = 0;
}

void Lec_Esc::init_escritura(){
    
    if(n_lec > 0 || escrib) //Si hay lectores o escritores te esperas
        escritura.wait();
    
    escrib = true; //Hay alguien escribiendo
    
}

void Lec_Esc::fin_escritura(){

    escrib = false; //Ya no estamos escribiendo

    if(!lectura.empty())
        lectura.signal(); //Si hay lectores esperando le mandamos señal (PREFERENCIA)
    else 
        escritura.signal();
}

void Lec_Esc::init_lectura(){
    
    if(escrib) //Si hay escritores te bloqueas
        lectura.wait();
    
    n_lec = n_lec + 1; //Hay un lector más 
    lectura.signal(); //Otros pueden leer conmigo

}

void Lec_Esc::fin_lectura(){
    
    n_lec = n_lec - 1; //Uno menos leyendo

    if(n_lec == 0)
        escritura.signal(); //Si nadie lee puede escribir alguien
    
}

void funcion_hebra_escritor( MRef<Lec_Esc> monitor){
    while(true){
        monitor->init_escritura();
        //COdigo de lectura
        monitor->fin_escritura();
        //resto codigo
    }
}

void funcion_hebra_lector( MRef<Lec_Esc> monitor){
    while(true){
        monitor->init_lectura();
        //Codigo de escritura
        monitor->fin_lectura();
        //resto codigo
    }
}

int main()
{
   thread escritores[num_escritores], lectores[num_lectores];
   MRef<Lec_Esc> monitor = Create<Lec_Esc>();

   for(int i = 0; i < num_escritores; i++)
      escritores[i] = thread(funcion_hebra_escritor, monitor);

    for(int i = 0; i < num_lectores; i++)
      lectores[i] = thread(funcion_hebra_lector, monitor);



   for(int i = 0; i < num_escritores; i++)
      escritores[i].join();

    for(int i = 0; i < num_lectores; i++)
        lectores[i].join();
    
   
}
