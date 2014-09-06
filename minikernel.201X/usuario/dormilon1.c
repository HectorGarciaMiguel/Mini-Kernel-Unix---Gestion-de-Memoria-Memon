/*
 * usuario/dormilon.c
 *
 *  Minikernel. Versión 1.0
 *
 *  Fernando Pérez Costoya
 *
 */

/*
 * Programa de usuario que duerme dos veces.
 */
#include <stdio.h> 

#include "servicios.h"

int main(){
	int i,id;
	
	id=obtener_id_pr();
	
	dormir(1);
	
	printf("P3 (%d): comienza\n", id);

	printf("P3 (%d): CREA MUTEX \n", id);		
		if ( (i = crear_mutex("mutex77", 1)) < 0 )
			printf("error en mutex = %d\n", i);	



	printf("P3 (%d): termina\n", id);
	return 0;
}
