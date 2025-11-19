#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

const int A = 4; //Hebras de gasoil
const int B = 6; //Hebras de gasolina   //La suma de A y B debe ser 10 por el enunciado

class gasolinera : public HoareMonitor{

    private:
        const int 
            num_sur_gasolina = 3,
            num_sur_gasoil = 2;
        
        int 
            gasolina_usando,
            gasoil_usando,
            total_usando;

        CondVar
            gasolina_libre,
            gasoil_libre;
    
    public:
        gasolinera();
        void entra_coche_gasoil(int matricula);
        void sale_coche_gasoil(int matricula);
        void entra_coche_gasolina(int matricula);
        void sale_coche_gasolina(int matricula);

};

gasolinera::gasolinera(){
    gasolina_usando = 0;
    gasoil_usando = 0;
    total_usando = 0;
    gasolina_libre = newCondVar();
    gasoil_libre = newCondVar();
}

void gasolinera::entra_coche_gasolina(int matricula){

    if(gasolina_usando == num_sur_gasolina) //Si están usando todos te paras
        gasolina_libre.wait();

    gasolina_usando++;
    total_usando = gasolina_usando + gasoil_usando;

    cout << "El coche de gasolina " << matricula << " ha entrado " << endl;
    cout << "Surtidores en uso: " << total_usando << endl << endl;

}

void gasolinera::sale_coche_gasolina(int matricula){
    
    gasolina_usando--;
    total_usando = gasolina_usando + gasoil_usando;

    cout << "El coche de gasolina " << matricula << " ha salido" << endl;
    cout << "Surtidores en uso: " << total_usando << endl << endl;

    if(!gasolina_libre.empty()) gasolina_libre.signal();

}

void gasolinera::entra_coche_gasoil(int matricula){

    if(gasoil_usando == num_sur_gasoil) //Si están usando todos te paras
        gasoil_libre.wait();

    gasoil_usando++;
    total_usando = gasoil_usando + gasolina_usando;

    cout << "El coche de gasoil " << matricula << " ha entrado " << endl;
    cout << "Surtidores en uso: " << total_usando << endl << endl;


}

void gasolinera::sale_coche_gasoil(int matricula){
    
    gasoil_usando--;
    total_usando = gasolina_usando + gasoil_usando;
    
    cout << "El coche de gasoil " << matricula << " ha salido" << endl;
    cout << "Surtidores en uso: " << total_usando << endl << endl;

    if(!gasoil_libre.empty()) gasoil_libre.signal();
}



void funcion_hebra_gasolina(MRef<gasolinera> monitor, int matricula){
    while(true){
        monitor->entra_coche_gasolina(matricula);
        this_thread::sleep_for(chrono::milliseconds(aleatorio<1000,2000>()));
        monitor->sale_coche_gasolina(matricula);
        this_thread::sleep_for(chrono::milliseconds(aleatorio<1000,2000>()));
    }
}

void funcion_hebra_gasoil(MRef<gasolinera> monitor, int matricula){
    while(true){
        monitor->entra_coche_gasoil(matricula);
        this_thread::sleep_for(chrono::milliseconds(aleatorio<1000,2000>()));
        monitor->sale_coche_gasoil(matricula);
        this_thread::sleep_for(chrono::milliseconds(aleatorio<1000,2000>()));
    }
}


int main(){

    MRef<gasolinera> monitor = Create<gasolinera>();

    thread gasoil[A], gasolina[B];

    for(int i = 0; i < A; i++)
        gasoil[i] = thread(funcion_hebra_gasoil, monitor ,i);

    for(int i = 0; i < B; i++)
        gasolina[i] = thread(funcion_hebra_gasolina, monitor, i);
    
    for(int i = 0; i < A; i++)
        gasoil[i].join();

    for(int i = 0; i < B; i++)
        gasolina[i].join();
}