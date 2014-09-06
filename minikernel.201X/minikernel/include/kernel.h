/*
 *  minikernel/include/kernel.h
 *
 *  Minikernel. Versión 1.0
 *
 *  Fernando Pérez Costoya
 *
 */

/*
 *
 * Fichero de cabecera que contiene definiciones usadas por kernel.c
 *
 *      SE DEBE MODIFICAR PARA INCLUIR NUEVA FUNCIONALIDAD
 *
 */

#ifndef _KERNEL_H
#define _KERNEL_H

#include "const.h"
#include "HAL.h"
#include "llamsis.h"

/* Constantes para mutex */
#define NO_RECURSIVO 0
#define RECURSIVO 1



 /* Tipo mutex abierto*/
struct tipo_mutex_abiertos{
	int descriptor;
	int estado; //LISTO = 1 & NO_USADA = 0
 } mutex_abiertos = {0, NO_USADA};

/*
 *
 * Definicion del tipo que corresponde con el BCP.
 * Se va a modificar al incluir la funcionalidad pedida.
 *
 */
 
typedef struct BCP_t *BCPptr;


typedef struct BCP_t {
        int id;				/* ident. del proceso */
        int estado;			/* TERMINADO|LISTO|EJECUCION|BLOQUEADO */
        contexto_t contexto_regs;	/* copia de regs. de UCP */
        void * pila;			/* dir. inicial de la pila */
		BCPptr siguiente;		/* puntero a otro BCP */
		void *info_mem;			/* descriptor del mapa de memoria */
		int segundos;			/* segundos * TICK que duerme el proceso */
		struct tipo_mutex_abiertos mutex_abiertos_proceso[NUM_MUT_PROC]; /* Conjunto de mutex del proceso */
		int bloqueado_por_mutex;

} BCP;

/*
 *
 * Definicion del tipo que corresponde con la cabecera de una lista
 * de BCPs. Este tipo se puede usar para diversas listas (procesos listos,
 * procesos bloqueados en semáforo, etc.).
 *
 */

typedef struct{
	BCP *primero;
	BCP *ultimo;
} lista_BCPs;


/*
 * Variable global que identifica el proceso actual
 */

BCP * p_proc_actual=NULL;

/*
 * Variable global que representa la tabla de procesos
 */

BCP tabla_procs[MAX_PROC];

/*
 * Variable global que representa la cola de procesos listos
 */
lista_BCPs lista_listos= {NULL, NULL};

/*
 * Variable global que representa la cola de procesos esperando plazos
 */
lista_BCPs lista_esperando= {NULL, NULL};

/*
 * Variable global que representa la cola de procesos bloqueados por un mutex
 */
 
lista_BCPs lista_bloqueados_mutex= {NULL, NULL};

/*
 * Variable global que representa la cola de procesos bloqueados al crear mutex
 */
 
lista_BCPs lista_bloqueados_crear_mutex= {NULL, NULL};


/*
 *
 * Definición del tipo que corresponde con una entrada en la tabla de
 * llamadas al sistema.
 *
 */
typedef struct{
	int (*fservicio)();
} servicio;


/*
 * Definición de estructura tiempos proceso y variable global
 */
struct tiempos_ejec {
	int usuario;
	int sistema;
	int nulo;
} tiempos;


/*
 * Declaración de variable zona de memoria de usuario tiempos
 */
int accediendo_en_memoria_tiempos_proceso = 0;

/*
 * Conjunto de mutex del sistema
 */
struct tipo_mutex {
	char *nombre;
	int tipo;
	int valor;
	int ultimo_proceso_lock;
}mutex = { NULL, 0 , 0, -1 };
 
struct tipo_mutex mutex_sistema[NUM_MUT];
  
  

 
 
/*
 * Prototipos de las rutinas que realizan cada llamada al sistema
 */
int sis_crear_proceso();
int sis_terminar_proceso();
int sis_escribir();
int obtener_id_pr();
int sis_dormir();
int sis_tiempos_proceso();
int sis_crear_mutex();
int sis_abrir_mutex();
int sis_lock();
int sis_unlock();
int sis_cerrar_mutex();


/*
 * Variable global que contiene las rutinas que realizan cada llamada
 */
servicio tabla_servicios[NSERVICIOS]={	{sis_crear_proceso},
					{sis_terminar_proceso},
					{sis_escribir},
					{obtener_id_pr},
					{sis_dormir},
					{sis_tiempos_proceso},
					{sis_crear_mutex},
					{sis_abrir_mutex},
					{sis_lock},
					{sis_unlock},
					{sis_cerrar_mutex}};
					


#endif /* _KERNEL_H */

