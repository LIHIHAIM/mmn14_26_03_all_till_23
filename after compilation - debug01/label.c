/* [mmn 14 : ]:
in this file: 

author: Uri K.H,   Lihi Haim       Date: 21.3.2021 
ID: 215105321,     313544165       Tutor: Danny Calfon */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "label.h"
#include "util.h"

#define LABEL_SIZE 31

char *readWord(char *line, int *lInd){
     char curr;
     int size = 1;
     char *label = malloc(size);
     if(!isAlloc(label))
          return NULL;
     while((curr = line[*lInd]) != '\0' && !isspace(curr) && curr != ','){
          label = realloc(label, ++size);
          if(!isAlloc(label))
               return NULL;
          label[size-1] = curr;
          (*lInd)++;
     }
     if(size == 1)
          return "\0";
     label[size] = '\0';
     return label;
}

boolean isLabel(char *string){
     int i;
     for(i = 0; i < strlen(string); i++){
          if(string[i] == ':')
               return TRUE;
     }
     return FALSE;
}

boolean isValidLabel(char *label, int lineCnt, boolean optLabel){
     boolean valid = TRUE;
     int i = 0;
     if(!optLabel && label[strlen(label)] != ':'){
          printf("error [line %d]: after the end of an optional label must come a semicolon\n", lineCnt);
          valid = FALSE; /* labbel is not valid */
     }
     if(!isalpha(label[i])){
          printf("error [line %d]: label \"%s\" must start with an alphabetic character\n", lineCnt, label);
          valid = FALSE;
     }
     if(strlen(label) > LABEL_SIZE){
          printf("error [line %d]: the maximum label length is %d characters\n", lineCnt, LABEL_SIZE);
          valid = FALSE;
     }
     for(i = 1; i < strlen(label); i++){
          if(label[i] == ':' && i != strlen(label)-1){
               printf("error [line %d]: after end of label \"%s\" must be at least one space\n", lineCnt, label);
               valid = FALSE;
               break;
          }
          if(!isalnum(label[i])){
               printf("error [line %d]: the label \"%s\" must include only alphabetical and numerical characters\n", lineCnt, label);
               valid = FALSE;
          }
     }
     if(label[strlen(label)-1] == ':')
          label[strlen(label)-1] = '\0';
     return valid;
}