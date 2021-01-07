
#define TAMANO 2048

#define MAX 40 
typedef struct data{
	pid_t pid;
	int prio;
	char *cline[MAX];
	time_t tiempo;
	int pstate;
}datoP;

typedef struct list{
	datoP dataproc[TAMANO];
	int ultimo;
}listaP;


int Senal(char * sen);
char *NombreSenal(int sen);
void insertElementP(datoP dato,listaP *lproc);
void createListP(listaP *lproc);
int isEmptyListP(listaP *lproc);
void borrarElementoP(int pos, listaP *lproc);
