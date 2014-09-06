/*
 *  memon/mapa.c
 *
 *  Fernando P�rez Costoya
 *
 */

/*
 *
 * Fichero que contiene operaciones relacionadas con la gesti�n del
 * mapa del proceso (sus regiones y las p�ginas contenidas en las mismas). 
 * Puede a�adir las funciones que considere oportuno. 
 *
 *	SE DEBE MODIFICAR PARA INCLUIR LA FUNCIONALIDAD PEDIDA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>

#include "mapa.h"
#include "apoyo.h"
#include "marcos.h"

/* Variable global que contiene el tama�o de la p�gina en el sistema */
extern long tam_pagina;

/* N�mero m�ximo de regiones previsto */
#define MAX_REG 16

/* Definici�n de la tabla de regiones */
entrada_tabla_regiones tabla_regiones[MAX_REG];

/* Prototipos de las funciones internas */

/* Reserva una regi�n libre */
static int reservar_region();

/* Obtiene la regi�n a a partir de su direcci�n inicial */
static int region(const void *dir);

/* Crea una tabla de p�ginas */
static entrada_tabla_paginas *creacion_tabla_paginas(
				entrada_tabla_regiones *reg);

/* Elimina una tabla de p�ginas */
static void eliminar_tabla_paginas(const entrada_tabla_regiones *reg);

/* Inicia una entrada de la tabla de p�ginas. DEBE COMPLETARSE */
static void iniciar_entrada_tpag(entrada_tabla_regiones *reg,
				void *dir, entrada_tabla_paginas *entrada);

/* Libera una entrada de la tabla de p�ginas. DEBE COMPLETARSE */
static void liberar_entrada_tpag(entrada_tabla_paginas *entrada);

/* FUNCIONES INVOCADAS DESDE EL M�DULO DE APOYO */

/* nos informan de que se ha creado una regi�n
	dir: direcci�n de comienzo de la regi�n
	inodo: fichero al que est� vinculada (0 si es an�nima)
	prot: permisos de acceso a la regi�n
	tamano: tama�o de la regi�n
	compartida: �es una regi�n de tipo compartida?
*/
void creacion_region(void *dir, int inodo, int prot,
			int tamano, int compartida)
{
        int reg;

	reg=reservar_region();
	/* Compruebo que no se han agotado las regiones */
	assert(reg!=-1);

        tabla_regiones[reg].npags=tamano/tam_pagina;
        tabla_regiones[reg].dir_inicial=dir;
        tabla_regiones[reg].ino=inodo;
        tabla_regiones[reg].prot=prot;
        tabla_regiones[reg].compartida=compartida;
	tabla_regiones[reg].tabla_paginas=
		creacion_tabla_paginas(&tabla_regiones[reg]);
}

/* nos informan de que se ha eliminado una regi�n
	dir: direcci�n de comienzo de la regi�n
*/
void eliminacion_region(const void *dir){
	int reg;

	reg=region(dir);
	/* Compruebo que la regi�n exsite */
	assert(reg!=-1);
	
        tabla_regiones[reg].usada=0;
	eliminar_tabla_paginas(&tabla_regiones[reg]);
}

/* nos informan de que ha cambiado el tama�o de una regi�n
        dir: direcci�n de comienzo de la regi�n
        tamano: nuevo tama�o de la regi�n
*/
void cambio_tam_region(void *dir, int tamano){
	int i = 0;
	int region_encontrada = 0;
	
	
	
	
	// Encontrar region -> i
	while ( i < MAX_REG && region_encontrada == 0 ){
		if (tabla_regiones[i].usada && tabla_regiones[i].dir_inicial == dir)
			region_encontrada = 1;
		else
		i++;
	}
	
	// Comparar tamano
	if (tabla_regiones[i].npags != tamano){
		if (tabla_regiones[i].npags < tamano){
			// Incrementar el tama�o
		
			(tabla_regiones[i].tabla_paginas) = realloc ( (tabla_regiones[i].tabla_paginas), tamano );
			
			int j = tabla_regiones[i].npags; // Tamano antiguo
			void *dir = tabla_regiones[i].tabla_paginas[j-1].dir_inicial + tam_pagina;
			
			for (  ; j < tamano/tam_pagina; j++, dir+=tam_pagina){
				iniciar_entrada_tpag(&tabla_regiones[i], dir, &(tabla_regiones[i].tabla_paginas[j]) );
				}
			tabla_regiones[i].npags = tamano/tam_pagina;
			
		}else{
			// Cambiar el tama�o a menos
			
			// Elimina en marco si fuese residente
			int j = tabla_regiones[i].npags-1;
			for (   ; j >= tamano ; j-- )
				liberar_entrada_tpag(&tabla_regiones[i].tabla_paginas[j]);
		
			// Poner a 0 
			
			
			// Reajustar tamano
			(tabla_regiones[i].tabla_paginas) = realloc ( (tabla_regiones[i].tabla_paginas), tamano * sizeof(entrada_tabla_paginas) );

			
		}
	}
}

/* Funciones auxiliares que a partir de la direcci�n de una entrada
   obtienen su posici�n en la tabla */

int regnum(entrada_tabla_regiones *reg) {

	return reg-tabla_regiones;
}
int pagnum(entrada_tabla_paginas *pag) {

	return pag-(pag->region->tabla_paginas);
}

/* Funci�n auxiliar que a partir de un n�mero de p�gina de una
regi�n devuelve la direcci�n de su entrada en la tabla */
entrada_tabla_paginas *entrada_pagina(int regnum, int pagnum) {
	return &(tabla_regiones[regnum].tabla_paginas[pagnum]);
}

/* FUNCIONES INTERNAS */

/* Reserva una regi�n libre */
static int reservar_region() {
	int i;

        for (i=0; i<MAX_REG; i++)
                if (!(tabla_regiones[i].usada)) {
                        tabla_regiones[i].usada=1;
                        return i;
		}
	return -1;
}

/* Obtiene la regi�n a partir de su direcci�n inicial */
static int region(const void *dir){
        int i;

        for (i=0; i<MAX_REG; i++)
                if ((tabla_regiones[i].usada) &&
		   (tabla_regiones[i].dir_inicial==dir))
                        return i;
	return -1;
}



/* Crea una tabla de p�ginas */
static entrada_tabla_paginas *creacion_tabla_paginas(
					entrada_tabla_regiones *reg) {
	int i;
	char *dir;
	int npags;
	entrada_tabla_paginas *tpag;

	dir=reg->dir_inicial;
	npags=reg->npags;
	tpag=malloc(npags * sizeof(entrada_tabla_paginas));
	for (i=0 ; i<npags; i++, dir+=tam_pagina)
		iniciar_entrada_tpag(reg, dir, &tpag[i]);

	return tpag;
}

/* Elimina una tabla de p�ginas */
static void eliminar_tabla_paginas(const entrada_tabla_regiones *reg) {
	int i;
	int npags;
	entrada_tabla_paginas *tpag;
	int reg_comp;

	reg_comp=reg->compartida;
	npags=reg->npags;
	tpag=reg->tabla_paginas;
	if (!reg_comp)
		for (i=0; i<npags; i++)
			liberar_entrada_tpag(&tpag[i]);
	free(tpag);
}

/* Inicia una entrada de la tabla de p�ginas */
static void iniciar_entrada_tpag(entrada_tabla_regiones *reg,
				void *dir,
				entrada_tabla_paginas *entrada){

	entrada->dir_inicial=dir;
	entrada->region=reg;
	entrada->nmarco = -1;
	
	mprotect( dir, tam_pagina, PROT_NONE );

	/* DEBE COMPLETARSE */

}

/* Libera una entrada de la tabla de p�ginas */
static void liberar_entrada_tpag(entrada_tabla_paginas *entrada){

	
	//printf("pagina a liberar: reg %d pag %d\n", regnum(entrada->region), (pagnum(entrada)));
	//printf("---------------------------------------------------\n");

	if (!(entrada->region->compartida)){
		if (entrada->nmarco != -1){		
			eliminar_pagina_de_marco(entrada->nmarco); 
			entrada->nmarco = -1;
		}
	}
}

/* PUEDE INCLUIR LAS FUNCIONES QUE CONSIDERE OPORTUNO */
/* Encuentra la pagina que est� incluida la direcci�n pasada por par�metro */
entrada_tabla_paginas *encontrar_pagina(void *dir_fallo){
	
	int i = 0;
	int j = 0;
	int region_encontrada = 0;
	int pag_encontrada = 0;

	while ( i < MAX_REG && region_encontrada == 0 ){
	
		if (tabla_regiones[i].usada){
	
			if ( (tabla_regiones[i].dir_inicial <= dir_fallo) &&  
					( dir_fallo < (tabla_regiones[i].dir_inicial + tabla_regiones[i].npags*tam_pagina) )){
		
				region_encontrada = 1;
					
				while ( j < tabla_regiones[i].npags && pag_encontrada == 0 ){
						
					if ( ( tabla_regiones[i].tabla_paginas[j].dir_inicial <= dir_fallo ) &&  
						( dir_fallo < (tabla_regiones[i].tabla_paginas[j].dir_inicial+tam_pagina) )){

						pag_encontrada = 1;
					}
					j++;
				}
			}
		}
		i++;
	}
	
	if (pag_encontrada == 0)
		return NULL;
	return entrada_pagina(i-1, j-1);
	
}

/* devuelve el tamano de una pagina*/
int get_tam_pagina(){
	return tam_pagina;
}


/* Solo depuraci�n */
void imprimir_regiones(){
	int i = 0;
	int j = 0;
	for (i = 0 ; i < MAX_REG ; i++ ){
		printf("Region %d, usada %d, dir inicial %p, npags %d, prot %d, compartida % d\n", 
		i, tabla_regiones[i].usada, tabla_regiones[i].dir_inicial, tabla_regiones[i].npags, tabla_regiones[i].prot, tabla_regiones[i].compartida);
		for (j = 0 ; j < tabla_regiones[i].npags ; j++){
			printf("----->Pagina %d, dir %p, residente %d\n", j, tabla_regiones[i].tabla_paginas[j].dir_inicial, tabla_regiones[i].tabla_paginas[j].nmarco);
		}
		j = 0;
	}
}