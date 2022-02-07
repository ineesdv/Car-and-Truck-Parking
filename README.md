# Car and Truck Parking simulation in C
Este programa simula un parking de m plantas, cada una con n plazas, en las que habrá un determinado número de coches y de camiones.
Los coches ocuparán un hueco del parking, mientras que los camiones ocuparán dos huecos contiguos.

Para la sincronización de recursos entre hilos, es necesario el uso de *threads* y *mutex de condición*.

</br>


**Compilación:** ``gcc parking.c -lpthread -o parking``


**Uso:** ``./parking <Plazas> <Plantas> <Coches> <Camiones>``
