#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#define MAXARRAY 4096

typedef void* lista2 [MAXARRAY];

struct node{
	void * p;
	size_t tam;
	char * name;
	char * t;
	char * type;
	int df;
	mode_t perm;
    key_t key;
};

void crearLista2 (lista2 l);
void insertarenLista2 (lista2 l, void* p);
void eliminarElemento2 (lista2 l, int pos);
void* getElement2 (lista2 l, int i);