#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array_blocks.h"

/* 
Autor rozwiazan: Tomasz Boron

Wszystkie funkcje z zad1, wypisane w opisie w sekcji "Biblioteka powinna umozliwiac" zostaly zaimplementowane. 

Ponizej prosty kod testowy, glownie do zobrazowania dzialania, ciekawsze testy umiescilem zgodnie z opisem w katalogu z zadaniem 2.

*/

int main(int argc, char** argv) {
    int size;
    printf("Structure size: ");
    scanf("%d",&size);
    if(size<=0){
        printf("Size should be positive integer\n");
        exit(1);
    }
    struct ArrayOfBlocks arr = mergeAll(size,argc-1,argv);    /// tutaj podac w mainie nazwy plikow przy wywolaniu
    arr.lastPos=addNewBlock(arr,"texts/b.txt");
    arr.lastPos=addNewBlock(arr,"texts/a.txt");
    printMerged(arr);
    /*struct ArrayOfBlocks arr2 = createMainArray(size);    /// tutaj wywolanie samo ./main
    arr2.lastPos=addNewBlock(arr2,"texts/a.txt");
    arr2.lastPos=addNewBlock(arr2,"texts/b.txt");
    arr2.lastPos=addNewBlock(arr2,"texts/c.txt");
    arr2.lastPos=addNewBlock(arr2,"texts/d.txt");
    removeBlock(arr2,2); 
    removeRow(arr2,3,2);
    printMerged(arr2);*/
    return 0;
}