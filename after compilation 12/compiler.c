/* [mmn 14: compiler.c]:
This file contiains the main function in the compilation process of the assembly file.
The compile() function will compile the wnated assmebly file into three optput files: 
fileName.ob, fileName.ent, fileName.ext (if possible)

This file also conatins the sub functions nedded for the main 'compile()' fucntion.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ineterface.h"
#include "files.h"
#include "line.h"
#include "util.h"
#include "label.h"
#include "instructionHandeling.h"
#include "dataHandeling.h"
#include "symbolTable.h"
#include "statmentType.h"

#define FOREVER 1 

/* compile(): The function compiles an Assembly file and make one to three files in addition:
 file.ob - a binary file, which includes the memory....
 file.ent - an "entry" file, which includes the names of all the variable which other files can see and
            their address in the RAM.
 file.ext - an "extern" file, which includes the names of all the external variables (from other files) 
            and their calling address in the RAM.
*/
boolean compile(char *fileName){
     /* files variables */
     FILE *fd;
     char *tempFileName = malloc(strlen(fileName) + MAX_SUFIX_LEN + 1);
     int lineCnt = 0;
     /* line variables */
     char line[LINE_LEN];             /* array for handling a line of assembly code */
     int *lInd = malloc(sizeof(int)); /* the Line-Index */
     char *tempLine, *optLabel;
     /* label and compilation variables and falgs */
     statType type;
     boolean compSuc = TRUE, labelExist, pushed;

     if (!isAlloc(lInd) || !isAlloc(tempFileName)){
          free(lInd);
          free(tempFileName);
          return ERROR;
     }
     if (!(fd = openf(addSufix(tempFileName, ".as", fileName), "r"))){
          free(lInd);
          free(tempFileName);
          return FALSE;
     }
     free(tempFileName);
     clean();

     while (FOREVER){
          labelExist = FALSE;
          lineCnt++;
          *lInd = 0;
          tempLine = readLine(fd, LINE_LEN); /* reading a line from the file */
          if (tempLine[0] == '\0') /* end of file */
               break;
          strcpy(line, tempLine);
          free(tempLine);
          printf("line: %s\n",line);

          if(isBlank(line, *lInd))
               continue;
          jumpSpaces(line, lInd);
          if (!(optLabel = readWord(line, lInd))){
               free(lInd);
               return ERROR;
          }
          if(/*strcmp(optLabel, "\0") != 0 &&*/ isLabel(optLabel)){
               labelExist = TRUE;
               if(!isValidLabel(optLabel, lineCnt, TRUE)) /* could not get a label */
                    compSuc = FALSE;
          }
          if(!labelExist){
               free(optLabel);
               *lInd = 0;
          }
          /*jumpSpaces(line, lInd);  for now */
          switch ((type = getStatType(line))){ /* finding the statment type (instruction/ directive/ balnk line/ comment) */
          case blank:
          case comment:
               break;
          case directive:
          {    
               int command = getDirCom(line, lInd, lineCnt);
               switch (command){
               case data:
               case string:
                    if (labelExist){
                         if (/*isIlegalName(optLabel, lineCnt) || wasDefined(optLabel, lineCnt) || */!(pushed = addToSymTab(optLabel, "code", lineCnt)))
                              compSuc = FALSE;
                         if(pushed == ERROR){
                              free(optLabel);
                              clean();
                              return ERROR;
                         }
                         free(optLabel);
                    }
                    if(command == data)
                         pushed = pushData(line, lInd, "data", lineCnt);
                    else if(command == string)
                         pushed = pushData(line, lInd, "string", lineCnt);
                    if(pushed == ERROR){
                         clean();
                         return ERROR;
                    }
                    compSuc = pushed;
                    break;
               case external:
                    if (command != external)
                         break;
                    if (labelExist){
                         isIlegalName(optLabel, lineCnt);
                         wasDefined(optLabel, lineCnt, TRUE, 1);
                         free(optLabel);
                         printf("warnning [line %d]: a label before the directive \".extern\" is meaningless\n", lineCnt);
                    }
                    if (!(pushed = pushExtern(line, lInd, lineCnt)))
                         compSuc = FALSE;
                    else if (pushed == ERROR){
                         clean();
                         return ERROR;
                    }
                    break;
               case entry:
               {    
                    boolean check;
                    if (command != entry)
                         break;
                    if(!(check = checkEntry(line, lInd, lineCnt)))
                         compSuc = FALSE;
                    if(check == ERROR){
                         free(optLabel);
                         clean();
                         return ERROR;
                    }
                    break;
               }
               }
          }
          case order:
               if (type != order)
                    break;
               if (labelExist){
                    if (/*isIlegalName(optLabel, lineCnt) || wasDefined(optLabel, lineCnt) || */!(pushed = addToSymTab(optLabel, "code", lineCnt)))
                         compSuc = FALSE;
                    if(pushed == ERROR){
                         free(optLabel);
                         clean();
                         return ERROR;
                    }
                    free(optLabel);
               }
               printSymTab();
               printf("lInd = %d\n", *lInd);
               if(!(pushed = pushInstFirst(line, lInd, lineCnt)))
                    compSuc = FALSE;
               else if(pushed == ERROR){
                    clean();
                    return ERROR;
               }
               break;
          }
     }
     
     if (compSuc == FALSE){
          clean();
          fclose(fd);
          return FALSE;
     }
     encPlusIC();
     rewind(fd);
     lineCnt = 0;

     while (FOREVER){
          *lInd = 0;
          lineCnt++;
          labelExist = FALSE;
          tempLine = readLine(fd, LINE_LEN); /* reading a line from the file */
          if (*tempLine == '\0') /* end of file */
               break;
          strcpy(line, tempLine);
          free(tempLine);

          if (!(optLabel = readWord(line, lInd))){
               free(lInd);
               free(tempFileName);
               return ERROR;
          }
          if(isLabel(optLabel))
               labelExist = TRUE;
          if(!labelExist)
               *lInd = 0;
          /*if(!strcmp(optLabel, "\0") && isLabel(optLabel))
               labelExist = TRUE;*/
          /*if(!(pushed = pushInstSecond(line, lInd, lineCnt)))
               return FALSE;
          if(pushed == ERROR)
               return ERROR;*/
          switch ((type = getStatType(line))){ /* finding the statment type (instruction/ directive/ balnk line/ comment) */
          case comment:
          case blank:
               continue;
          default:
          {
               boolean added;
               if(!(added = pushEntry(line, lInd, lineCnt))){
                    compSuc = FALSE;
                    break;
               }
               else if(added == ERROR){
                    compSuc = ERROR;
                    break;
               }
               else if(isBlank(line, *lInd) || *lInd == strlen(line))
                    break;
               jumpSpaces(line, lInd);
                    
               while(line[*lInd] != '\0'){
                    if(!(added = pushInstSecond(line, lInd, lineCnt))){
                         compSuc = FALSE;
                         break;
                    }
                    else if(added == ERROR){
                         compSuc = ERROR;
                         break;
                    }
                    isThereComma(line, lInd, lineCnt);
               }
          }
          }
     }
     if(compSuc == TRUE)
          buildOutputFiles(fileName);
     clean();
     fclose(fd);
     return compSuc;
}

void clean(){
     cleanDC();
     cleanIC();
     cleanSymTab();
}