#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "array_blocks.h"

/// tworzenie glownej tablicy wskaznikow na bloki zmergowanych plikow

struct ArrayOfBlocks createMainArray(int size){
    struct ArrayOfBlocks array;
    array.blocks=(struct Block*) calloc(size,sizeof(struct Block));
    array.max_size=size;
    array.lastPos=-1;
    return array;
}

/// zwraca tablice nazw plikow

char** defineSequence(int numOfFiles,char** argv){
    char** names=(char**) calloc(numOfFiles,sizeof(char*));
    for(int i=0;i<numOfFiles;i++){
        names[i]=(char*) calloc(100,sizeof(char));
        strcpy(names[i],argv[i+1]);
    }
    return names;
}

/// utworzenie bloku wierszy na podstawie pliku tymczasowego, zwraca indeks elementu z tablicy glownej wskazujacego na ten blok

int addNewBlockWithNoFile(struct ArrayOfBlocks array,struct Block block){
    array.blocks[array.lastPos+1]=block;
    return array.lastPos+1;
}

int addNewBlock(struct ArrayOfBlocks array,char* tmpFileName){
    FILE* tmp=fopen(tmpFileName,"r");
    if(tmp==NULL){
        printf("No such file %s\n",tmpFileName);
        exit(1);
    }
    char line[256];
    int lines=0;
    while(fgets(line,sizeof(line),tmp)){
        lines+=1;
    }
    struct Block block;
    block.last=0;
    block.rows=(char**) calloc(lines,sizeof(char*));
    for(int i=0;i<lines;i++){
        block.rows[i]=(char*) calloc(200,sizeof(char));
    }
    rewind(tmp);
    while(fgets(line,sizeof(line),tmp)){
        strcpy(block.rows[block.last++],line);
    }
    array.blocks[array.lastPos+1]=block;
    fclose(tmp);
    return array.lastPos+1;
} 

/// merge dla par plikow z sekwencji, wynik zapisany w pliku tymczasowym

struct Block mergeFiles(char* file1, char* file2){
    FILE *f1 = fopen(file1,"r");
    if(f1==NULL){
        printf("No such file %s\n",file1);
        exit(1);
    }
    FILE *f2 = fopen(file2,"r");
    if(f2==NULL){
        printf("No such file %s\n",file2);
        exit(1);
    }
    char line[256];
    int lines=0;
    while(fgets(line,sizeof(line),f1)){
        lines+=1;
    }
    struct Block block;
    block.last=0;
    block.rows=(char**) calloc(2*lines,sizeof(char*));
    for(int i=0;i<2*lines;i++){
        block.rows[i]=(char*) calloc(200,sizeof(char));
    }
    rewind(f1);
    rewind(f2);
    char line1[256];
    char line2[256];
    while(fgets(line1,sizeof(line1),f1) && fgets(line2,sizeof(line2),f2)){
        strcpy(block.rows[block.last++],line1);
        strcpy(block.rows[block.last++],line2);
    }
    fclose(f1);
    fclose(f2);
    return block;
}

struct ArrayOfBlocks mergeAll(int size,int amountOfFiles,char** argv){
    FILE* tmp=fopen("tmp.txt","w");
    struct ArrayOfBlocks array=createMainArray(size);
    char** seq=defineSequence(amountOfFiles,argv);
    for(int i=0;i<amountOfFiles;i+=2){
        struct Block block=mergeFiles(seq[i],seq[i+1]);
        for(int k=0;k<block.last;k++){
            fputs(block.rows[k],tmp);
        }
        fputs("\n",tmp);
        array.lastPos+=1;
        array.blocks[array.lastPos]=block;
    }
    fclose(tmp);
    return array;
}

/// ilosc wierszy w bloku

int countLines(struct ArrayOfBlocks array,int blockIndex){
    return array.blocks[blockIndex].last;
}

/// usuwa wiersz z okreslonego bloku

void removeRow(struct ArrayOfBlocks array,int blockIndex,int rowIndex){
    free(array.blocks[blockIndex].rows[rowIndex]);
    array.blocks[blockIndex].rows[rowIndex]=NULL;
}

/// usuwa z pamieci blok o podanym indeksie

void removeBlock(struct ArrayOfBlocks array,int blockIndex){
    int lines=countLines(array,blockIndex);
    for(int i=0;i<lines;i++){
        removeRow(array,blockIndex,i);
    }
    array.blocks[blockIndex].last=0;
}

/// wypisanie struktury

void printMerged(struct ArrayOfBlocks array){
    for(int i=0;i<=array.lastPos;i++){
        if(array.blocks[i].last>0) printf("Blok %d\n",i);
        for(int j=0;j<array.blocks[i].last;j++){
            if(array.blocks[i].rows[j] != NULL) printf("Line %d: %s",j,array.blocks[i].rows[j]);
        }
    }
}