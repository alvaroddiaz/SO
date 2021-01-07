/*Álvaro Díaz Díaz  DNI 34288483Z*/
/*Lucas Ledo Alonso  DNI 21050228F*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/resource.h>


#include "list.h"
#include "lproc.h"

#define TAMANO 2048

#define MAX 40 

#define MAXLINE 2048
#define MAXNAME 1024


#define MAXARRAY 4096

typedef char* lista [MAXARRAY];


void procesarEntrada(char *c,char *trozos[], listaP *lproc);


char LetraTF (mode_t m) {
	switch (m&S_IFMT) { /*and bit a bit con los bits de formato,0170000 */
		case S_IFSOCK: return 's'; /*socket */
		case S_IFLNK: return 'l'; /*symbolic link*/
		case S_IFREG: return '-'; /* fichero normal*/
		case S_IFBLK: return 'b'; /*block device*/
		case S_IFDIR: return 'd'; /*directorio */
		case S_IFCHR: return 'c'; /*char device*/
		case S_IFIFO: return 'p'; /*pipe*/
		default: return '?'; /*desconocido, no deberia aparecer*/
	}
}

char * ConvierteModo2 (mode_t m){
	static char permisos[12];
	strcpy (permisos,"---------- ");
	permisos[0]=LetraTF(m);
	if (m&S_IRUSR) permisos[1]='r'; /*propietario*/
	if (m&S_IWUSR) permisos[2]='w';
	if (m&S_IXUSR) permisos[3]='x';
	if (m&S_IRGRP) permisos[4]='r'; /*grupo*/
	if (m&S_IWGRP) permisos[5]='w';
	if (m&S_IXGRP) permisos[6]='x';
	if (m&S_IROTH) permisos[7]='r'; /*resto*/
	if (m&S_IWOTH) permisos[8]='w';
	if (m&S_IXOTH) permisos[9]='x';
	if (m&S_ISUID) permisos[3]='s'; /*setuid, setgid y stickybit*/
	if (m&S_ISGID) permisos[6]='s';
	if (m&S_ISVTX) permisos[9]='t';
	return (permisos);
}

struct tm * GetFecha (){
	time_t t;
	struct tm *tm;

	t=time(0);
	tm=localtime(&t);
	if (t && tm){
		return tm;
	}else {
		perror("error, time couldn`t be collected");
		return NULL;
	}
}


void crearlista (lista l){
    int i;
    for (i=0;i<MAXARRAY;i++){
        l[i]=NULL;
    }
}

void insertarenLista (char * p,lista l){
    int i=0;
    while (l[i]!=NULL){
        i++;
    }
    l[i] = p;
}

void cleanList(char *l [MAXARRAY]){
    int i=0;
    while (l[i]!=NULL){
        free (l[i]);
        l[i]=NULL;
        i++;
    }
}

void imprimirLista(lista l){
    int i=0;
    while (l[i]!=NULL){
        printf("%s",l[i]);
        i=i+1;
    }
}

void removeElement (lista l, int pos){
    void * tmp;
    tmp = l[pos];
    int i = pos;
    while (l[i+1]!=NULL){
        i++;
    }
    l[pos] = l[i];
    free(tmp);
    l[i]=NULL;
}

void* getElement (lista l, int pos){
    return &l[pos];
}

void printMalloc (lista2 l){
	int i=0;
	while (l[i]!=NULL){
		if (!strcmp((((struct node*) l[i])->type),"malloc")) {
			printf("%p: size:%lu. malloc %s\n",((struct node *)l[i])->p, ((struct node *)l[i])->tam, ((struct node *)l[i])->t);
		}
		i=i+1;
	}
}

void printMmap (lista2 l){
	int i=0;
	while (l[i]!=NULL){
		if (!strcmp((((struct node *)l[i])->type),"mmap")  ) {
			printf("%p: size:%lu. mmap %s (fd:%d) %s",((struct node *)l[i])->p, ((struct node *)l[i])->tam,((struct node *)l[i])->name,
			((struct node *)l[i])->df, ((struct node *)l[i])->t);
		}
		i=i+1;
	}
}

void printShared (lista2 l){
	int i=0;
	while (l[i]!=NULL){
		if (!strcmp((((struct node *)l[i])->type),"shared memory")){
			printf("%p: size:%lu. shared memory (key %d) %s",((struct node *)l[i])->p, ((struct node *)l[i])->tam,((struct node *)l[i])->key,
			((struct node *)l[i])->t);
		}
		i++;
	}
}
void imprimirLista2(lista2 l2){
	printMalloc(l2);
	printMmap(l2);
	printShared(l2);
}



lista l ;
lista2 l2;


int TrocearCadena(char *cadena, char *trozos[]){ 
    int i=1;
    if ((trozos[0]=strtok(cadena," \n\t"))==NULL){
        return 0;
    }
    while ((trozos[i]=strtok(NULL," \n\t"))!=NULL){
        i++;
    }
    return i;
}

void authors(char *trozos[]){
    
    if(trozos[1] != NULL){
        if(strcmp(trozos[1],"-l")==0){
            printf("alvaro.ddiaz@udc.es\n");
            printf("lucas.ledo@udc.es\n");
        }
        else if(strcmp(trozos[1],"-n")==0){
            printf("Alvaro Diaz Diaz\n");
            printf("Lucas Ledo Alonso\n");
        }
        else{
            printf("Comando: '%s %s' no valido\n", trozos[0],trozos[1]);
        }
    }
    else{
        printf("Alvaro Diaz Diaz -> alvaro.ddiaz@udc.es,\n");
        printf("Lucas Ledo Alonso -> lucas.ledo@udc.es\n"); 
    }
}

void pid(){

    printf("Pid de shell %d\n",getpid());
}

void ppid(){

    printf("Pid de padre %d\n",getppid());
}

void pwd(){
    char directorio[256];
    
    if (getcwd(directorio,sizeof(directorio)) !=NULL){
        printf("%s\n",directorio);
    }
    else{
        perror("Error");
    }
}

void cdir(char *trozos[]){

    if(trozos[1] != NULL){
        if(chdir(trozos[1])==-1){
            perror("Imposible cambiar directorio");
        }
    }
    else{
        pwd();
    }
}

void date(){
    time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	printf("%02d/%02d/%04d\n",tm.tm_mday,tm.tm_mon+1,tm.tm_year+1900);

}


void hora(){
    time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	printf("%02d:%02d:%02d\n",tm.tm_hour,tm.tm_min,tm.tm_sec);

}

void historic(char *trozos[], listaP *lproc){
    int n,i;
    if(trozos[1] != NULL){
        if(strcmp(trozos[1],"-c")==0){
            cleanList(l);
        }
        else if(strncmp(trozos[1],"-r",2)==0){//Lo mismo que arriba
            //Ejecutar el comando en la posicion N
            //Coges el comando en esa posicion, seguramente procesarEntrada
            trozos[1]+=2;
            i = atoi(trozos[1]);
			i--;
            trozos[1]-=2;
            if (i != 0){
                if(l[i]!=NULL){
                    procesarEntrada(l[i],trozos, lproc);
					
                }
                else{
                    printf("No hay elemento %d en el historico",i);
                }
                
            }
            else{
                printf("Comando: '%s %s' no valido\n", trozos[0],trozos[1]);
            } 
        }
        else if(strncmp(trozos[1],"-",1)==0){
            
            n = atoi(++trozos[1]);
            trozos[1]--;
            if (n != 0){
                for (i=0;i<n ; i++){
                    if(l[i] != NULL){
                        printf("%s",l[i]);
                    }
                    else{
                        printf("No hay elemento %d en el historico\n",i);
                    }
                    
                }
            }
            else{
                printf("Comando: '%s %s' no valido\n", trozos[0],trozos[1]);
            } 
        }
        else{
            printf("Comando: '%s %s' no valido\n", trozos[0],trozos[1]);
        }
    }
    else{
    	imprimirLista(l);
	}    
}


void infoAux(char *path) {
	struct stat buf;
	char slink[MAX];
	
	if (lstat(path,&buf)==-1) { //comprobamos que funcione lstat
		perror("****error al acceder ");
	}
	else { //listamos la informacion
	printf(" %ld ",buf.st_ino);
	printf("%10s ",ConvierteModo2(buf.st_mode));
	printf("%ld ",buf.st_nlink);
	printf("%s ",getpwuid(buf.st_uid)->pw_name);
	printf("%s ",getgrgid(buf.st_gid)->gr_name);
	printf("%7ld ",buf.st_size);
	printf("%s ", strtok(asctime(localtime(&buf.st_mtime)), "\n"));
		
		if (LetraTF(buf.st_mode) == 'l') { //si es un link, lo muestra y a donde apunta
			readlink(path,slink,buf.st_size);
			printf("%s -> %s \n",path,slink);
		}
		else {
			printf("%s\n",path);
		}
	}
}


void listAux(char *path,bool op_long,bool op_dir,bool op_hid,bool op_rec){
	struct stat buf;
	struct dirent *dir;
	DIR *d;
	char *cola[MAX];
	char aux[MAX] = "";
	int i = 0;
	
	d = opendir(path);
	if (d!= NULL) {
		printf("******** %s\n",path);
		while ((dir = readdir(d)) != NULL) {
			strcpy(aux,path);
			strcat(aux,"/");
			strcat(aux,dir->d_name);
			lstat(aux,&buf);
			if((op_hid == true) || ((strcmp(dir->d_name,".")!=0) && (strcmp(dir->d_name,"..")!=0)) ) {
                if((LetraTF(buf.st_mode) != 'd') || (op_dir == false)){
				    if (op_long == true ) {	// si queremos listar con su informacion
					    infoAux(aux);
				    }
				    else {
					    printf("%s %ld\n",dir->d_name,buf.st_size);
                    }
				}
				if ((op_rec == true)&&(strcmp(".",dir->d_name)!=0) && (strcmp("..",dir->d_name)!=0) && (LetraTF(buf.st_mode) == 'd')){
					cola[i]= dir->d_name; //si queremos listar recursivamente añadimos los directorios a una cola
					i++;
				}
			}
		}
		for (int j=0; j < i; j++) { //procesamos los directorios de la cola
			strcpy(aux,path);
			strcat(aux,"/");
			strcat(aux,cola[j]);
			listAux(aux,op_long,op_dir,op_hid,op_rec);
		}
	}
	else { //si no hemos podido abrir el directorio
		perror("Error al listar directorio ");
	}
	closedir(d);
}


void create(char * trozos[]) { 
	if (trozos[1]!= NULL) { 
		if (strcmp(trozos[1], "-dir")==0){  //si la opcion es -dir
			if (trozos[2]!=NULL){
				if (mkdir(trozos[2], 0777)==-1){ //comprobamos que se ha creado el directorio
					perror(trozos[2]);
				}
			}
			else{ //si no hay mas argumentos lista
				listAux(".",false,true,false,false);
			}
		}
		else { //si no hay '-dir' creamos un archivo y comprobamos su creacion
			if(open(trozos[1], O_CREAT|O_EXCL, 0777)==-1){
				perror(trozos[1]);
			}
		}
	}
	else{ //si no hay mas argumentos lista
		listAux(".",false,false,false,false);
	}
}

void deleteAux(char *path) {
	struct dirent *dir;
	DIR *d;
	char aux[MAX] = "";
	d = opendir(path);
	if (d!= NULL) {
		while((dir = readdir(d)) != NULL) {
			strcpy(aux, path);
			strcat(aux,"/");
			strcat(aux,dir->d_name);
			if ((dir->d_type == DT_DIR) && (strcmp(dir->d_name,".")!=0) 
			 && (strcmp(dir->d_name,"..")!=0)){ //si es un directorio borramos recursivamente
				deleteAux(aux);
				if (remove(aux)==-1) {//comprobamos si se ha borrado
					perror(aux);
				}
			}
			else if ((dir->d_type == DT_REG) || (dir->d_type == DT_LNK)){ //si es un archivo o un link simbolico borramos directamente
				if (remove(aux)==-1){ //comprobamos borrado
					perror(aux);
				}
			}
		}
		if (remove(path)==-1){ //comprobamos borrado
			perror(path);
		}
	}
	else {
		perror("Imposible Borrar"); //si no hemos podido abrir el directorio
	}//Al borrar recursivamente un directorio con un directorio vacío dentro imprime un error de que no lo ha encontrado porque intenta borrar el contenido de la carpeta ya borrada (No sabemos arreglarlo) 
	closedir(d);
}

void delete (char *trozos[]) {
	if (trozos[1]!= NULL) { 
		if (strcmp(trozos[1], "-rec")==0){ //si la opcion es -rec
			if (trozos[2]!=NULL){ 
				deleteAux(trozos[2]); //borramos recursivamente
			}
			else {
				listAux(".",false,false,false,true); //si no hay mas argumentos lista
			}
		}
		else { // si no hay -rec
			if(remove(trozos[1])==-1) { //comprobamos el borrado
				perror("Imposible Borrar");
			}
		}
	}
	else{ //si no hay mas argumentos lista
		listAux(".",false,false,false,false);
	}
}

//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************


void list(char *trozos[],int n){
	bool op_long = false;
    bool op_dir = false;
	bool op_hid = false;
	bool op_rec = false;
	int comienzo = 1;
	
	if (trozos[1]!= NULL) {
		for (int i=1; i<n ; i++) { //vemos que opciones hay
			if (strcmp(trozos[i], "-long")==0) {
				op_long = true;
				comienzo++;
			}
            else if (strcmp(trozos[i], "-dir")==0) {
				op_dir = true;
				comienzo++;
            }
			else if (strcmp(trozos[i], "-hid")==0) {
				op_hid = true;
				comienzo++;
			}
			else if (strcmp(trozos[i], "-rec")==0) {
				op_rec = true;
				comienzo++;
			}
			else {
				break;
			}
		}
	}
	if ( n == comienzo) { // si solo hay listar (sin opciones)
		listAux(".",op_long,op_dir,op_hid,op_rec);
	}
	else {
		for (int i=comienzo; i<n ; i++) { //listamos segun las opciones incluidas
			listAux(trozos[i],op_long,op_dir,op_hid,op_rec);
		}
	}
}
void doRecursiva (int n){
	char automatico[TAMANO];
	static char estatico[TAMANO];
	printf ("parametro n:%d en %p\n",n,&n);
	printf ("array estatico en:%p \n",estatico);
	printf ("array automatico en %p\n",automatico);
	n--;
	if (n>0)
		doRecursiva(n);
}


void * ObtenerMemoriaShmget (key_t clave, size_t tam,lista2 l2){
	void * p;
	int aux,id,flags=0777;
	struct shmid_ds s;
	void* aux2=NULL;
	if ((aux2=malloc (sizeof(struct node)))==NULL){
		perror("Failure allocating node mmap");
		return 0;
	}


	if (tam) //si tam no es 0 la crea en modo exclusivo 
		flags=flags | IPC_CREAT | IPC_EXCL;
			//si tam es 0 intenta acceder a una ya creada
	if (clave==IPC_PRIVATE) //no nos vale
		{errno=EINVAL; return NULL;}
		
	if ((id=shmget(clave, tam, flags))==-1)
		return (NULL);
		
	if ((p=shmat(id,NULL,0))==(void*) -1){
		aux=errno; //si se ha creado y no se puede mapear
		if (tam) //se borra 
			shmctl(id,IPC_RMID,NULL);
		errno=aux;
		return (NULL);
		}
	shmctl (id,IPC_STAT,&s);
	((struct node *)aux2)->p = p;
	((struct node *)aux2)->tam = s.shm_segsz;
	if ((((struct node *)aux2)->t = malloc(200))==NULL){
		perror("Failure allocating shared node");
		return 0;
	}
	strftime(((struct node *)aux2)->t,200,"%a %b %d %H:%M:%S %Y \n",GetFecha());
	((struct node *)aux2)->type = "shared memory";
	((struct node *)aux2)->key = clave;
	insertarenLista2(l2,aux2);
	return (p);
}
void Cmd_AlocateCreateShared (char *arg[],lista2 l2) /*arg[0] is the keyand arg[1] is the size*/{
	key_t k;
	size_t tam=0;
	void *p;
	if (arg[3]==NULL || arg[4]==NULL){
		printShared(l2);
		return;
		}
	k=(key_t) atoi(arg[3]);
	if (arg[4]!=NULL)
		tam=(size_t) atoll(arg[4]);
	if ((p=ObtenerMemoriaShmget(k,tam,l2))==NULL)
		perror ("Imposible obtener memoria shmget");
	else
		printf ("Memoria de shmget de clave %d asignada en %p\n",k,p);
}
/************************************************************************/
/************************************************************************/
void * MmapFichero (char * fichero, int protection){
	int df, map=MAP_PRIVATE,modo=O_RDONLY;
	struct stat s;
	void *p;
	void* aux;
	if ((aux=malloc (sizeof(struct node)))==NULL){
		perror("Failure allocating node mmap");
		return 0;
	}

	if (protection&PROT_WRITE)  modo=O_RDWR;
	if (stat(fichero,&s)==-1 || (df=open(fichero, modo))==-1)
		return NULL;
	if ((p=mmap (NULL,s.st_size, protection,map,df,0))==MAP_FAILED)
		return NULL;


	((struct node *)aux)->p = p;
	((struct node *)aux)->tam =s.st_size;
	((struct node *)aux)->df = df;
	if ((((struct node *)aux)->t = malloc(200))==NULL){
	perror("Failure allocating shared node");
	return 0;
	}
	strftime(((struct node *)aux)->t,200,"%a %b %d %H:%M:%S %Y \n",GetFecha());
	if ((((struct node *)aux)->name = malloc(sizeof(fichero)))==NULL){
	perror("Failure allocating shared node");
	return 0;
	}
	strcpy(((struct node *)aux)->name,fichero);
	if ((((struct node *)aux)->type = malloc(sizeof("mmap")))==NULL){
		perror("Failure allocating node mmap");
		return 0;
	}
	strcpy(((struct node *)aux)->type,"mmap");
	insertarenLista2(l2,aux);
	return p;
}
void Cmd_AllocateMmap (char *arg[],lista2 l2) /*arg[0] is the file nameand arg[1] is the permissions*/{
	char *perm;
	void *p;
	int protection=0;
	if (arg[3]==NULL){
		printMmap(l2);
		return;
		 }
	if ((perm=arg[3])!=NULL && strlen(perm)<4) {
		if (strchr(perm,'r')!=NULL) protection|=PROT_READ;
		if (strchr(perm,'w')!=NULL) protection|=PROT_WRITE;
		if (strchr(perm,'x')!=NULL) protection|=PROT_EXEC;
	}
	if ((p=MmapFichero(arg[3],protection))==NULL)
		perror ("Imposible mapear fichero");
	else
		printf ("fichero %s mapeado en %p\n", arg[3], p);
}

void Cmd_AllocateShared (char *arg[],lista2 l2){
	key_t k;
	size_t tam=0;
	void *p;
	
	if (arg[3]==NULL ){
		printShared(l2); 
		return;
	}
	k=(key_t) atoi(arg[3]);
	if ((p=ObtenerMemoriaShmget(k,tam,l2))==NULL)
		perror ("Imposible obtener memoria shmget");
	else
		printf ("Memoria de shmget de clave %d asignada en %p\n",k,p);
}


/*********************************************************************/
/*********************************************************************/
void Cmd_deletekey (char *args[]) /*arg[0] points to a str containing the key*/{
	key_t clave;
	int id;
	char *key=args[2];
	if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE){
		printf ("   rmkey   clave_valida\n");
		return;
	}
	if ((id=shmget(clave,0,0666))==-1){
		perror ("shmget: imposible obtener memoria compartida");
		return;
	}
	if (shmctl(id,IPC_RMID,NULL)==-1)
		perror ("shmctl: imposible eliminar memoria compartida\n");
}
void Cmd_dopmap (char *args[]) /*no arguments necessary*/{
	pid_t pid;
	char elpid[32];
	char *argv[3]={"pmap",elpid,NULL};
	sprintf (elpid,"%d", (int) getpid());
	if ((pid=fork())==-1)
		{perror ("Imposible crear proceso");
		return;
	}
	if (pid==0){
		if (execvp(argv[0],argv)==-1)
			perror("cannot execute pmap");
		exit(1);
	}
	waitpid (pid,NULL,0);
}

void* allocateMalloc(char* trozos[], lista2 l2){
	void* p;
	void* aux;
	if ((aux=malloc (sizeof(struct node)))==NULL){
		perror("Failure allocating node mmap");
		return 0;
	}
	
	if(trozos[3] != NULL){
		if(atol(trozos[3]) < 1)
			printf("Invalid argument [tam]\n");
		else{
			p = malloc(atoi(trozos[3]));
			if(p == NULL)
				perror("");
			else{
				printf("allocated %d at %p\n", atoi(trozos[3]), p);
				((struct node *)aux)->p = p;
				((struct node *)aux)->tam = atoi(trozos[3]);
				if ((((struct node *)aux)->t = malloc(200))==NULL){
					perror("Failure allocating shared node");
					return 0;
				}
				strftime(((struct node *)aux)->t,200,"%a %b %d %H:%M:%S %Y \n",GetFecha());
				((struct node *)aux)->name = "";
				((struct node *)aux)->type = "malloc";
				insertarenLista2(l2,aux);
			}
		}
	}else {
		printMalloc(l2);
	}
	return 0;
}

void* allocate(char* trozos[], lista2 l2){
	if (trozos[2]!= NULL) {
		if (strcmp(trozos[2], "-malloc")==0){
			allocateMalloc(trozos,l2);
			
		}
		else if (strcmp(trozos[2], "-mmap")==0){
			Cmd_AllocateMmap(trozos,l2);
		}
		else if (strcmp(trozos[2], "-createshared")==0){
			Cmd_AlocateCreateShared(trozos,l2);
		}
		else if (strcmp(trozos[2], "-shared")==0){
			Cmd_AllocateShared(trozos,l2);
		}
	}
	else {
		imprimirLista2(l2);
	}
	return 0;
}
void dshared (lista2 l,int i){
	if(!shmdt(((struct node*)l[i])->p))
		printf("block at address %p deallocated (shared)\n",((((struct node *)l[i])->p)));
	else
	perror("Block could not be deallocated");
	free(((((struct node *)l[i])->t)));
	

}
void deallocShared (char *arg[],lista2 l2){
	key_t k;
	int i=0;
	int done =0;
	
	if(arg[3]==NULL){
		printShared(l2);
		return;
	}else{
		k=(key_t) atoi(arg[3]);
		while((l2[i]!=NULL)&&(done==0)){
			if(((!strcmp((((struct node *)l2[i])->type),"shared memory")) && ((((struct node *)l2[i])->key)==k))){
				done =1;
				dshared(l2,i);
				eliminarElemento2(l2,i);
			}
			i++;
	}
	if(!done)
		printShared(l2);
	}
}	

void dmmap(lista2 l2,int i) {
	printf("block at address %p deallocated (mmap)\n",((((struct node *)l2[i])->p)) );
	if ( ( munmap( ((((struct node *)l2[i])->p)) , ((((struct node *)l2[i])->tam)) ) ) == -1 )
		perror("Error unmapping: ");
	if (close( ((((struct node *)l2[i])->df)) ) == -1)
		perror("Error closing: ");
	free(((((struct node *)l2[i])->t)));
	free(((((struct node *)l2[i])->name)));
	
}
void deallocMmap (char *arg[],lista2 l2){
	int done = 0;
	int i=0;
	if (arg[3]==NULL)
		{printMmap(l2);
			return;}
	else {
		while ((l[i]!=NULL) && (done == 0)){
			if ((!strcmp((((struct node *)l2[i])->type),"mmap") && (!strcmp((((struct node *)l2[i])->name),arg[3])) ))  {
				done = 1;
				dmmap(l2,i);
				eliminarElemento2(l2,i);
			}
		i = i + 1;
		}
	}
	if (!done){
		printMmap(l2);
	} 
}

void dmalloc(size_t tam,lista2 l2,int i) {
	printf("deallocated %lu at %p\n", tam,((((struct node *)l2[i])->p)) );
	free(((((struct node *)l2[i])->p)));
	free(((((struct node *)l2[i])->t)));
}

void deallocmalloc(char * arg[],lista2 l2){
	int done = 0;
	int i =0;
	int tam;
	if (arg[3]==NULL){
			printMalloc(l2);
			return;
	}else {
		tam = atoi(arg[3]);
		while ((l[i]!=NULL) && (done == 0)){
			if ((!strcmp((((struct node *)l2[i])->type),"malloc")) && ((((struct node *)l2[i])->tam == tam ))) {
				done = 1;
				dmalloc(tam,l2,i);
				eliminarElemento2(l2,i);
			}
			i = i + 1;
		}
	}
	if (!done)
		printMalloc(l2);
}
void deallocAddress(char * arg[],lista2 l2){
	int i = 0;
	int done = 0;
	void * p;
	sscanf(arg[2],"%p",&p);
	while ((l[i]!=NULL) && (done == 0)){
		if (((struct node *)l2[i])->p == p) {
			done = 1;
			if (!strcmp(((struct node *)l2[i])->type,"malloc")) {
				dmalloc( ((struct node *)l2[i])->tam ,l2,i);
			}else if (!strcmp((((struct node *)l2[i])->type),"mmap") )
				dmmap(l2,i);
			else if (!strcmp((((struct node *)l2[i])->type),"shared")){
				dshared(l2,i);
			}	
			eliminarElemento2(l2,i);
		}
		i++;
	}
	if (!done)
		imprimirLista2(l2);
}


void * dealloc(char* trozos[], lista2 l2){
	if (trozos[2]!= NULL) {
		if (strcmp(trozos[2], "-malloc")==0){
			deallocmalloc(trozos,l2);
			
		}
		else if (strcmp(trozos[2], "-mmap")==0){
			deallocMmap(trozos,l2);
		}
		else if (strcmp(trozos[2], "-shared")==0){
			deallocShared(trozos,l2);
		}
		else {
			deallocAddress(trozos,l2);

		}
	}
	else {
		imprimirLista2(l2);
	}
	return 0;
}

void show(char* trozos[], lista2 l2,int n){
	char caracter;
	float decimal;
	int i=2;
	if (trozos[i]!= NULL) {
		while(trozos[i]!=NULL){
			if (strcmp(trozos[i], "-malloc")==0){
				printMalloc(l2);
			}
			else if (strcmp(trozos[i], "-mmap")==0){
				printMmap(l2);
			}
			else if (strcmp(trozos[i], "-shared")==0){
				printShared(l2);
			}
			else if (strcmp(trozos[i], "-all")==0){
				imprimirLista2(l2);
			}
			i++;
		}
	}
	else {
	printf("Funciones\n");
	printf(" Direccion de Cmd_deletekey: %p \n",&Cmd_deletekey);
	printf(" Direccion de dealloc: %p \n",&dealloc);
	printf(" Direccion de allocate: %p \n",&allocate);
	printf("Variables globales\n");
	printf(" Direccion de l2: %p \n",&l2);
	printf(" Direccion de trozos: %p \n",&trozos);
	printf(" Direccion de len: %p \n",&n);
	printf("Variables locales\n");
	printf(" Direccion de entero: %p \n",&i);
	printf(" Direccion de caracter: %p \n",&caracter);
	printf(" Direccion de decimal: %p \n",&decimal);
	}
}

void showvars(char* trozos[], lista2 l2,int n){
	int entero;
	char caracter;
	float decimal;
	printf("Variables globales\n");
	printf(" Direccion de l2: %p \n",&l2);
	printf(" Direccion de trozos: %p \n",&trozos);
	printf(" Direccion de len: %p \n",&n);
	printf("Variables locales\n");
	printf(" Direccion de entero: %p \n",&entero);
	printf(" Direccion de caracter: %p \n",&caracter);
	printf(" Direccion de decimal: %p \n",&decimal);
}

void showfuncs(char* trozos[], lista2 l2,int n){
	printf("Funciones\n");
	printf(" Direccion de Cmd_deletekey: %p \n",&Cmd_deletekey);
	printf(" Direccion de dealloc: %p \n",&dealloc);
	printf(" Direccion de allocate: %p \n",&allocate);
	/*printf("Librerias\n");
	printf(" Direccion de stdio: %p \n",&stdio.h);
	printf(" Direccion de stdlib: %p \n",&stdlib.h);
	printf(" Direccion de string: %p \n",&string.h);
	*/
}

void memory(char* trozos[],lista2  l2,int n){
	if(trozos[1]!=NULL){
		if (strcmp(trozos[1], "-allocate")==0){
			l2=allocate(trozos,l2);
		}
		if (strcmp(trozos[1], "-dealloc")==0){
			dealloc(trozos,l2);
		}
		if (strcmp(trozos[1], "-deletekey")==0){
			Cmd_deletekey(trozos);
		}
		if (strcmp(trozos[1], "-show")==0){
			show(trozos,l2,n);
		}
		if (strcmp(trozos[1], "-show-vars")==0){
			showvars(trozos,l2,n);
		}
		if (strcmp(trozos[1], "-show-funcs")==0){
			showfuncs(trozos,l2,n);
		}
		if (strcmp(trozos[1], "-dopmap")==0){
			Cmd_dopmap(trozos);
		}
	}
}

void memdump(char * arg[]) {
	int t=25,k=0,i=0,b=1;
	char * p;
	if (arg[2] != NULL){	t = atoi(arg[2]);}
	p = (char *) strtol(arg[1],NULL,16);
	while(i != t) {
		while (k<t && b){
			if (!isprint(*(p+k)))
				printf("  ");
			else
				printf("%c  ",*(p+k));
			k++; 
			if (k%25==0 && k!=0)
				b=0; }
		printf("\n");
		b=1;
		while (i<t && b){
			printf("%02hhx ",*(p+i));
			i++;
			if (i%25==0 && i!=0)
				b=0; }
		printf("\n");
		b=1; }
}


void llenar(char * p, int cont, char letra){
	int i;
	for (i=0;i<cont;i++){
		*p=letra;
		p++;
	}
}

void memfill(char * arg[]){
	char * p;
	char letra='A';
	if (arg[1]==NULL){
		return;
	}
	p=(char *) strtol(arg[1],NULL,16);
	if (arg[2]==NULL){
		llenar(p,128,letra);
	}else if (arg[3]!=NULL){
		letra = (char) strtol(arg[3],NULL,16);
		llenar (p,atoi(arg[2]),letra);
	}else if(atoi(arg[2])==0){
		letra = (char) strtol(arg[2],NULL,16);
		llenar (p,128,letra);
	}else
		llenar(p,atoi(arg[2]),letra);
}


#define LEERCOMPLETO ((ssize_t)-1)
ssize_t LeerFichero (char *fich, void *p, ssize_t n){
				/*n=-1 indica que se lea todo*/
	ssize_t  nleidos,tam=n;
	int df, aux;
	struct stat s;
	if (stat (fich,&s)==-1 || (df=open(fich,O_RDONLY))==-1)
		return ((ssize_t)-1);
	if (n==LEERCOMPLETO)
		tam=(ssize_t) s.st_size;
	if ((nleidos=read(df,p, tam))==-1){
		aux=errno;
		close(df);
		errno=aux;
		return ((ssize_t)-1);
	}
	close (df);
	return (nleidos);
}

void readfile(char *trozos[]){
	ssize_t leido;
	if ((trozos[1]!=NULL) && (trozos [2]!=NULL)) {
		void *p = (char *) strtoull (trozos[2],NULL,16);
		if (trozos[3]==NULL) {
			leido=LeerFichero(trozos[1],p,LEERCOMPLETO);
		}
		else {
			leido=LeerFichero(trozos[1],p, atoi(trozos[3]));
		}
		printf("Leidos %ld bytes de %s \n",leido,trozos[1]);
	}
	else {
		printf ("faltan parametros\n");
	}
}

ssize_t wrfile (char *fich, void *p, ssize_t n,int opciones){
	ssize_t nleidos,tam=n;
	int df, aux;
	struct stat s;
	s.st_size = 0;

	if ((df=open(fich,opciones,0777))==-1)
		return ((ssize_t)-1);
	if (n==LEERCOMPLETO)
		tam=(ssize_t) s.st_size;
	if ((nleidos=write(df,p, tam))==-1){
		aux=errno;
		close(df);
		errno=aux;
		return ((ssize_t)-1);
		}
	close (df);
	return (nleidos);
}

void writefile(char *trozos[],int n){
	ssize_t leido;
	void *p;
	if (n==5){
		if (strcmp("-o",trozos[1])==0){
			p = (char *) strtoul(trozos[3],NULL,16);
			leido=wrfile(trozos[2],p,(ssize_t)atoi(trozos[4]),O_WRONLY | O_CREAT | O_TRUNC);
			if (leido==-1) {
			printf("error cannot read file\n");
			}
			else {
			printf("escritos %ld bytes en %s \n",leido,trozos[2]);
			}
		}
	}
	else if (n==4){
		p = (char *) strtoul(trozos[2],NULL,16);
		leido=wrfile(trozos[1],p,(ssize_t)atoi(trozos[3]),O_WRONLY | O_CREAT | O_EXCL);
		if (leido==-1) {
			printf("error cannot read file\n");
		}
		else {
		printf("escritos %ld bytes en %s \n",leido,trozos[1]);
		}
	}
	else {
		printf("faltan parametros\n");
		}
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************



void getprio(char *trozos[]){
    pid_t pid;
    int prio;

    if (trozos[1]!= NULL) {
        pid =(pid_t) atoi(trozos[1]);
    }
    else {
        pid= getpid();
    }
        prio=getpriority(PRIO_PROCESS,pid);
        printf("Priority of process: %d %d\n",pid,prio);
}


void setprio(char *trozos[]){
	pid_t pid;
	int prio;
	
	if (trozos[1]!= NULL) {
		if (trozos[2]!= NULL) {
            pid = (pid_t) atoi(trozos[1]);
            prio = atoi(trozos[2]);
        }    
		else{
            pid = getpid();
            prio = atoi(trozos[1]);
        }
        setpriority(PRIO_PROCESS,pid,prio);
    }
    else{
        getprio(trozos);
    }
}

char * NombreUsuario (uid_t uid){
    struct passwd *p;
        if ((p=getpwuid(uid))==NULL)
            return (" ??????");
        return p->pw_name;
}


uid_t UidUsuario (char * nombre){
    struct passwd *p;
        if ((p=getpwnam (nombre))==NULL){
            return (uid_t) -1;
		}
        return p->pw_uid;
}


void guid (char *tr[]){
    uid_t real=getuid(), efec=geteuid();
    printf ("Real credential : %d, (%s)\n", real, NombreUsuario (real));
    printf ("Efective credential : %d, (%s)\n", efec, NombreUsuario (efec));
}

void suid(char *trozos[]){
    uid_t uid;
    int u;

    if (trozos[0] == NULL || (!strcmp(trozos[0], "-l") && trozos[1] == NULL)){
        guid(trozos);
        return;
    }
    if (!strcmp(trozos[0], "-l")){
        if ((uid = UidUsuario(trozos[1])) == (uid_t)-1){
            printf("This user does not exist %s\n", trozos[1]);
            return;
        }
    }
    else if ((uid = (uid_t)((u = atoi(trozos[0])) < 0) ? -1 : u) == (uid_t)-1){
        printf("Invalid value %s\n", trozos[0]);
        return;
    }
    if (setuid(uid) == -1)
        printf("Cannot change credential %s\n", strerror(errno));
}


void functionfork(){
	pid_t pid;
	
	if((pid=fork())==0){
		waitpid(pid,NULL,0);
	}
	else if(pid==-1){
		perror("Error: ");
	}
	
}


void execute(char *trozos[], int n){
	char * aux_trozos[MAX-1];
	int j=0;

	if(trozos[1]!=NULL){
		while (trozos[j+1] != NULL){
			j++;
		}
		if (trozos[j][0]=='@'){
			strcpy(trozos[j],trozos[j]+1);
			setpriority(PRIO_PROCESS,getpid(),atoi(trozos[j]));
			for(int i = 1; i < n-1; i++){
				aux_trozos[i-1]=trozos[i];
			}
		}
		else{
			for(int i = 1; i < n; i++){
				aux_trozos[i-1]=trozos[i];
			}
		}	
		aux_trozos[n-1]= NULL;
		if(execvp(trozos[1],aux_trozos)==-1){
			perror("Imposible execute: ");
		}
	}	
}


void foreground(char *trozos[], int n){
	pid_t pid;
	
	if((pid=fork())==0){
		execute(trozos,n);
	}
	else if(pid==-1){
		perror("Error: ");
	}
	waitpid(pid,NULL,0);
}


void background (char *trozos[], int n, listaP *lproc){
	pid_t pid;
	datoP dato;
	int signal;
	
	
	if((pid=fork())==0){
		execute(trozos,n);
	}
	else{ 	
		if(pid==-1){
			perror("Error: ");
		}
		if(strcmp(trozos[0],"background")==0){
			for(int i = 1; i < n; i++){
				dato.cline[i-1]=strdup(trozos[i]);
			}
			dato.cline[n-1] = NULL;
			
		}
		else{
			for (int i = 0; i < n; i++){
				dato.cline[i] = strdup(trozos[i]);
			}
			dato.cline[n] = NULL;
		}
		dato.pid = pid;
		dato.tiempo = time(0);
		waitpid(pid, &signal, WNOHANG |WUNTRACED |WCONTINUED);
		dato.pstate = -1;
		dato.prio = getpriority(PRIO_PROCESS,pid);
		insertElementP(dato,lproc);
	}
}




void ampersand(char *trozos[],int n,listaP *lproc){
	char * aux_trozos[MAX];
	if(strcmp(trozos[n-1],"&")==0){
		aux_trozos[0]= "background";
		for(int i = 1; i < n; i++){
			aux_trozos[i]=trozos[i-1];
		}
		background(aux_trozos,n,lproc);
		
	}
	else{
		aux_trozos[0]= "foreground";
		for(int i = 1; i < n+1; i++){ 
			aux_trozos[i]=trozos[i-1];
		}
		foreground(aux_trozos,n+1);
	}
}


void listprocs(listaP *lproc){
	datoP dato;
	int j = 0;
	char buff[MAX];
	int estado;
	for(int i = 0;i <= lproc->ultimo; i++){
		j = 0;
		if(lproc->dataproc[i].pid == waitpid(lproc->dataproc[i].pid, &estado, WNOHANG |WUNTRACED |WCONTINUED)){
			lproc->dataproc[i].pstate = estado;
		}
		lproc->dataproc[i].prio=getpriority(PRIO_PROCESS,lproc->dataproc[i].pid);
		dato = lproc->dataproc[i];
		printf("%d p=%d ",dato.pid,dato.prio);
		strftime(buff, 40, "%Y-%m-%d %H:%M:%S",localtime(&(dato.tiempo)));
		printf("%s ",buff);
		if(WIFEXITED(dato.pstate)){
			printf("FINISHED (%d) ", WEXITSTATUS(dato.pstate));
		}
		else if (WIFSIGNALED(dato.pstate)){
			printf("SIGNALED (%s) ", NombreSenal(WTERMSIG(dato.pstate)));
		}
		else if(WIFSTOPPED(dato.pstate)){
			printf("STOPPED (%s) ", NombreSenal(WSTOPSIG(dato.pstate)));

		}
		else{
			printf("ACTIVE ");
		}
		while(dato.cline[j]!=NULL){
			printf("%s ",dato.cline[j]);
			j++;
		}
		printf("\n");
	}
}



void proc(char *trozos[],listaP *lproc){
	pid_t pid;
	datoP dato;
	char buff[MAX];
	bool pertenece = false;
	int j = 0,estado;
	if(trozos[1]!=NULL){
		if(strcmp(trozos[1],"-fg")==0){
			if (trozos[2]!=NULL){
				pid = (pid_t) atoi(trozos[2]);
				for(int i = 0;i <= lproc->ultimo; i++){
					if(pid==lproc->dataproc[i].pid){
						pertenece = true;
						waitpid(lproc->dataproc[i].pid,&estado,0);
						printf("Process %d finished ",pid);
						if(WIFEXITED(estado)){
							printf("correctly. Value returned: %d\n",WEXITSTATUS(estado));
						}
						else if(WIFSIGNALED(estado)){
							printf(" by signal %s\n",NombreSenal(WTERMSIG(estado)));
						}
						borrarElementoP(i,lproc);
					}
				}
				if(!pertenece){
					listprocs(lproc);
				}
			}
			else{
				listprocs(lproc);
			}
		}
		else{
			pid = (pid_t) atoi(trozos[1]);
			for(int i = 0;i <= lproc->ultimo; i++){
				if(pid==lproc->dataproc[i].pid){
					pertenece=true;
					if(lproc->dataproc[i].pid ==waitpid(lproc->dataproc[i].pid, &estado, WNOHANG |WUNTRACED |WCONTINUED)){
						lproc->dataproc[i].pstate = estado;
					}
					lproc->dataproc[i].prio=getpriority(PRIO_PROCESS,lproc->dataproc[i].pid);
					dato = lproc->dataproc[i];
					printf("%d %d ",dato.pid,dato.prio);
					strftime(buff, 40, "%Y-%m-%d %H:%M:%S",localtime(&(dato.tiempo)));
					printf("%s ",buff);
					if(WIFEXITED(dato.pstate)){
						printf("FINISHED (%d) ", WEXITSTATUS(dato.pstate));
					}
					else if (WIFSIGNALED(dato.pstate)){
						printf("SIGNALED (%s) ", NombreSenal(WTERMSIG(dato.pstate)));
					}
					else if(WIFSTOPPED(dato.pstate)){
						printf("STOPPED (%s) ", NombreSenal(WSTOPSIG(dato.pstate)));

					}
					else{
						printf("ACTIVE ");
					}
					while(dato.cline[j]!=NULL){
						printf("%s ",dato.cline[j]);
						j++;
					}
					printf("\n");
				}
			}
			if(!pertenece){
				listprocs(lproc);
			}
		}
	}
	else{
		listprocs(lproc);
	}
}



void deleteprocs(char *trozos[],listaP *lproc){
	int estado;
	if(trozos[1]!=NULL){
		if(strcmp(trozos[1],"-term")==0){
			for(int i = 0;i <= lproc->ultimo; i++){
				if(lproc->dataproc[i].pid == waitpid(lproc->dataproc[i].pid, &estado, WNOHANG |WUNTRACED |WCONTINUED)){
					lproc->dataproc[i].pstate = estado;
				}
				if(WIFEXITED(lproc->dataproc[i].pstate)){
					borrarElementoP(i,lproc);
					i--;
				}
			}
		}
		else if(strcmp(trozos[1],"-sig")==0){
			for(int i = 0;i <= lproc->ultimo; i++){
				if(lproc->dataproc[i].pid == waitpid(lproc->dataproc[i].pid, &estado, WNOHANG |WUNTRACED |WCONTINUED)){
					lproc->dataproc[i].pstate = estado;
				}
				if(WIFSIGNALED(lproc->dataproc[i].pstate)){
					borrarElementoP(i,lproc);
					i--;
				}
			}
		}
	}
}







void imprimirPrompt(){
    printf("© ");
}

void leerEntrada(char *c){
    fgets(c, MAX, stdin);
}


void procesarEntrada(char *c,char *trozos[],listaP *lproc){
    int n = TrocearCadena(c,trozos);
    if(n>0){
        if(strcmp(trozos[0],"authors")==0){
            authors(trozos);
        }
        else if(strcmp(trozos[0],"getpid")==0){
            pid();
        }
        else if(strcmp(trozos[0],"getppid")==0){
            ppid();
        }
        else if(strcmp(trozos[0],"pwd")==0){
            pwd();
        }
        else if(strcmp(trozos[0],"chdir")==0){
            cdir(trozos);
        }
        else if(strcmp(trozos[0],"date")==0){
            date();
        }
        else if(strcmp(trozos[0],"time")==0){
            hora();
        }
        else if(strcmp(trozos[0],"historic")==0){
            historic(trozos,lproc);
        }
        else if(strcmp(trozos[0],"create")==0){
            create(trozos);
        }
        else if(strcmp(trozos[0],"delete")==0){
            delete(trozos);
        } 
        else if(strcmp(trozos[0],"list")==0){
            list(trozos,n);
        }
		else if(strcmp(trozos[0],"memory")==0){
            memory(trozos,l2,n);
        }
		else if(strcmp(trozos[0],"memdump")==0){
            memdump(trozos);
        }
		else if(strcmp(trozos[0],"memfill")==0){
            memfill(trozos);
        }
		else if(strcmp(trozos[0],"recurse")==0){
            doRecursiva(n);
        }
		else if(strcmp(trozos[0],"readfile")==0){
            readfile(trozos);
        }
		else if(strcmp(trozos[0],"writefile")==0){
            writefile(trozos,n);
        }
		else if(strcmp(trozos[0],"getpriority")==0){
            getprio(trozos);
        }
		else if(strcmp(trozos[0],"setpriority")==0){
            setprio(trozos);
        }
        else if(strcmp(trozos[0],"getuid")==0){
            guid(trozos);
        }
		else if(strcmp(trozos[0],"setuid")==0){
            suid(trozos);
        }
        else if(strcmp(trozos[0],"fork")==0){
            functionfork();
        }
		else if(strcmp(trozos[0],"foreground")==0){
            foreground(trozos,n);
        }
		else if(strcmp(trozos[0],"execute")==0){
            execute(trozos,n);
        }
		else if(strcmp(trozos[0],"background")==0){
            background(trozos,n,lproc);
        }
		else if(strcmp(trozos[0],"listprocs")==0){
    	    listprocs(lproc);
        }
		else if(strcmp(trozos[0],"proc")==0){
    	    proc(trozos,lproc);
        }
		else if(strcmp(trozos[0],"deleteprocs")==0){
    	    deleteprocs(trozos,lproc);
        }
        else if((strcmp(trozos[0],"quit")==0) || (strcmp(trozos[0],"end")==0) || (strcmp(trozos[0],"exit")==0)){
            exit(0);
        } 
        else{
				ampersand(trozos,n,lproc);
			}
    }      
}

int main()
{
    char *trozos[MAX];
    bool terminado = false;
	char * input=malloc(MAXLINE);

	listaP lproc;
	crearlista(l);
	createListP(&lproc);

    while (!terminado){
		char *array =malloc(MAXLINE);
        imprimirPrompt();
        leerEntrada(input);
		strcpy(array,input);
		insertarenLista(array, l);
        procesarEntrada(input,trozos,&lproc);
    }
    return 0;
}
