#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

// Colores
#define boldText "\033[1m"
#define normalText "\033[0m"
#define endColor "\033[0m\e[0m"

// Variables Globales
int COCHES;
int CAMIONES;
int PLAZAS;
int PLANTAS;
int PLAZASTOTALES;
int* parking;
int nPlazasOcupadas;

pthread_mutex_t mutex;
pthread_cond_t plazaLiberada;

void* parkingCoche(void* id);
void* parkingCamion(void* id);

int main(int argc, char const *argv[])
{
    //Argumentos introducidos
    switch (argc)
    {
        case 3:
            PLAZAS = atoi(argv[1]);
            PLANTAS = atoi(argv[2]);
            COCHES = 2 * PLANTAS * PLAZAS;
            CAMIONES = 0;
            break;
        
        case 4:
            PLAZAS = atoi(argv[1]);
            PLANTAS = atoi(argv[2]);
            COCHES = atoi(argv[3]);
            CAMIONES = 0;
            break;
        
        case 5:
            PLAZAS = atoi(argv[1]);
            PLANTAS = atoi(argv[2]);
            COCHES = atoi(argv[3]);
            CAMIONES = atoi(argv[4]);
            break;

        default:
            printf("Número de argumentos inválido.\n");
            return 1;
            break;
    }

    PLAZASTOTALES = PLAZAS * PLANTAS;

    //Comprobación de argumentos
    if (!(PLAZAS>0 && COCHES>0 && COCHES<100 && PLANTAS>0)){
        printf("Alguno de los argumentos es inválido: todos los argumentos deben ser positivos, y el número de coches debe ser menor que 100.\n");
        return 1;
    }

    // Inicializamos el parking y las variables utilizadas
    srand(time(NULL));
    nPlazasOcupadas = 0;
    parking = (int *) malloc(PLAZASTOTALES * sizeof(int));
    memset(parking, 0, PLAZASTOTALES);
    int* id_coches = (int *) malloc(COCHES * sizeof(int));
    int* id_camiones = (int *) malloc(CAMIONES * sizeof(int));

    //Los coches se identifican con el número 1-99 y los camiones a partir del 100
    for (int i = 0; i < COCHES; i++){
        id_coches[i] = i+1;
    }

    for (int i = 0; i < CAMIONES; i++){
        id_camiones[i] = 100+i+1;
    }

    // Inicializamos los threads y el mutex asociado
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&plazaLiberada, NULL);

    pthread_t* threadsCoches = (pthread_t *) malloc(COCHES * sizeof(pthread_t));
    pthread_t* threadsCamiones = (pthread_t *) malloc(CAMIONES * sizeof(pthread_t));

    for (int i = 0; i < COCHES; i++){
        pthread_create(&threadsCoches[i], NULL, parkingCoche, (void *) &id_coches[i]);
    }

    for (int i = 0; i < CAMIONES; i++){
        pthread_create(&threadsCamiones[i], NULL, parkingCamion, (void *) &id_camiones[i]);
    }
    
    // Forzamos al programa a continuar en ejecución
    while(1);

    return 0;
}

int esperarPlazaCamion(){

    for (int i = 0; i < PLAZASTOTALES; i++)
    {
        if (parking[i] == 0 && parking[i+1] == 0 && (i+1) % PLAZAS != 0){ 
            return 1;
        }
    }
    return 0; 
}

void* parkingCoche(void* id){
   
    int tiempo;
    
    while(1){

        // Esperamos aleatoriamente entre 1-12 segundos
        tiempo = (rand() % 12) + 1; 
        sleep(tiempo);

        // Bloquear el mutex
        pthread_mutex_lock(&mutex);

        // Esperar mientras no hay plazas
        while(nPlazasOcupadas == PLAZASTOTALES){
            // Al recibir la señal de condición se vuelve a hacer la comprobación del while
            pthread_cond_wait(&plazaLiberada, &mutex);
        }


        int plazaDisponible;
        int encontrado = 0;

        for (int i = 0; i<PLAZASTOTALES && !encontrado ; i++)
        {
            if(parking[i] == 0){
                plazaDisponible = i;
                encontrado = 1;
            }
        }

        int identificador = *(int*) id;
            
        // Aparcamos el coche
        parking[plazaDisponible] = identificador;
        nPlazasOcupadas++;
        printf("%sENTRADA: Coche %d aparca en %d. Plazas libres: %d%s\n", boldText, identificador, plazaDisponible, PLAZASTOTALES-nPlazasOcupadas, endColor);

        int planta = 0;
        printf("Planta %d: ", planta);

        for (int i = 0; i < PLAZASTOTALES; i++){
            if(i % PLAZAS == 0 && i!=0){ 
                planta++;
                printf("\nPlanta %d: ", planta);
            }

            printf("[%d]\t", parking[i]);

        }
        printf("\n\n");

        // Desbloquear el mutex
        pthread_mutex_unlock(&mutex);

        // Esperar en el aparcamiento
        tiempo = (rand() % 5) + 1; 
        sleep(tiempo);
        
        // Bloquear el mutex
        pthread_mutex_lock(&mutex);

        // Salir del aparcamiento
        int plazaCoche;
        encontrado = 0;

        for (int i = 0; i<PLAZASTOTALES && !encontrado ; i++)
        {
            if(parking[i] == identificador){
                parking[i] = 0;
                encontrado = 1;
            }
        }
        nPlazasOcupadas--;
        printf("%sSALIDA: Coche %d saliendo. Plazas libres totales: %d%s\n", boldText, identificador, PLAZASTOTALES-nPlazasOcupadas, endColor);
        planta = 0;
        printf("Planta %d: ", planta);

        for (int i = 0; i < PLAZASTOTALES; i++)
        {
            if(i % PLAZAS == 0 && i!=0){ 
                planta++;
                printf("\nPlanta %d: ", planta); 
                
            }
            printf("[%d]\t", parking[i]);
        }
        printf("\n\n"); 

        // Enviamos la señal de que se ha liberado una plaza
        pthread_cond_signal(&plazaLiberada);
 
        // Desbloquear el mutex
        pthread_mutex_unlock(&mutex);

    }

}

void* parkingCamion(void* id){

    int tiempo;
    
    while(1){
        tiempo = (rand() % 5) + 1; 
        sleep(tiempo);

        // Bloquear el mutex
        pthread_mutex_lock(&mutex);

        // Esperar mientras no hay plazas
        while(esperarPlazaCamion() == 0){
            // Al recibir la señal de condición se vuelve a hacer la comprobación del while
            pthread_cond_wait(&plazaLiberada, &mutex);
        }

        int plazaDisponible;
        int encontrado = 0;

        for (int i = 0; i<PLAZASTOTALES && !encontrado ; i++)
        {
            if (parking[i] == 0 && parking[i+1] == 0 && (i+1) % PLAZAS != 0){ 
                plazaDisponible = i;
                encontrado = 1;
            }
        }

        int identificador = *(int*) id;
        
        // Aparcamos el camión
        parking[plazaDisponible] = identificador;
        parking[plazaDisponible+1] = identificador;
        nPlazasOcupadas += 2;
        
        printf("%sENTRADA: Camión %d aparca en %d y %d. Plazas libres: %d%s\n", boldText, identificador, plazaDisponible, plazaDisponible+1, PLAZASTOTALES-nPlazasOcupadas, endColor);
        int planta = 0;
        printf("Planta %d: ", planta);

        for (int i = 0; i < PLAZASTOTALES; i++)
        {
            if(i % PLAZAS == 0 && i!=0){ 
                planta++;
                printf("\nPlanta %d: ", planta);
                
            }
            printf("[%d]\t", parking[i]);
        }
        printf("\n\n"); 

        // Desbloquear el mutex
        pthread_mutex_unlock(&mutex);

        // Esperar en el aparcamiento
        tiempo = (rand() % 5) + 1; 
        sleep(tiempo);

        // Bloquear el mutex
        pthread_mutex_lock(&mutex);

        // Salir del aparcamiento
        int plazaCoche;
        encontrado = 0;

        for (int i = 0; i<PLAZASTOTALES && !encontrado ; i++)
        {
            if(parking[i] == identificador){
                parking[i] = 0;
                parking[i+1] = 0;
                encontrado = 1;
            }
        }

        nPlazasOcupadas -= 2;


        printf("%sSALIDA: Camión %d saliendo. Plazas libres: %d%s\n", boldText, identificador, PLAZASTOTALES-nPlazasOcupadas, endColor);
        planta = 0;
        printf("Planta %d: ", planta);

        for (int i = 0; i < PLAZASTOTALES; i++)
        {
            if(i % PLAZAS == 0 && i!=0){ 
                planta++;
                printf("\nPlanta %d: ", planta);  
                
            }
            printf("[%d]\t", parking[i]);
        }
        printf("\n\n"); 

        // Enviamos la señal de que se ha liberado una plaza
        pthread_cond_signal(&plazaLiberada);
 
        // Desbloquear el mutex
        pthread_mutex_unlock(&mutex);

    }
}