#include <string>
#include <iostream> 
#include <thread>
#include <chrono>   
#include <ratio>    

using namespace std ;
using namespace std::chrono ;
using namespace std::this_thread ;

// Tipo para duraciones en milisegundos, en coma flotante, para mayor precisión en el informe [cite: 130, 131]
typedef duration<float,ratio<1,1000>> milliseconds_f ;

// -----------------------------------------------------------------------------
// Tarea genérica que simula carga de trabajo mediante una espera bloqueada [cite: 259]
void Tarea( const std::string & nombre, milliseconds tcomputo )
{
   cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... " ;
   sleep_for( tcomputo );
   cout << "fin." << endl ;
}

void TareaA() { Tarea( "A", milliseconds(100) );  }
void TareaB() { Tarea( "B", milliseconds( 80) );  }
void TareaC() { Tarea( "C", milliseconds( 50) );  }
void TareaD() { Tarea( "D", milliseconds( 40) );  }
void TareaE() { Tarea( "E", milliseconds( 20) );  }

// -----------------------------------------------------------------------------
int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario (250ms) [cite: 173, 224]
   const milliseconds Ts_ms( 250 );

   // ini_sec = instante de inicio esperado de la iteración actual [cite: 225]
   time_point<steady_clock> ini_sec = steady_clock::now();

   while( true ) 
   {
      cout << endl
           << "---------------------------------------" << endl
           << "Comienza iteración del ciclo principal." << endl ;

      for( int i = 1 ; i <= 4 ; i++ ) 
      {
         cout << endl << "Comienza iteración " << i << " del ciclo secundario." << endl ;

         switch( i )
         {
            case 1 : TareaA(); TareaB(); TareaC();           break ;
            case 2 : TareaA(); TareaB(); TareaD(); TareaE(); break ;
            case 3 : TareaA(); TareaB(); TareaC();           break ;
            case 4 : TareaA(); TareaB(); TareaD();           break ;
         }

         // 1. Calculamos el instante en el que DEBERÍA terminar este ciclo (instante esperado) 
         ini_sec += Ts_ms ;

         // 2. Esperamos hasta ese instante [cite: 251]
         sleep_until( ini_sec );

         // --- CAMBIOS PARA EJERCICIO EJECUTIVO1-COMPR.CPP ---

         // 3. Obtenemos el instante actual REAL después de despertar [cite: 114, 161]
         time_point<steady_clock> instante_actual = steady_clock::now();

         // 4. Calculamos el retraso: diferencia entre el tiempo real y el esperado [cite: 135, 262]
         // Usamos milliseconds_f para obtener la parte fraccionaria (decimales) [cite: 139]
         float retraso_ms = milliseconds_f( instante_actual - ini_sec ).count();

         // 5. Informamos del retraso obtenido 
         cout << ">> Informe de ciclo: retraso detectado de " << retraso_ms << " ms." << endl ;
         
         // ---------------------------------------------------
      }
   }
}