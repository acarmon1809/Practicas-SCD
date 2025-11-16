# 📚 Sistemas Concurrentes y Distribuidos (SCD) 2025-26

Este repositorio contiene las soluciones implementadas para las prácticas de la asignatura **Sistemas Concurrentes y Distribuidos** (SCD) de la Universidad de Granada (UGR), curso 2025-26. Las prácticas abordan la programación concurrente y distribuida usando modelos de **memoria compartida** (semáforos y monitores) y **paso de mensajes** (MPI).

***

## 💻 Prácticas y Temas Clave

| Práctica | Título | Herramienta | Temas Principales |
| :--- | :--- | :--- | :--- |
| **P1** | Sincronización con Semáforos | C++11 / Semáforos | Productor-Consumidor (un/múltiples) y el problema de los Fumadores. |
| **P2** | Casos Prácticos de Monitores | C++11 / Monitores SU | Múltiples Prod./Cons., Fumadores, y Lectores-Escritores con monitores Hoare.|
| **P3** | Algoritmos Distribuidos con MPI | MPI | Productor-Consumidor con buffer acotado, selección no determinista, y Cena de los Filósofos (con/sin Camarero). |
| **P4** | Implementación de Sistemas de Tiempo Real | C++11 / `chrono` | Diseño e implementación de un **Ejecutivo Cíclico** utilizando relojes monotónicos (`steady_clock`) y esperas bloqueadas (`sleep_until`).|

***

## ⚙️ Compilación

Para compilar las prácticas que usan semáforos o monitores (P1 y P2) es necesario enlazar con el archivo de la biblioteca de soporte (`scd.cpp`).

Utiliza el siguiente comando para compilar:

```bash
g++ -std=c++11 -pthread -o ejecutable.exe codigo.cpp scd.cpp
