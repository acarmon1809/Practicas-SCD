//Marco Zafra Rodríguez Dni: 54830625M
/**
 * @file pregunta1.cpp
 * @author Carlos Ureña (curena@ugr.es)
 * @version 0.1
 * @date 2022-11-03
 * @copyright Copyright (c) 2022
 * 
 * @brief
 * Implementar un programa con semáforos (en un archivo llamado pregunta1.cpp), con los siguientes requerimientos .... (ver pdf) .....
 * 
 */

 #include <iostream>
 #include <thread> 
 #include <cmath>
 #include "scd.h" // incluye tipo 'Semaphore', entre otros
 
 using namespace std ; // permite acortar la notación ('abc' en lugar de 'std::abc')
 using namespace scd ; // permite usar 'Semaphore' en lugar de 'scd::Semaphore'
 
 
 // constantes y variables globales (compartidas)
 constexpr unsigned 
    num_vegetariano = 3, // clientes vegetarianos
    num_vegano = 3,  //clientes veganos
    num_celiaco = 2,  //clientes celiacos
    num_bandejas_vegetariano = 3, // numero total de bandejas para vegetarianos
    num_bandejas_vegano = 2, // número total de bandejas para veganos
    num_bandejas_celiaco = 1; // numero total de bandejas para celiacos

 constexpr unsigned 
    tiempo_min_comer_ms = 20,
    tiempo_max_comer_ms = 200,
    tiempo_min_fuera_ms    = 20,
    tiempo_max_fuera_ms    = 200 ;
 
 unsigned 
    num_bandejas_en_uso = 0 ,  // número de bandejas en uso
    ocupadas[3];                //numero de bandejas ocupadas, vegetarianos indice 0, veganos indice 1 y celiacos indice 2
 
 Semaphore 
    bandejas_vegetarianas_libres = num_bandejas_vegetariano, // semáforo cuyo valor es el número de bandejas vegetarianas libres
    bandejas_veganas_libres = num_bandejas_vegano, // idem , de veganas
    bandejas_celiaco_libres = num_bandejas_celiaco, // idem, de celiacas
    mutex_num_bandejas   = 1,             // semáforo para acceder o imprimir el número de bandejas en uso      
    mutex_cout       = 1 ;            // semáforo para exclusión mutua en los 'cout' (al imprimir)
 
 /** -------------------------------------------------------------------------- **/
 /// @brief función que simula un retardo aleatorio al comer
 /// @param nc número de orden del comensal dentro de su tipo
 /// 
 void comer(const unsigned nc )
 {
    this_thread::sleep_for(chrono::milliseconds( aleatorio<tiempo_min_comer_ms,tiempo_max_comer_ms>() ) );
 }
 
 /** -------------------------------------------------------------------------- **/
 /// @brief función que ejecutan las hebras de comensales de vegetarianos
 /// @param ncg - número de orden del comensal vegetariano
 /// 
 void vegetariano (unsigned ncv )
 {
    while( true )
    {
       
       sem_wait( bandejas_vegetarianas_libres ); // esperar a que quede una bandeja vegetariana libre

            //entra a comer

          // incrementar e imprimir el valor de 'num_bandejas_en_uso' y bandejas vegetarianas
          sem_wait( mutex_num_bandejas ); // espera a que ninguna hebra esté leyendo o escribiendo 'num_bandejas_en_uso'
             num_bandejas_en_uso ++ ;
             ocupadas[0]++; // Incrementamos las bandejas vegetarianas usadas
             sem_wait( mutex_cout );
                cout << "Cliente VEGETARIANO " << ncv << " toma bandeja, Bandejas ocupadas: " << num_bandejas_en_uso << ", Vegetariano: " << ocupadas[0] 
                     << ", Vegano: " << ocupadas[1] << ", Celíaco: " << ocupadas[2]  << endl ;
             sem_signal( mutex_cout );
          sem_signal( mutex_num_bandejas ); // señalar que ninguna hebra está ya leyendo o escribiendo 'num_bandejas_en_uso'
 
          // comer 
          comer( ncv );
 
          // decrementar e imprimir el valor de 'num_bandejas_en_uso'
          sem_wait( mutex_num_bandejas ); // espera a que ninguna hebra esté leyendo o escribiendo 'num_bandejas_en_uso'
             num_bandejas_en_uso -- ;
             ocupadas[0]--;             // Decrementamos las bandejas vegetarianas usadas
             sem_wait( mutex_cout );
             cout << "Cliente VEGETARIANO " << ncv << " devuelve bandeja,Bandejas ocupadas " << num_bandejas_en_uso << "Vegetariano " << ocupadas[0] 
             << ", Vegano" << ocupadas[1] << ",Celíaco " << ocupadas[2]  << endl ;

             sem_signal( mutex_cout );
          sem_signal( mutex_num_bandejas ); // señalar que ninguna hebra está ya leyendo o escribiendo 'num_bandejas_en_uso'

            //devuelve bandeja
       sem_signal( bandejas_vegetarianas_libres ); // señalar que queda una bandeja vegetariana libre
 
       this_thread::sleep_for(chrono::milliseconds( aleatorio<tiempo_min_fuera_ms,tiempo_max_fuera_ms>() ) );
    }
 }
 
 /** -------------------------------------------------------------------------- **/
 /// @brief función que ejecutan las hebras de bandejas veganas
 /// @param ncvg - número de orden del cliente vegetariano
 /// 
 void vegano( unsigned ncvg )
 {
    while( true )
    {
       
       sem_wait( bandejas_veganas_libres ); // esperar a que quede una bandeja vegana libre
            //entra a comer

          // incrementar e imprimir el valor de 'num_bandejas_en_uso' y bandejas veganas
          sem_wait( mutex_num_bandejas ); // espera a que ninguna hebra esté leyendo o escribiendo 'num_bandejas_en_uso'
             num_bandejas_en_uso ++ ;
             ocupadas[1]++;                 // Incrementamos las bandejas veganas usadas
             sem_wait( mutex_cout );
             cout << "Cliente VEGANO " << ncvg << " toma bandeja,Bandejas ocupadas " << num_bandejas_en_uso << "Vegetariano " << ocupadas[0] 
             << ", Vegano" << ocupadas[1] << ",Celíaco " << ocupadas[2]  << endl ;
             sem_signal( mutex_cout );
          sem_signal( mutex_num_bandejas ); // señalar que ninguna hebra está ya leyendo o escribiendo 'num_bandejas_en_uso'
 
          // comer 
          comer( ncvg );
 
          // decrementar e imprimir el valor de 'num_bandejas_en_uso'
          sem_wait( mutex_num_bandejas ); // espera a que ninguna hebra esté leyendo o escribiendo 'num_bandejas_en_uso'
             num_bandejas_en_uso -- ;
             ocupadas[1]--;                 // Decrementamos las bandejas vegetarianas usadas
             sem_wait( mutex_cout );
             cout << "Cliente VEGANO " << ncvg << " devuelve bandeja,Bandejas ocupadas " << num_bandejas_en_uso << "Vegetariano " << ocupadas[0] 
             << ", Vegano" << ocupadas[1] << ",Celíaco " << ocupadas[2]  << endl ;
             sem_signal( mutex_cout );
          sem_signal( mutex_num_bandejas ); // señalar que ninguna hebra está ya leyendo o escribiendo 'num_bandejas_en_uso'
            //devuelve bandeja
       sem_signal( bandejas_veganas_libres ); // señalar que queda una bandeja vegana libre
 
       this_thread::sleep_for(chrono::milliseconds( aleatorio<tiempo_min_fuera_ms,tiempo_max_fuera_ms>() ) );
    }
 }

 /** -------------------------------------------------------------------------- **/
 /// @brief función que ejecutan las hebras celiacas
 /// @param ncc - número de orden del cliente celiaco
 /// 
 void celiaco( unsigned ncc )
 {
    while( true )
    {
      
       sem_wait( bandejas_celiaco_libres ); // esperar a que quede una bandeja celiaca libre
            //entra a comer

         // incrementar e imprimir el valor de 'num_bandejas_en_uso'
          sem_wait( mutex_num_bandejas ); // espera a que ninguna hebra esté leyendo o escribiendo 'num_bandejas_en_uso'
             num_bandejas_en_uso ++ ;
             ocupadas[2]++; // Incrementamos las bandejas celiacas usadas
             sem_wait( mutex_cout );
             cout << "Cliente CELIACO " << ncc << " toma bandeja,Bandejas ocupadas " << num_bandejas_en_uso << "Vegetariano " << ocupadas[0] 
             << ", Vegano" << ocupadas[1] << ",Celíaco " << ocupadas[2]  << endl ;
             sem_signal( mutex_cout );
          sem_signal( mutex_num_bandejas ); // señalar que ninguna hebra está ya leyendo o escribiendo 'num_bandejas_en_uso'
 
          // comer 
          comer(ncc );
 
          // decrementar e imprimir el valor de 'num_bandejas_en_uso' y bandejas ocupadas celiacas
          sem_wait( mutex_num_bandejas ); // espera a que ninguna hebra esté leyendo o escribiendo 'num_bandejas_en_uso'
             num_bandejas_en_uso -- ;
             ocupadas[2]++;                 // Decrementamos las bandejas vegetarianas usadas
             sem_wait( mutex_cout );
             cout << "Cliente CELIACO " << ncc << " devuelve bandeja,Bandejas ocupadas " << num_bandejas_en_uso << "Vegetariano " << ocupadas[0] 
             << ", Vegano" << ocupadas[1] << ",Celíaco " << ocupadas[2]  << endl ;
             sem_signal( mutex_cout );
          sem_signal( mutex_num_bandejas ); // señalar que ninguna hebra está ya leyendo o escribiendo 'num_bandejas_en_uso'

            //devuelve bandeja
       sem_signal( bandejas_celiaco_libres ); // señalar que queda una bandeja celiaca libre
 
       this_thread::sleep_for(chrono::milliseconds( aleatorio<tiempo_min_fuera_ms,tiempo_max_fuera_ms>() ) );
    }
 }
 
 /** -------------------------------------------------------------------------- **/
 /// @brief función principal
 /// @return int 
 ///
 int main()
 {
    cout << "-----------------------------------------------------------" << endl
         << "Examen Restaurante(Semáforos)" << endl
         << "-----------------------------------------------------------" << endl
         << flush ;
 
   
    // crear y lanzar las hebras
    thread hebras_vegetarianas[num_vegetariano],
           hebras_veganas[num_vegano] ,
           hebras_celiacas[num_celiaco];
 
    for( unsigned i = 0 ; i < num_vegetariano ; i++ )
    hebras_vegetarianas[i] = thread( vegetariano, i );
 
    for( unsigned i = 0 ; i < num_vegano ; i++ )
    hebras_veganas[i] = thread( vegano, i );
 
    for( unsigned i = 0 ; i < num_celiaco ; i++ )
    hebras_celiacas[i] = thread( celiaco, i );

    // esperar a que terminen las hebras
    for( unsigned i = 0 ; i < num_vegetariano ; i++ )
        hebras_vegetarianas[i].join();
    for( unsigned i = 0 ; i < num_vegano ; i++ )
        hebras_veganas[i].join();
    for( unsigned i = 0 ; i < num_celiaco ; i++ )
        hebras_celiacas[i].join();
 
 
 
 }
 