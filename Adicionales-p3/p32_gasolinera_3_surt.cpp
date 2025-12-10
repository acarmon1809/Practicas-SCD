#include <iostream>
#include <random>  // para 'aleatorio'
#include <thread>  // this_thread::sleep_for
#include <chrono>  // chrono::duration, chrono::milliseconds
#include <cstring>  // strlen
#include <mpi.h>

using namespace std ;
using namespace this_thread ;  // sleep_for
using namespace chrono ;       // milliseconds, seconds, etc

const int 
    num_tipos = 3,
    num_coches = 10,
    etiq_empezar[num_tipos] = {0,1,2}, //3 representa salir de repostar
    etiq_terminar = num_tipos+1,
    id_gasolinera = 0,
    num_surtidores[num_tipos] = {1,2,1}, //El numero de surtidores debe ser menor que el de coches.
    num_procesos = num_coches+1, //Debe ser mayor que 1 para que haya coches y gasolinera
    tiempo_min_repostar_ms = 20,
    tiempo_max_repostar_ms = 200,
    tiempo_min_fuera_ms    = 20,
    tiempo_max_fuera_ms    = 200 ;; //Por poner algo

//-----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// function template for generating random numbers (between two integer 
// compile-time constants)

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void funcion_coche(int id){

    int tipo = aleatorio<0,2>();

    while (true){
        //Solicita repostar
        MPI_Ssend(&tipo, 1, MPI_INT, id_gasolinera, etiq_empezar[tipo], MPI_COMM_WORLD);
        //Espera un tiempo
        this_thread::sleep_for(chrono::milliseconds( aleatorio<tiempo_min_repostar_ms,tiempo_max_repostar_ms>() ) );
        
        //Avisa que deja de repostar
        MPI_Ssend(&tipo, 1, MPI_INT, id_gasolinera, etiq_terminar, MPI_COMM_WORLD);
    }

}
//-----------------------------------------------------------------------------
void funcion_gasolinera(int id){
    int en_uso[num_tipos] = {0,0,0}; //Contador
    int etiq_aceptable = MPI_ANY_TAG;
    MPI_Status estado;
    int tipo_recibido;
    int hay_msg;

    while(true){
        hay_msg = false; //Para ver si hay delay o no entre la proxima vuelta

        //Vemos si alguien quiere dejar de repostar
        MPI_Iprobe( MPI_ANY_SOURCE, etiq_terminar , MPI_COMM_WORLD, &hay_msg, &estado );
        
        if(hay_msg){
            MPI_Recv(&tipo_recibido, 1, MPI_INT, estado.MPI_SOURCE, etiq_terminar, MPI_COMM_WORLD, &estado);
            en_uso[tipo_recibido]--;
            cout << "El coche " << estado.MPI_SOURCE << " del tipo " << tipo_recibido << " dejo de repostar " << endl;
            cout << "Surtidores actuales en uso: 0: " << en_uso[0] << " || 1: " << en_uso[1] << " || 2: " << en_uso[2] << endl << endl;
        }

        //Revisamos si se pueden meter de un tipo y en ese caso revisamos mensajes
        for(int i = 0; i < num_tipos; i++){
            if(en_uso[i] < num_surtidores[i]){ //Si quedan libres
                MPI_Iprobe( MPI_ANY_SOURCE, etiq_empezar[i], MPI_COMM_WORLD, &hay_msg, &estado);
                if(hay_msg){
                    MPI_Recv(&tipo_recibido, 1, MPI_INT, estado.MPI_SOURCE, etiq_empezar[i], MPI_COMM_WORLD, &estado);
                    en_uso[tipo_recibido]++;
                    cout << "El coche " << estado.MPI_SOURCE << " del tipo " << tipo_recibido << " empezo a repostar " << endl;
                    cout << "Surtidores actuales en uso: 0: " << en_uso[0] << " || 1: " << en_uso[1] << " || 2: " << en_uso[2] << endl << endl;
                }
            } 

        }
        //Si no se recibió nada esperamos
        if(!hay_msg) this_thread::sleep_for(chrono::milliseconds(20));
    }
}
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int id_propio, num_procesos_actual ;
    int num_coches[3];

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
    MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

    if(num_procesos_actual == num_procesos){
                
        if(id_propio != id_gasolinera)
            funcion_coche(id_propio);
        else funcion_gasolinera(id_propio);

    }   
    else
    {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
    }

    MPI_Finalize();
    return 0;

}