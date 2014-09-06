/*
 *  memon/fallo.c
 *
 *  Fernando P�rez Costoya
 *
 */

/*
 *
 * Fichero que contiene la rutina que trata el fallo de p�gina.
 * Esta rutina se encarga de llevar las estad�sticas de la monitorizaci�n
 *
 *      SE DEBE MODIFICAR
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>

#include "mapa.h"
#include "marcos.h"
#include "apoyo.h"

/* Variables que almacenan las estad�sticas de la monitorizaci�n */
int fallos_total=0;

/* �El fallo de p�gina se debe a la falta de memoria f�sica? 
*/
int fallos_no_forzados=0;  /* S� */
int fallos_forzados=0; /* No, ocurre debido a la carga por demanda */

/* �El fallo de p�gina ha causado un reemplazo? */
int fallos_sin_reemplazo=0;
int fallos_con_reemplazo=0;

/* �El fallo de p�gina implica una operaci�n de lectura de fichero, de
   swap o no implica lectura? */
int fallos_sin_lectura=0; /* sin lectura (o sea, rellenar con 0) */
int fallos_con_lectura_fichero=0; /* con lectura de fichero */
int fallos_con_lectura_swap=0; /* con lectura de swap */

/* N�mero de operaciones de escritura (pageouts) */
int escrituras_en_fichero=0;	/* se ha escrito en fichero */
int escrituras_en_swap=0;	/* se ha escrito en swap */

/* Rutina de reemplazo que se va a usar (iniciado por memon.c) */
int (*reemplazo)();

/* Rutina que trata el fallo de p�gina */

void fallo_pagina(void *dir_fallo) {
	int numero_marco;

	printf("-------- fallo %p -------------\n", dir_fallo);
		
	// Encontrar region y pagina a la que pertenece la pagina
	entrada_tabla_paginas *pagina;	
		
	if ( (pagina = encontrar_pagina (dir_fallo)) == NULL ){ 	
		//No pertenece a ninguna
		printf("acceso a memoria inv�lido %p\n", dir_fallo);
		_exit(1);
	}
		
	fallos_total++;
	
	if (!pagina->forzado){
		fallos_forzados++;
		pagina->forzado = 1;
	}else
		fallos_no_forzados++;
	
	
	// Reservar un marco libre
	if ( (numero_marco = reservar_marco_libre()) < 0){
	// no hay marco libre
		fallos_con_reemplazo++;
		// Aplicar el algoritmo de reemplazo que selecciona un marco
		numero_marco = reemplazo_FIFO();
		// Invalidar la pagina contenida en ese marco
		if ( mprotect( leer_entrada_marco(numero_marco)->dir_inicial, get_tam_pagina() , PROT_NONE) ){
			perror("Error devolviendo permisos");
		_exit(1);
		}
	}else{
	// si hay marco libre
		fallos_sin_reemplazo++;
	}
			
	// Asociar la nueva p�gina con el marco
	rellenar_entrada_marco(numero_marco, regnum(pagina->region), pagnum(pagina));
	// Poner como v�lida la pagina
	printf("------- dir mprotect %p -------------\n", pagina->dir_inicial);
	if( mprotect( pagina->dir_inicial, get_tam_pagina() , pagina->region->prot) < 0 ){
		perror("Error devolviendo permisos");
		_exit(1);
	}
	
	printf("-------- FIN fallo %p -------------\n", dir_fallo);
	return;
}
