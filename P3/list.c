#include "list.h"

void crearLista2 (lista2 l){
    int i;
    for (i=0;i<MAXARRAY;i++){
        l[i]=NULL;
    }
}


void insertarenLista2 (lista2 l, void* p){
	int i=0;
	while (l[i]!=NULL){
		i++;
	}
	l[i] = p;
}


void eliminarElemento2 (lista2 l, int pos){
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

void* getElement2 (lista2 l, int pos){
    return &l[pos];
}

