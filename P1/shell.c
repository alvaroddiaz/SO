/*Álvaro Díaz Díaz  DNI 34288483Z*/
/*Lucas Ledo Alonso  DNI 21050228F*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>

#define MAX 40 

#define MAXLINE 2048
#define MAXNAME 1024


#define MAXARRAY 4096

typedef char* lista [MAXARRAY];

void procesarEntrada(char *c,char *trozos[]);


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

char * ConvierteModo2 (mode_t m)
{
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

void ImprimirLista(lista l){
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
lista l ;


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

void historic(char *trozos[]){
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
                    procesarEntrada(l[i],trozos);
					
                }
                else{
                    printf("No hay elemento %d en el historico",n);
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
                        printf("No hay elemento %d en el historico\n",n);
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
        
         ImprimirLista(l);
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

void imprimirPrompt(){
    printf("© ");
}

void leerEntrada(char *c){
    fgets(c, MAX, stdin);
}


void procesarEntrada(char *c,char *trozos[]){
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
            historic(trozos);
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
        else if((strcmp(trozos[0],"quit")==0) || (strcmp(trozos[0],"end")==0) || (strcmp(trozos[0],"exit")==0)){
            exit(0);
        } 
        else{
            printf("Comando: '%s' no valido\n", trozos[0]);
        }   
    }    
    
}

int main()
{
    char *trozos[MAX];
    bool terminado = false;
	char * input=malloc(MAXLINE);
	crearlista(l);

    while (!terminado){
		char *array =malloc(MAXLINE);
        imprimirPrompt();
        leerEntrada(input);
		strcpy(array,input);
		insertarenLista(array, l);
        procesarEntrada(input,trozos);
    }
    return 0;
}
