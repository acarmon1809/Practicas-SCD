#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

const int   num_lectores = 4,
            num_escritores = 2;

mutex mtx;

class Lec_Esc : public HoareMonitor{

    private:
        int n_lec; //Numero de lectores leyendo
        bool escrib; //True si hay algún escritor escribiendo
        CondVar lectura, escritura;
    
    public:
        Lec_Esc();
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

void funcion_hebra_escritor( MRef<Lec_Esc> monitor, int numero_escritor){
    while(true){
        monitor->init_escritura();
        
        
        // informa de que comienza a producir
        mtx.lock();
        cout << "Escritor " << numero_escritor << " comienza a escribir" << endl;
        mtx.unlock();
        // espera bloqueada un tiempo igual a ''duracion' milisegundos
        this_thread::sleep_for(chrono::milliseconds(aleatorio<2000,20000>()));
        mtx.lock();
        cout << "Escritor " << numero_escritor << " termina de escribir" << endl;
        mtx.unlock();
        monitor->fin_escritura();
    }
}

void funcion_hebra_lector( MRef<Lec_Esc> monitor, int numero_lector){
    while(true){

        monitor->init_lectura();

        // Se quedan mas tiempo leyendo
        chrono::milliseconds duracion( aleatorio<10,1000>() );
        // informa de que comienza a producir
        mtx.lock();
        cout << "Lector " << numero_lector << " comienza a leer" << endl;
        mtx.unlock();
        this_thread::sleep_for(chrono::milliseconds(aleatorio<100,1000>()));
        mtx.lock();
        cout << "Lector " << numero_lector << " termina de leer" << endl;   
        mtx.unlock();
        monitor->fin_lectura();
    }
}

int main()
{
   thread escritores[num_escritores], lectores[num_lectores];
   MRef<Lec_Esc> monitor = Create<Lec_Esc>();

   for(int i = 0; i < num_escritores; i++)
      escritores[i] = thread(funcion_hebra_escritor, monitor, i);

    for(int i = 0; i < num_lectores; i++)
      lectores[i] = thread(funcion_hebra_lector, monitor, i);



   for(int i = 0; i < num_escritores; i++)
      escritores[i].join();

    for(int i = 0; i < num_lectores; i++)
        lectores[i].join();
    
   
}
