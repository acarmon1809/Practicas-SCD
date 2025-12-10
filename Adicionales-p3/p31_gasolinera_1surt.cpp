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
    etiq_fin_repostar = 100,
    etiq_empiza_repostar = 101,
    id_gasolinera = 0,
    num_surtidores = 4,
    num_coches = 10,
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
    
    int valor;

    while (true){
        //Solicita repostar
        MPI_Ssend(&valor, 1, MPI_INT, id_gasolinera, etiq_empiza_repostar, MPI_COMM_WORLD);
        //Espera un tiempo
        this_thread::sleep_for(chrono::milliseconds( aleatorio<tiempo_min_repostar_ms,tiempo_max_repostar_ms>() ) );
        
        //Avisa que deja de repostar
        MPI_Ssend(&valor, 1, MPI_INT, id_gasolinera, etiq_fin_repostar, MPI_COMM_WORLD);
    }

}
//-----------------------------------------------------------------------------
void funcion_gasolinera(int id){
    int en_uso = 0; //Contador
    int etiq_aceptable = MPI_ANY_TAG;
    MPI_Status estado;
    int valor;

    while(true){
        if(en_uso < num_surtidores){
            etiq_aceptable = MPI_ANY_TAG;
        }
        else etiq_aceptable = etiq_fin_repostar;

        //Esperamos un mensaje para dar permiso
        MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado);

        switch (estado.MPI_TAG)
        {
            case etiq_fin_repostar:
                en_uso--;
                cout << "El coche " << estado.MPI_SOURCE << " deja de repostar" << endl;
                cout << "Se estan usando " << en_uso << " surtidores" << endl;
                break;
                
                case etiq_empiza_repostar:
                en_uso++;
                cout << "El coche " << estado.MPI_SOURCE << " empieza a repostar" << endl;
                cout << "Se estan usando " << en_uso << " surtidores" << endl;
                break;
        }
    }
}
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int id_propio, num_procesos_actual ;

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