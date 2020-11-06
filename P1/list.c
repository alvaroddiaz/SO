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


int main(int n,char *trozos[]){
	list(trozos,n);
	return 0;
}