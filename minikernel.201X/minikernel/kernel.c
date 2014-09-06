/*
 *  kernel/kernel.c
 *
 *  Minikernel. Versión 1.0
 *
 *  Fernando Pérez Costoya
 *
 */
/*
* Isabel Perera Montero 	a930092
* Héctor García Miguel 		n060036
*/
 
 
 
 
 
/*
 *
 * Fichero que contiene la funcionalidad del sistema operativo
 *
 */
#include <unistd.h>
#include "kernel.h"	/* Contiene defs. usadas por este modulo */
#include <string.h>

void imprimir_estado(){
	int i, j = 0;
	
			printk("***************************************************************\n");
			printk("---> MUTEX DEL SISTEMA\n");
			printk("***************************************************************\n");

			for (i=0; i < NUM_MUT ; i++){

				printk("----> mutex: %d, nombre = %s, tipo = %d, valor = %d\n", 
				i, 
				mutex_sistema[i].nombre, 
				mutex_sistema[i].tipo, 
				mutex_sistema[i].valor);

			}


			
			
			printk("\n***************************************************************\n");
			printk("------> PROCESOS LISTOS\n");
			printk("***************************************************************\n");


			BCP * puntero_a_proceso_listo = lista_listos.primero;
			while (puntero_a_proceso_listo != NULL){

				printk("---------> proceso %d\n", puntero_a_proceso_listo->id);
				j=0;
				while ( j < NUM_MUT_PROC ){
					printk("------------> espacio %d: estado = %d, descriptor = %d\n",
										j, 
										puntero_a_proceso_listo->mutex_abiertos_proceso[j].estado, 
										puntero_a_proceso_listo->mutex_abiertos_proceso[j].descriptor);
					j++;
				}
				puntero_a_proceso_listo = puntero_a_proceso_listo->siguiente;
			}
			
			printk("\n***************************************************************\n");
			printk("------> PROCESOS DORMIDOS\n");
			printk("***************************************************************\n");

			
			BCP * puntero_a_proceso_dormido = lista_esperando.primero;
			while (puntero_a_proceso_dormido != NULL){

				printk("---------> proceso %d\n", puntero_a_proceso_dormido->id);
				j=0;
				while ( j < NUM_MUT_PROC ){
					printk("------------> espacio %d: estado = %d, descriptor = %d\n",
										j, 
										puntero_a_proceso_dormido->mutex_abiertos_proceso[j].estado, 
										puntero_a_proceso_dormido->mutex_abiertos_proceso[j].descriptor);
					j++;
				}
				puntero_a_proceso_dormido = puntero_a_proceso_dormido->siguiente;
			}	
			
			
			printk("\n***************************************************************\n");
			printk("------> PROCESOS BLOQUEADOS POR MUTEX\n");
			printk("***************************************************************\n");

			BCP * puntero_a_proceso_bloqueado = lista_bloqueados_mutex.primero;
			while (puntero_a_proceso_bloqueado != NULL){
			
				printk("---------> proceso %d y bloqueado por = %d\n", 
				puntero_a_proceso_bloqueado->id,
				puntero_a_proceso_bloqueado->bloqueado_por_mutex);
				j=0;	
				while ( j < NUM_MUT_PROC ){
					printk("------------> espacio %d: estado = %d, descriptor = %d\n",
										j, 
										puntero_a_proceso_bloqueado->mutex_abiertos_proceso[j].estado, 
										puntero_a_proceso_bloqueado->mutex_abiertos_proceso[j].descriptor);
					j++;
				}
				puntero_a_proceso_bloqueado = puntero_a_proceso_bloqueado->siguiente;
			}	

			printk("\n***************************************************************\n");
			printk("------> PROCESOS BLOQUEADOS POR CREAR MUTEX\n");
			printk("***************************************************************\n");

			BCP * puntero_a_proceso_bloqueado_crear_mutex = lista_bloqueados_crear_mutex.primero;
			while (puntero_a_proceso_bloqueado_crear_mutex != NULL){
			
				printk("---------> proceso %d\n", puntero_a_proceso_bloqueado_crear_mutex->id);
				j=0;	
				while ( j < NUM_MUT_PROC ){
					printk("------------> espacio %d: estado = %d, descriptor = %d\n",
										j, 
										puntero_a_proceso_bloqueado_crear_mutex->mutex_abiertos_proceso[j].estado, 
										puntero_a_proceso_bloqueado_crear_mutex->mutex_abiertos_proceso[j].descriptor);
					j++;
				}
				puntero_a_proceso_bloqueado_crear_mutex = puntero_a_proceso_bloqueado_crear_mutex->siguiente;
			}	
			
			
		printk("\n\n");

			
			
}

/*
 * Si son iguales devuelve 0 y distintos 1
 */

int comparar_nombre(char *nombre1, char *nombre2){
	int i=0;
	int distintos = 0;

		if (nombre1 == NULL || nombre2 ==NULL)
			distintos = 1;
		else{

			while (( nombre1[i] != '\0' || nombre2[i] != '\0' ) 
								&& distintos==0 && i < MAX_NOM_MUT){
		
				if ( *(nombre1+i) != *(nombre2+i) )
					distintos = 1;
				i++;
			}
		}
	
	return distintos;
}

/*
 *
 * Funciones relacionadas con la tabla de procesos:
 *	iniciar_tabla_proc buscar_BCP_libre
 *
 */

/*
 * Función que inicia la tabla de procesos
 */
static void iniciar_tabla_proc(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		tabla_procs[i].estado=NO_USADA;
}

/*
 * Función que busca una entrada libre en la tabla de procesos
 */
static int buscar_BCP_libre(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		if (tabla_procs[i].estado==NO_USADA)
			return i;
	return -1;
}

/*
 *
 * Funciones que facilitan el manejo de las listas de BCPs
 *	insertar_ultimo eliminar_primero eliminar_elem
 *
 * NOTA: PRIMERO SE DEBE LLAMAR A eliminar Y LUEGO A insertar
 */

/*
 * Inserta un BCP al final de la lista.
 */
static void insertar_ultimo(lista_BCPs *lista, BCP * proc){
	if (lista->primero==NULL)
		lista->primero= proc;
	else
		lista->ultimo->siguiente=proc;
	lista->ultimo= proc;
	proc->siguiente=NULL;
}

/*
 * Elimina el primer BCP de la lista.
 */
static void eliminar_primero(lista_BCPs *lista){

	if (lista->ultimo==lista->primero)
		lista->ultimo=NULL;
	lista->primero=lista->primero->siguiente;
}

/*
 * Elimina un determinado BCP de la lista.
 */
static void eliminar_elem(lista_BCPs *lista, BCP * proc){
	BCP *paux=lista->primero;

	if (paux==proc)
		eliminar_primero(lista);
	else {
		for ( ; ((paux) && (paux->siguiente!=proc));
			paux=paux->siguiente);
		if (paux) {
			if (lista->ultimo==paux->siguiente)
				lista->ultimo=paux;
			paux->siguiente=paux->siguiente->siguiente;
		}
	}
}

/*
 *
 * Funciones relacionadas con la planificacion
 *	espera_int planificador
 */

/*
 * Espera a que se produzca una interrupcion
 */
static void espera_int(){
	int nivel;

	printk("-> NO HAY LISTOS. ESPERA INT\n");
	/* Baja al mínimo el nivel de interrupción mientras espera */
	nivel=fijar_nivel_int(NIVEL_1);
	halt();
	fijar_nivel_int(nivel);
	
}

/*
 * Función de planificacion que implementa un algoritmo FIFO.
 */
static BCP * planificador(){
	while (lista_listos.primero==NULL)
		espera_int();		/* No hay nada que hacer */
	return lista_listos.primero;
}

/*
 *
 * Funcion auxiliar que termina proceso actual liberando sus recursos.
 * Usada por llamada terminar_proceso y por rutinas que tratan excepciones
 *
 */
static void liberar_proceso(){

	BCP * p_proc_anterior;
	int a=0;
	int encontrado=0;
	int j=0;
	int mutexid=0;

	/* ELIMINAR MUTEX ABIERTOS */
	while (a < NUM_MUT_PROC ){
		
		if (p_proc_actual->mutex_abiertos_proceso[a].estado == LISTO){
		
			p_proc_actual->mutex_abiertos_proceso[a].estado = NO_USADA;
						
			// comprobar si hay que eliminar en  mutex_sistema -> 
			// cuando ningun otro proceso lo tiene abierto	
				BCP * puntero_a_proceso_listo = lista_listos.primero->siguiente;
				while (puntero_a_proceso_listo != NULL && encontrado == 0){
					j=0;
					while ( j < NUM_MUT_PROC && encontrado == 0){
					
						if (puntero_a_proceso_listo->mutex_abiertos_proceso[j].estado == LISTO){
							if (puntero_a_proceso_listo->mutex_abiertos_proceso[j].descriptor 
										== p_proc_actual->mutex_abiertos_proceso[a].descriptor){
								encontrado = 1;
							}
						}
						j++;
					}
					puntero_a_proceso_listo = puntero_a_proceso_listo->siguiente;
				}
				
				
				BCP * puntero_a_proceso_dormido = lista_esperando.primero;
				while (puntero_a_proceso_dormido != NULL && encontrado == 0){
				j=0;
					while ( j < NUM_MUT_PROC && encontrado == 0){
										
						if (puntero_a_proceso_dormido->mutex_abiertos_proceso[j].estado == LISTO){
							if (puntero_a_proceso_dormido->mutex_abiertos_proceso[j].descriptor 
											== p_proc_actual->mutex_abiertos_proceso[a].descriptor){
								encontrado = 1;
							}
						}
						j++;
					}
					puntero_a_proceso_dormido = puntero_a_proceso_dormido->siguiente;
				}	
				
	
				
				BCP * puntero_a_proceso_bloqueado = lista_bloqueados_mutex.primero;
				while (puntero_a_proceso_bloqueado != NULL)	{
				

					if (p_proc_actual->mutex_abiertos_proceso[a].descriptor == puntero_a_proceso_bloqueado->bloqueado_por_mutex){
				
						int nivel_anterior=fijar_nivel_int(NIVEL_3);
							eliminar_elem( &lista_bloqueados_mutex, puntero_a_proceso_bloqueado );
							insertar_ultimo(  &lista_listos, puntero_a_proceso_bloqueado );	
						fijar_nivel_int(nivel_anterior);
						encontrado = 1;
						mutex_sistema[p_proc_actual->mutex_abiertos_proceso[a].descriptor].valor = 0;
				
					}
				
					puntero_a_proceso_bloqueado = puntero_a_proceso_bloqueado->siguiente;

				}
				
				
				BCP * puntero_a_proceso_bloqueado_crear_mutex = lista_bloqueados_crear_mutex.primero;
				while (puntero_a_proceso_bloqueado_crear_mutex != NULL && encontrado == 0){
				j=0;	
					while ( j < NUM_MUT_PROC && encontrado == 0){
					
						if (puntero_a_proceso_bloqueado_crear_mutex->mutex_abiertos_proceso[j].estado == LISTO){
							if (puntero_a_proceso_bloqueado_crear_mutex->mutex_abiertos_proceso[j].descriptor 
											== p_proc_actual->mutex_abiertos_proceso[a].descriptor){
								encontrado = 1;
							}
						}
						j++;
					}
					puntero_a_proceso_bloqueado_crear_mutex = puntero_a_proceso_bloqueado_crear_mutex->siguiente;
				}	
			
			if (encontrado == 0){	// el mutex solo estaba abierto por actual
				mutexid = p_proc_actual->mutex_abiertos_proceso[a].descriptor;
				mutex_sistema[mutexid].nombre = NULL;
				mutex_sistema[mutexid].tipo = 0;
				mutex_sistema[mutexid].valor = 0;
				
				if (lista_bloqueados_crear_mutex.primero != NULL){
				
					BCP * desbloqueado = lista_bloqueados_crear_mutex.primero;
					int nivel_anterior=fijar_nivel_int(NIVEL_3);
						eliminar_elem( &lista_bloqueados_crear_mutex, desbloqueado );
						insertar_ultimo(  &lista_listos, desbloqueado );	
					fijar_nivel_int(nivel_anterior);
				}

				
				
			}
		}
		encontrado = 0;
		a++;
	}
	

	/* FIN DE ELIMINAR MUTEX ABIERTOS */
	
	
	liberar_imagen(p_proc_actual->info_mem); /* liberar mapa */

	p_proc_actual->estado=TERMINADO;
	
	int nivel_anterior=fijar_nivel_int(NIVEL_3);

		eliminar_primero(&lista_listos); /* proc. fuera de listos */

	fijar_nivel_int(nivel_anterior);

		
	/* Realizar cambio de contexto */
	p_proc_anterior=p_proc_actual;
	p_proc_actual=planificador();

	printk("-> C.CONTEXTO POR FIN: de %d a %d\n",
			p_proc_anterior->id, p_proc_actual->id);

	liberar_pila(p_proc_anterior->pila);
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
        return; /* no debería llegar aqui */
}

/*
 *
 * Funciones relacionadas con el tratamiento de interrupciones
 *	excepciones: exc_arit exc_mem
 *	interrupciones de reloj: int_reloj
 *	interrupciones del terminal: int_terminal
 *	llamadas al sistemas: llam_sis
 *	interrupciones SW: int_sw
 *
 */

/*
 * Tratamiento de excepciones aritmeticas
 */
static void exc_arit(){

	if (!viene_de_modo_usuario())
		panico("excepcion aritmetica cuando estaba dentro del kernel");


	printk("-> EXCEPCION ARITMETICA EN PROC %d\n", p_proc_actual->id);
	liberar_proceso();

        return; /* no debería llegar aqui */
}

/*
 * Tratamiento de excepciones en el acceso a memoria
 */
static void exc_mem(){

	if (accediendo_en_memoria_tiempos_proceso==0)
		if (!viene_de_modo_usuario())
			panico("excepcion de memoria cuando estaba dentro del kernel");


	printk("-> EXCEPCION DE MEMORIA EN PROC %d\n", p_proc_actual->id);
	accediendo_en_memoria_tiempos_proceso = 0; //restaurar condicion
	liberar_proceso();

        return; /* no debería llegar aqui */
}

/*
 * Tratamiento de interrupciones de terminal
 */
static void int_terminal(){
	char car;

	car = leer_puerto(DIR_TERMINAL);
	printk("-> TRATANDO INT. DE TERMINAL %c\n", car);

        return;
}

/*
 * Tratamiento de interrupciones de reloj
 */
static void int_reloj(){

	printk("-> TRATANDO INT. DE RELOJ\n");
	
	// incrementar tiempos proceso
	if (lista_listos.primero!=NULL ){  //Hay procesos listos
		if (viene_de_modo_usuario())
			tiempos.usuario++;
		else
			tiempos.sistema++;
		
	} else
		tiempos.nulo++;
	

	// calcular si se ha despertado un proceso dormido
	BCP *puntero = lista_esperando.primero;
	BCP *siguiente;

	while ( puntero != NULL ){
	
		siguiente = puntero->siguiente;
		puntero->segundos --;
	
		if (puntero->segundos==0){
			//proceso despierta a listo
			puntero->estado=1;	//LISTO
			//reajustar listas
			eliminar_elem( &lista_esperando, puntero );
			insertar_ultimo(  &lista_listos, puntero );	
		}		
		puntero = siguiente;
	}
			
    return;
}

/*
 * Tratamiento de llamadas al sistema
 */
static void tratar_llamsis(){
	int nserv, res;

	nserv=leer_registro(0);
	if (nserv<NSERVICIOS)
		res=(tabla_servicios[nserv].fservicio)();
	else
		res=-1;		/* servicio no existente */
	escribir_registro(0,res);
	return;
}

/*
 * Tratamiento de interrupciuones software
 */
static void int_sw(){

	printk("-> TRATANDO INT. SW\n");
	
	
	return;
}

/*
 *
 * Funcion auxiliar que crea un proceso reservando sus recursos.
 * Usada por llamada crear_proceso.
 *
 */
static int crear_tarea(char *prog){
	void * imagen, *pc_inicial;
	int error=0;
	int proc;
	BCP *p_proc;

	proc=buscar_BCP_libre();
	if (proc==-1)
		return -1;	/* no hay entrada libre */

	/* A rellenar el BCP ... */
	p_proc=&(tabla_procs[proc]);

	/* crea la imagen de memoria leyendo ejecutable */
	imagen=crear_imagen(prog, &pc_inicial);
	if (imagen)
	{
		p_proc->info_mem=imagen;
		p_proc->pila=crear_pila(TAM_PILA);
		fijar_contexto_ini(p_proc->info_mem, p_proc->pila, TAM_PILA,
			pc_inicial,
			&(p_proc->contexto_regs));
		p_proc->id=proc;
		p_proc->estado=LISTO;
	
	/* lo inserta al final de cola de listos */
	int nivel_anterior=fijar_nivel_int(NIVEL_3);
		insertar_ultimo(&lista_listos, p_proc);
	fijar_nivel_int(nivel_anterior);

		error= 0;
	}
	else
		error= -1; /* fallo al crear imagen */

	return error;
}
/*
 *
 * Dormir
 *
 */


int dormir(unsigned int segundos){
	BCP * p_proc_dormido=NULL;

	if (segundos>0){
	
		int nivel_anterior=fijar_nivel_int(NIVEL_3);
	
			//inicializar contador
			p_proc_actual->segundos = segundos * TICK;
	
			//estado bloqueado
			p_proc_actual->estado=3;	//BLOQUEADO
	
			//reajustar listas
			eliminar_elem( &lista_listos, p_proc_actual );
			insertar_ultimo(  &lista_esperando  , p_proc_actual);
	
		fijar_nivel_int(nivel_anterior);			
			

			// nuevo proceso a ejecutar (el 2do de listos)
			p_proc_dormido=p_proc_actual;
			p_proc_actual = planificador();
			p_proc_actual->estado=2;	//EN EJECUCION

			//cambio de contexto 
			if (p_proc_actual != p_proc_dormido)
				cambio_contexto(&(p_proc_dormido->contexto_regs), &(p_proc_actual->contexto_regs));
				
	}
	
return 0;
}


/*
 *
 * Calculo tiempos proceso
 *
 */
int tiempos_proceso(struct tiempos_ejec *t_ejec){

	int interrupciones;

	int nivel_anterior=fijar_nivel_int(NIVEL_3);

		if (t_ejec==0){
	
			interrupciones = 0;
	
		}else{
	
			accediendo_en_memoria_tiempos_proceso = 1;
				t_ejec->usuario = (tiempos.usuario);
				t_ejec->sistema = (tiempos.sistema);
			accediendo_en_memoria_tiempos_proceso = 0;

			interrupciones = tiempos.usuario + tiempos.sistema + tiempos.nulo;
		}
	
	fijar_nivel_int(nivel_anterior);
	
	return interrupciones;
}

/*
 *
 * Funcion auxiliar crear mutex
 *
 */
int crear_mutex(char *nombre, int tipo){	
	int i=0;
	int j=0;
	
	/* buscamos el posicion libre en el conjunto de mutex abiertos del proceso */
	while ( j < NUM_MUT_PROC &&	p_proc_actual->mutex_abiertos_proceso[j].estado != NO_USADA )
		j++;
	
	// DESMASIADOS MUTEX ABIERTOS
	if ( j == NUM_MUT_PROC)
		return -1; //no hay espacio para abrir en el proceso
	
	// LA POSICION j ESTA LIBRE
	else{
	
	/* buscamos el primer descriptor libre del conjunto de mutex del sistema */
		while ( i < NUM_MUT && mutex_sistema[i].nombre != NULL ){
			i++;			
		}
		
		// EXCESO DE MUTEX CREADOS DEL SISTEMA
			if ( i == NUM_MUT){
			
				p_proc_actual->estado=3;	//BLOQUEADO
			
				BCP *p_proc_bloqueado = p_proc_actual;
			
				//reajustar listas
				int nivel_anterior=fijar_nivel_int(NIVEL_3);
					eliminar_elem( &lista_listos, p_proc_actual );
					insertar_ultimo(  &lista_bloqueados_crear_mutex, p_proc_actual );	
				fijar_nivel_int(nivel_anterior);
				
				p_proc_actual = planificador();
				p_proc_actual->estado=2;	//EN EJECUCION
				
				
				//cambio de contexto 
				cambio_contexto(&(p_proc_bloqueado->contexto_regs), &(p_proc_actual->contexto_regs));
				
				int res = crear_mutex(nombre, tipo);
				return res;
				
		// EL DESCRIPTOR i ESTÁ LIBRE
			}else{		
				//sabiendo que hay espacio para crear el mutex nuevo, ver si tiene el mismo nombre.
				int k = 0;
				while ( k < NUM_MUT ){
					if ( mutex_sistema[k].nombre == nombre )
						return -2; // error mutex con el mismo nombre
				k++;			
				}		
			
				mutex_sistema[i].nombre = nombre;	// metemos en conjunto de descriptores
				mutex_sistema[i].tipo = tipo;
				mutex_sistema[i].valor = 0;

				p_proc_actual->mutex_abiertos_proceso[j].estado = LISTO;	// metemos mutex_abierto en la posicion j-esima
				p_proc_actual->mutex_abiertos_proceso[j].descriptor = i;
				}
		}
	
		
return i;
}

/*
 *
 * Funcion auxiliar abrir mutex
 *
 */
int abrir_mutex(char *nombre){
	int i=0;
	int j=0;
	int res = 1; 
	
	/* buscamos el descriptor por el nombre indico por parametro en los mutex del sistema */
		while ( i < NUM_MUT && ( res==1 ) ){
			res = comparar_nombre(mutex_sistema[i].nombre, nombre);
			if (res != 0)
				i++;			
		}

	// EXCESO DE MUTEX CREADOS DEL SISTEMA
		if (i == NUM_MUT)
			return -1;
	// EL DESCRIPTOR i ESTA LIBRE
		else{
			
			/* buscamos una posicion libre en el conjunto de mutex abiertos del proceso */
			while ( j < NUM_MUT_PROC && 
					p_proc_actual->mutex_abiertos_proceso[j].estado != NO_USADA )
				j++;
		
		// DESMASIADOS MUTEX ABIERTOS
			if ( j == NUM_MUT_PROC)
				return -2; //
		// LA POSICION j ESTA LIBRE
			else{
				p_proc_actual->mutex_abiertos_proceso[j].estado = LISTO;	// metemos mutex_abierto en la posicion j-esima
				p_proc_actual->mutex_abiertos_proceso[j].descriptor = i;
			}
		}

	
	return i;
}

/*
 *
 * Funcion auxiliar lock
 *
 */
int lock(unsigned int mutexid){
	int i=0;
	int res=0;

	// comprobar si el proceso actual tiene abierto dicho mutex
	while (i < NUM_MUT_PROC && p_proc_actual->mutex_abiertos_proceso[i].descriptor != mutexid )
		i++;	
		
	if (i == NUM_MUT_PROC || p_proc_actual->mutex_abiertos_proceso[i].estado == NO_USADA)	
		return -1;
	

	if (mutex_sistema[mutexid].tipo == NO_RECURSIVO){
			// estamos en mutex NO_RECURSIVO
		if ( mutex_sistema[mutexid].valor == 1 ){

			if (p_proc_actual->id == mutex_sistema[mutexid].ultimo_proceso_lock)
				return -2;
					
			p_proc_actual->bloqueado_por_mutex = mutexid;
			
			//cambio de contexto
			p_proc_actual->estado=3;	//BLOQUEADO
			BCP *p_proc_bloqueado = p_proc_actual;
			
			//reajustar listas
			int nivel_anterior=fijar_nivel_int(NIVEL_3);
				eliminar_elem( &lista_listos, p_proc_actual );
				insertar_ultimo(  &lista_bloqueados_mutex, p_proc_actual );	
			fijar_nivel_int(nivel_anterior);

			p_proc_actual = planificador();
			p_proc_actual->estado=2;	//EN EJECUCION
			//cambio de contexto 
			cambio_contexto(&(p_proc_bloqueado->contexto_regs), &(p_proc_actual->contexto_regs));

			res = lock(mutexid);
			return res;
				
		}else // valor del mutex = 0
			mutex_sistema[mutexid].valor = 1;
			mutex_sistema[mutexid].ultimo_proceso_lock = p_proc_actual->id;

			
		
	}else	// estamos en mutex RECURSIVO
	
		if (mutex_sistema[mutexid].valor == 0){
		
			mutex_sistema[mutexid].valor++;
			mutex_sistema[mutexid].ultimo_proceso_lock	= 	p_proc_actual->id;
		
		}else {
		
			if (p_proc_actual->id == mutex_sistema[mutexid].ultimo_proceso_lock){
				mutex_sistema[mutexid].valor++;
			
			}else{
									
				p_proc_actual->bloqueado_por_mutex = mutexid;
				
				//cambio de contexto
				p_proc_actual->estado=3;	//BLOQUEADO
				BCP *p_proc_bloqueado = p_proc_actual;
				
				//reajustar listas
				int nivel_anterior=fijar_nivel_int(NIVEL_3);
					eliminar_elem( &lista_listos, p_proc_actual );
					insertar_ultimo(  &lista_bloqueados_mutex, p_proc_actual );	
				fijar_nivel_int(nivel_anterior);

				p_proc_actual = planificador();
				p_proc_actual->estado=2;	//EN EJECUCION
				//cambio de contexto 
				cambio_contexto(&(p_proc_bloqueado->contexto_regs), &(p_proc_actual->contexto_regs));

				res = lock(mutexid);
				return res;
			}
		
		}
		

		
	return 0;
}

/*
 *
 * Funcion auxiliar unlock
 *
 */
int unlock(unsigned int mutexid){
	int i=0;
	int encontrado = 0;
	
	// comprobar si el proceso actual tiene abierto dicho mutex
	while (i < NUM_MUT_PROC && 
			p_proc_actual->mutex_abiertos_proceso[i].descriptor != mutexid )
		i++;	

	if (i == NUM_MUT_PROC || p_proc_actual->mutex_abiertos_proceso[i].estado == NO_USADA)	
		return -1;
		

	if (mutex_sistema[mutexid].tipo == NO_RECURSIVO){
		// estamos en mutex NO_RECURSIVO
		if ( mutex_sistema[mutexid].valor == 0 )
			return -2;
		else{ 
			mutex_sistema[mutexid].valor = 0;
			mutex_sistema[mutexid].ultimo_proceso_lock = -1;

			if (lista_bloqueados_mutex.primero != NULL){
			
				BCP * puntero_a_proceso_bloqueado = lista_bloqueados_mutex.primero;
				while (puntero_a_proceso_bloqueado != NULL && encontrado){
				
					if (puntero_a_proceso_bloqueado->bloqueado_por_mutex == mutexid){
						encontrado = 1;
					}else{
						puntero_a_proceso_bloqueado = puntero_a_proceso_bloqueado->siguiente;
					}
				}
			
				int nivel_anterior=fijar_nivel_int(NIVEL_3);
					eliminar_elem( &lista_bloqueados_mutex, puntero_a_proceso_bloqueado );
					insertar_ultimo(  &lista_listos, puntero_a_proceso_bloqueado );	
				fijar_nivel_int(nivel_anterior);
			}
		}			
	
	}else{	// estamos en mutex RECURSIVO
	
		if (mutex_sistema[mutexid].valor == 0)
			return -3;
		else{
				 
			mutex_sistema[mutexid].valor--;
			
			if (mutex_sistema[mutexid].valor == 0){
			
				mutex_sistema[mutexid].ultimo_proceso_lock = -1;

				// buscar si algun proceso esta bloqueado por hacer lock a este mutex recien liberado
				if (lista_bloqueados_mutex.primero != NULL){
			
					BCP * puntero_a_proceso_bloqueado = lista_bloqueados_mutex.primero;
					while (puntero_a_proceso_bloqueado != NULL && encontrado){
				
						if (puntero_a_proceso_bloqueado->bloqueado_por_mutex == mutexid){
							encontrado = 1;
						}else{
							puntero_a_proceso_bloqueado = puntero_a_proceso_bloqueado->siguiente;
						}
					}
			
					int nivel_anterior=fijar_nivel_int(NIVEL_3);
						eliminar_elem( &lista_bloqueados_mutex, puntero_a_proceso_bloqueado );
						insertar_ultimo(  &lista_listos, puntero_a_proceso_bloqueado );	
					fijar_nivel_int(nivel_anterior);
				}
			}
		
		}	
	}
		
	return 0;
}

/*
 *
 * Funcion auxiliar cerrar_mutex
 *
 */
int cerrar_mutex( unsigned int mutexid ){
	int i=0;
	int j=0;
	int encontrado=0;
	

	
	// cerrar mutex del proceso actual
	while (p_proc_actual->mutex_abiertos_proceso[i].descriptor != mutexid){
		i++;
	}
		// no estaba abierto por el proceso actual
		if ( i == NUM_MUT_PROC )
			return -1;
		else
			p_proc_actual->mutex_abiertos_proceso[i].estado = NO_USADA;

				
	// comprobar si hay que eliminar en el mutex_sistema -> 
	// cuando ningun otro proceso lo tiene abierto	
	BCP * puntero_a_proceso_listo = lista_listos.primero->siguiente;
	while (puntero_a_proceso_listo != NULL && encontrado == 0){
		j=0;
		while ( j < NUM_MUT_PROC && encontrado == 0){
		
			if (puntero_a_proceso_listo->mutex_abiertos_proceso[j].descriptor == mutexid)
				encontrado = 1;
			j++;
		}
		puntero_a_proceso_listo = puntero_a_proceso_listo->siguiente;
	}

	BCP * puntero_a_proceso_dormido = lista_esperando.primero;
	while (puntero_a_proceso_dormido != NULL && encontrado == 0){
		j=0;
		while ( j < NUM_MUT_PROC && encontrado == 0){
			if (puntero_a_proceso_dormido->mutex_abiertos_proceso[j].descriptor == mutexid)
				encontrado = 1;
			j++;
		}
		puntero_a_proceso_dormido = puntero_a_proceso_dormido->siguiente;
	}	
	
	
	
	BCP * puntero_a_proceso_bloqueado = lista_bloqueados_mutex.primero;
	while (puntero_a_proceso_bloqueado != NULL)	{
				
		if (mutexid == puntero_a_proceso_bloqueado->bloqueado_por_mutex){
				
			int nivel_anterior=fijar_nivel_int(NIVEL_3);
				eliminar_elem( &lista_bloqueados_mutex, puntero_a_proceso_bloqueado );
				insertar_ultimo(  &lista_listos, puntero_a_proceso_bloqueado );	
			fijar_nivel_int(nivel_anterior);
			encontrado = 1;			
			mutex_sistema[mutexid].valor = 0;
				
		}
				
		puntero_a_proceso_bloqueado = puntero_a_proceso_bloqueado->siguiente;

	}
	
	
	BCP * puntero_a_proceso_bloqueado_crear	= lista_bloqueados_crear_mutex.primero;
	while (puntero_a_proceso_bloqueado_crear != NULL && encontrado == 0){
		j=0;		
		while ( j < NUM_MUT_PROC && encontrado == 0){
			if (puntero_a_proceso_bloqueado_crear->mutex_abiertos_proceso[j].descriptor == mutexid)
				encontrado = 1;
			j++;
		}
		puntero_a_proceso_bloqueado_crear = puntero_a_proceso_bloqueado_crear->siguiente;
	}	
	
	
		// el mutex solo estaba abierto por el proceso actual ->
		// eliminar de mutex_sistema
		if (encontrado == 0){
			mutex_sistema[mutexid].nombre = NULL;
			mutex_sistema[mutexid].valor = 0;
						
			if (lista_bloqueados_crear_mutex.primero != NULL){
		
				BCP *desbloqueado = lista_bloqueados_crear_mutex.primero;
		
				int nivel_anterior=fijar_nivel_int(NIVEL_3);
					eliminar_elem( &lista_bloqueados_crear_mutex, desbloqueado );
					insertar_ultimo(  &lista_listos, desbloqueado );	
				fijar_nivel_int(nivel_anterior);
			}
			
		}
	
 return 0;
}



/*
 *
 * Rutinas que llevan a cabo las llamadas al sistema
 *	sis_crear_proceso sis_escribir
 *
 */

/*
 * Tratamiento de llamada al sistema crear_proceso. Llama a la
 * funcion auxiliar crear_tarea sis_terminar_proceso
 */
int sis_crear_proceso(){
	char *prog;
	int res;

		printk("-> PROC %d: CREAR PROCESO\n", p_proc_actual->id);
		prog=(char *)leer_registro(1);
		res=crear_tarea(prog);
		
	return res;
}

/*
 * Tratamiento de llamada al sistema escribir. Llama simplemente a la
 * funcion de apoyo escribir_ker
 */
int sis_escribir()
{
	char *texto;
	unsigned int longi;

	texto=(char *)leer_registro(1);
	longi=(unsigned int)leer_registro(2);

	escribir_ker(texto, longi);
	return 0;
}

/*
 * Tratamiento de llamada al sistema terminar_proceso. Llama a la
 * funcion auxiliar liberar_proceso
 */
int sis_terminar_proceso(){

	printk("-> FIN PROCESO %d\n", p_proc_actual->id);

	liberar_proceso();

        return 0; /* no debería llegar aqui */
}

int sis_dormir(){
	unsigned int segundos;
	int res;
	
//	printk("-> PROC %d: DORMIR \n", p_proc_actual->id);
	segundos=leer_registro(1);
	res=dormir(segundos);
	return res;
}
/*
 *
 * Obtener id del proceso actual
 *
 */
int obtener_id_pr(){
//	printk("-> PROC %d: OBTENER ID \n", p_proc_actual->id);

return p_proc_actual->id;
}

/*
 *
 * Tratamiento de la llamada de sistema tiempos_proceso
 *
 */
int sis_tiempos_proceso(){
	struct tiempos_ejec *t_ejec;
	int res;

		t_ejec = (struct tiempos_ejec *)leer_registro(1);
		res = tiempos_proceso(t_ejec);
		return res;
}

/*
 *
 * Tratamiento de la llamada de sistema crear_mutex
 *
 */
int sis_crear_mutex(){
	char *nombre;
	int tipo, res;

	nombre=(char *)leer_registro(1);
	tipo=(int)leer_registro(2);
	res = crear_mutex(nombre, tipo);
		
	return res;
}

/*
 *
 * Tratamiento de la llamada de sistema abrir_mutex
 *
 */
int sis_abrir_mutex(){
	char *nombre;
	int res;

	nombre=(char *)leer_registro(1);
	res = abrir_mutex(nombre);
		
	return res;
}

/*
 *
 * Tratamiento de la llamada de sistema lock
 *
 */
int sis_lock(){
	unsigned int mutexid;
	int res;

	mutexid=leer_registro(1);
	res = lock(mutexid);
		
	return res;
}

/*
 *
 * Tratamiento de la llamada de sistema unlock
 *
 */
int sis_unlock(){
	unsigned int mutexid;
	int res;

	mutexid=leer_registro(1);
	res = unlock(mutexid);
		
	return res;
}

/*
 *
 * Tratamiento de la llamada de sistema cerrar_mutex
 *
 */
int sis_cerrar_mutex(){
	unsigned int mutexid;
	int res;

	mutexid=leer_registro(1);
	res = cerrar_mutex(mutexid);
		
	return res;
}


/*
 *
 * Rutina de inicialización invocada en arranque
 *
 */
int main(){
	/* se llega con las interrupciones prohibidas */

	instal_man_int(EXC_ARITM, exc_arit); 
	instal_man_int(EXC_MEM, exc_mem); 
	instal_man_int(INT_RELOJ, int_reloj); 
	instal_man_int(INT_TERMINAL, int_terminal); 
	instal_man_int(LLAM_SIS, tratar_llamsis); 
	instal_man_int(INT_SW, int_sw); 

	iniciar_cont_int();		/* inicia cont. interr. */
	iniciar_cont_reloj(TICK);	/* fija frecuencia del reloj */
	iniciar_cont_teclado();		/* inici cont. teclado */

	iniciar_tabla_proc();		/* inicia BCPs de tabla de procesos */
	
	
	/* crea proceso inicial */
	if (crear_tarea((void *)"init")<0)
		panico("no encontrado el proceso inicial");
	

	/* activa proceso inicial */
	p_proc_actual=planificador();
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
	panico("S.O. reactivado inesperadamente");
	return 0;
}
