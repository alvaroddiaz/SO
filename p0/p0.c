#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>








#define MAXLINE 2048
#define MAXNAME 1024
#define MAXMATRIZ 4096




struct COMMAND{
	char *name;
	void(*pfunc)(char *tr[]);
};


typedef char *lista[MAXMATRIZ];
lista l;





/************************************************************************/
/************************************************************************/




void Inicializarlista(char *lista){
	char li[MAXMATRIZ];

	for(int i=0;i<MAXMATRIZ;i++)
	l[i]=&li[i];
	
}


int EsListaVacia(char *l){
	return (&l[0]==NULL);
}
	
void insertarenLista(char *a, char *l,int p){
	

	l[p]=a;
		
}



void ImprimirLista(char *l){
	int p;
    p=0;
	while(&l[p] != NULL){
		printf("%s",&l[p]);
		printf("\n");
		p++;
	}
	
}




				
				

void CmdAutores(char* tr[]){
char* nombre= "           Lucas Ledo Alonso        |     Alvaro Diaz Diaz  ";
char* login = "        lucas.ledo.alonso@udc.es    |      alvaro.ddiaz@udc.es  ";
if (tr[0] == NULL){
	printf("%s\n",nombre);
	printf("%s\n",login);
}
else if(strcmp(tr[0] , "-n")==0){
	printf("%s\n",nombre);
}
else if(strcmp(tr[0] , "-l")==0){
	printf("%s\n",login);
}
};

void cmdPwd (){
	char directorio [256];
    	if(getcwd(directorio,sizeof(directorio))!=NULL){
			printf("%s\n",directorio);
		}else
				perror("Error");
}

void CmdChdir(char * tr[]){
	if(tr[1]!=NULL){
		(chdir(tr[1]==-1));{
			perror("imposible cambiar directorio");
		}
		
	}else{cmdPwd();}

}


void CmdPid (char *tr[]){
			
		printf("process pid: %d\n",getpid());
		
}
void CmdPpid (char *tr[]){
	
		printf("parent proces pid %d\n",getppid());
		
}

void cmdFecha(char* tr[]){
  time_t t;
  struct tm *tm;
  char fecha[100];

  t=time(NULL);
  tm=localtime(&t);
  strftime(fecha, 100, "%d/%m/%Y", tm);
  printf ("Hoy es: %s\n", fecha);
}
void cmdHora(char* tr[]){
  time_t t;
  struct tm *tm;
  char hora[100];

  t=time(NULL);
  tm=localtime(&t);
  strftime(hora, 100, "%H:%M:%S", tm);
  printf ("Hora: %s\n", hora);
}	
void cmdExit(char* tr[]){
	exit(0);
}
void cmdhist(char *tr[]){
	
		if (tr[0] != NULL && strcmp(tr[0],"-c")==0){
	}else
		ImprimirLista(l);
}
			
	







int TrocearCadena (char * cadena, char * trozos[]){
	int i=1;

	if((trozos[0]=strtok(cadena," \n\t"))==NULL)
		return 0;
	while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
		i++;
	return i;
}

struct COMMAND cmd[]={
	{"authors", CmdAutores},
	{"getpid",CmdPid},
    {"getppid",CmdPpid},
    {"pwd",cmdPwd},
	{"chdir",CmdChdir},
	{"date",cmdFecha},
	{"time",cmdHora},
	{"historic",cmdhist},
	{"exit",cmdExit},
	{"fin",cmdExit},
	{"end",cmdExit},
	{NULL,NULL}
};

void ProcessEntrada (char imp[]){
	char *tr[MAXLINE/2];
	int i;
	if(TrocearCadena(imp, tr)==0){
		return;
	}


	for (i=0; cmd[i].name!=NULL;i++){
		if (strcmp(tr[0],cmd[i].name)==0){
			(*cmd[i].pfunc)(tr+1);
			return;
		}
	}
}



int main (int argc, char *argv[]){
	
	int i;
	int p;
	int t;

	
	Inicializarlista(l);
	

	for (i=0; i<argc; i++)
		printf("main's %dth argument value in: %s\n",i,argv[i]);
	
	while (1){
		t=0;
		char *input =malloc(MAXLINE);
		printf("#) ");
		fgets (input,MAXLINE,stdin);
		insertarenLista(input, l,t);
		ProcessEntrada (input);
	    t++;
		}
}