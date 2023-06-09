#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>


#define CANT_SEGMENTOS 5
#define MV_SIZE 1024*16
#define cantRegistros 16
#define TDDS_SIZE 8
#define cantFunciones 255
#define CS 0
#define DS 1
#define KS 2
#define ES 3
#define SS 4
#define IP 5
#define SP 6
#define BP 7
#define CC 8
#define AC 9
#define EAX 10
#define EBX 11
#define ECX 12
#define EDX 13
#define EEF 14
#define EFX 15


typedef int tpar;
typedef tpar* tppar;

void inicializaFunciones(void (*funciones[cantFunciones])(tppar,tppar));
void leeArchivoBinario(unsigned char[], int* ,char*,int, char*);
void inicializaRegistros(int,char);
void dissasambly(unsigned char[],int);
void stringWrite();
void leeDeTeclado();
void stringRead();
void imprimePorPantalla();
void breakpointDebugger();
void clearScreen();
short calculaTamanoMV();
void accesoDisco();
char* creaArchivoDisco(char*);
int getReg(int,char );
void setReg(int*, char, int);
int tamanoSegmento(int);
void gestionDinamicaSeg();

void MOV(tppar,tppar);
void ADD(tppar,tppar);
void SUB(tppar,tppar);
void SWAP(tppar,tppar);
void MUL(tppar,tppar);
void DIV(tppar,tppar);
void CMP(tppar,tppar);
void SHL(tppar,tppar);
void SHR(tppar,tppar);
void AND(tppar,tppar);
void OR(tppar,tppar);
void XOR(tppar,tppar);
void SYS(tppar,tppar);
void JMP(tppar,tppar);
void JZ(tppar,tppar);
void JP(tppar,tppar);
void JN(tppar,tppar);
void JNZ(tppar,tppar);
void JNP(tppar,tppar);
void JNN(tppar,tppar);
void LDL(tppar,tppar);
void LDH(tppar,tppar);
void RND(tppar,tppar);
void NOT(tppar,tppar);
void PUSH(tppar,tppar);
void POP(tppar,tppar);
void CALL(tppar,tppar);
void STOP(tppar,tppar);
void RET(tppar,tppar);
tpar address(tpar);

unsigned char* mv;
char debugger[30]="\0";
char breakpoint=0;

struct tdiscos {
   char* discos[255];
   char size;
} discos;



int reg[cantRegistros]; //registros, variable global para que cualquier funcion pueda modificarlos
unsigned int tdds[TDDS_SIZE]; 
int cantSegTot = 0;
int m = MV_SIZE;

int main(int argc, char *argv[]) {
   
   char* extension_vmx = ".vmx";
   char* extension_vmi= ".vmi";
   char* extension_vdd= ".vdd";
   char *nombre_archivo=NULL;
   char version=0;
   int cantInstrucciones=0;


   void (*funciones[cantFunciones])(tppar,tppar)={NULL};
   inicializaFunciones(funciones);

   discos.size=0;

   for (int i = 1; i < argc; i++) {
      char *param = argv[i];
      if (strstr(param, extension_vmx) != NULL)  //.vmx
         nombre_archivo = param;

      else if (strstr(param, extension_vmi) != NULL) //.vmi
         strcpy(debugger,param);

      else if (strstr(param, extension_vdd) != NULL)//.vdd
         discos.discos[discos.size++]=creaArchivoDisco(param);
            
      else if (strncmp(argv[i], "m=", 2) == 0) { //m=M
         m = atoi(argv[i] + 2); // Extracción del valor de M como un entero

      }
   }
   if (nombre_archivo == NULL) {
      printf("Por favor indique el nombre del archivo");
      exit(-4);
   }
 
   mv = (char*)malloc(sizeof(char)*m);
   memset(mv, 0, m); //inicializa en 0 todo mv

   leeArchivoBinario(mv, &cantInstrucciones, nombre_archivo,m,&version); // lee el archivo binario y carga las instrucciones en el codesegment

// BUSCA PARAMETRO -d
   for (int i = 1 ; i < argc ; i++) { // El primer argumento es el nombre del programa
       if (strcmp(argv[i], "-d") == 0) {
           dissasambly(mv, cantInstrucciones); // Ejecuta dissasambly
           break;
       }
   }


   inicializaRegistros(m,version);

   //programa principal
   short posMemA, posMemB;
   tpar auxA;
   int aux1,aux2;
   char treg1, treg2; //0 si los valores de registros son completos por ej EFX, otro valor si son EL, EC, o EX
   unsigned char codOp, tOpA,tOpB; //codigo de operacion, tipo op1, tipo op2
   tpar op1,op2; //operandos
   tppar pOp1, pOp2; //punteros a los operandos
   char sizeA,sizeB; // cantidad de bytes para operar en memoria
   
   
   printf("\n");
   while (reg[IP] < cantInstrucciones) {

      treg1 = treg2 = op1 = op2 = aux1 = 0;


      tOpA = (mv[address(reg[IP])] >> 6) & 3; // asigna los bits 11000000
      tOpB = (mv[address(reg[IP])] >> 4) & 3; // asigna los bits 00110000


      if (tOpB == 3){ // cod operacion uno o sin operandos

         if (tOpA == 3) { // cod operacion 1111 sin operandos
            codOp = mv[address(reg[IP]++)];
         }
         else { //cod op xx11 1 operando
            codOp = mv[address(reg[IP]++)] & 63; // &00111111

         }
      }
      else { //cod op 2 operandos
            codOp = mv[address(reg[IP]++)] & 15; //&00001111
      }
      //asigno valores a los operandos dependiendo el tama�o

      if (tOpA == 0){  // operando apunta a direccion en memoria

      switch (((mv[address(reg[IP])]) >> 6) & 0b11) { //si es long(l), word(w) o byte(b)
         
         case 0b00:
            sizeA=4;
            break;
         case 0b10:
            sizeA=2;
            break;
         case 0b11:
            sizeA=1;
            break;
      }  
         int quetdds=((reg[(mv[address(reg[IP])])&0x0F]) >> 16) & 0x0000FFFF;
         auxA  = ((tdds[((reg[(mv[address(reg[IP])])&0x0F]) >> 16) & 0x0000FFFF]) >> 16) & 0x0000FFFF; //valor del registro, por ejemplo lo que esta contenido en DS (0x00010000)
         auxA += ((reg[mv[address(reg[IP]++)]&0x0F]) & 0x0000FFFF);
         
         posMemA = mv[address(reg[IP]++)];
         posMemA <<= 8;

         posMemA |= mv[address(reg[IP]++)];
         posMemA += auxA; // offset += valor del reg, por ej [DS+10]

          if ((((tdds[quetdds]>>16)&0xFFFF)) > posMemA || ((((tdds[quetdds]>>16)&0xFFFF))+(((tdds[quetdds])&0xFFFF)) < (posMemA+sizeA)) ) {
             printf("Error: Segmentation fault\n");
             exit(-420);
          }

         op1 = 0; //limpio 

         for (int j = 0; j < sizeA ; j++){ //recorta y almacena las celdas de memoria en op1
            op1 <<= 8;
            op1 |= mv[posMemA + j]; // primero es ds[posMem + 0]
         }

         op1 <<= (32-sizeA*8);
         op1 >>= (32-sizeA*8);

         pOp1 =& op1;
      }


      else if (tOpA == 1){

         op1 = mv[address(reg[IP]++)];
         op1 = op1 <<  8;

         op1 = op1 | mv[address(reg[IP]++)];
         op1<<=16; op1>>=16;

         pOp1 =& op1;

      }
      else if (tOpA == 2){

         treg1 = (mv[address(reg[IP])] >> 4) & 0b0011;
         op1   = mv[address(reg[IP]++)] & 0b1111;

         aux1 = op1;

         switch (treg1) {
            //nada, es el completo, EAX
            case 0b00: op1 = reg[op1];
               break;
            //Low register, AL
            case 0b01: op1 = (reg[op1] & 0x000000FF);
            op1<<=24; op1>>=24;
               break;
            //High register, AH
            case 0b10: op1 = ((reg[op1] & 0x0000FF00) >> 8);
            op1<<=24; op1>>=24;
               break;
            //Mitad del registro, AX
            case 0b11: op1 = (reg[op1] & 0x0000FFFF);
            op1<<=16; op1>>=16;
               break;
         }



         pOp1 =& op1;
      }

      if (tOpB == 0){
         int auxB;

      switch (((mv[address(reg[IP])])>>6)&0b11) { //si es long(l), word(w) o byte(b)
         
         case 0b00:
            sizeB=4;
            break;
         case 0b10:
            sizeB=2;
            break;
         case 0b11:
            sizeB=1;
            break;
       }
         int quetdds=((reg[(mv[address(reg[IP])])&0x0F]) >> 16) & 0x0000FFFF;
         auxB = ((tdds[((reg[(mv[address(reg[IP])])&0xF])>>16)&0x0000FFFF])>>16)&0x0000FFFF; //valor del registro, por ejemplo lo que esta contenido en DS (0x00010000)
         auxB +=((reg[mv[address(reg[IP]++)]&0xF])&0x0000FFFF);

         posMemB = mv[address(reg[IP]++)];
         posMemB <<= 8;

         posMemB |= mv[address(reg[IP]++)];

         posMemB += auxB; // offset += valor del reg, por ej [DS+10]
         
          if ((((tdds[quetdds]>>16)&0xFFFF)) > posMemB || ((((tdds[quetdds]>>16)&0xFFFF))+(((tdds[quetdds])&0xFFFF)) < (posMemB+sizeB)) )  { //En la segunda parte puede que sea modificado (el 4)
             printf("Error: Segmentation fault\n");
             exit(-420);
          }


         op2=0; //limpio
         for (int j = 0; j < sizeB ; j++){ //recorta y almacena las celdas de memoria en op2
            op2 <<= 8;
            op2 |= mv[posMemB + j]; // primero es ds[posMem + 0]
         }

         op2<<=32-sizeB*8;
         op2>>=32-sizeB*8;

         pOp2 =& op2;
      }
      else if (tOpB == 1){

         op2 = mv[address(reg[IP]++)];
         op2 = op2 <<  8;

         op2 = op2 | mv[address(reg[IP]++)];
         op2<<=16; op2>>=16;


         pOp2 =& op2;
      }
      else
         if (tOpB == 2){

            treg2 = (mv[address(reg[IP])] >> 4) & 0b0011;
            op2   = mv[address(reg[IP]++)] & 0b1111;
            aux2 = op2;

               switch (treg2) {
                  //nada, es el completo, EAX
                  case 0b00: op2 = (reg[op2]);
                     break;
                  //Low register, AL
                  case 0b01: op2 = (reg[op2] & 0x000000FF);
                  op2<<=24; op2>>=24;
                     break;
                  //High register, AH
                  case 0b10: op2 = ((reg[op2] & 0x000000FF00) >> 8);
                  op2<<=24; op2>>=24;
                     break;
                  //Mitad del registro, AX
                  case 0b11: op2 = (reg[op2] & 0x0000FFFF);
                  op2<<=16; op2>>=16;
                     break;
               }
            pOp2 =& op2;

         }

      if (funciones[codOp] == NULL) {
      printf("Error: Codigo de operacion invalido\n");
      exit(1);
      }

      funciones[codOp](pOp1,pOp2);

      if (tOpA == 2) { //mande un registro como tOpA
         switch (treg1)
         {
            //nada, es el registro completo EAX
            case 0b00: reg[aux1] = *pOp1;
               break;
            //Low register, AL
            case 0b01:
               reg[aux1] &= 0xFFFFFF00;
               op1 &= 0x000000FF;
               reg[aux1] |= op1;
            break;
            //High register, AH
            case 0b10:
               reg[aux1] &= 0xFFFF00FF;
               *pOp1   <<= 8;
               op1&=0x0000FF00;
               reg[aux1] |= *pOp1;
            break;
            //Mitad del registro, AX
            case 0b11:
               reg[aux1] &= 0xFFFF0000;
               op1&=0x0000FFFF;
               reg[aux1] |= *pOp1;
            break;
         }
      }
      else
         if (tOpA == 0){


            for (int j = sizeA-1; j >= 0 ; j--){ //recorta y almacena las celdas de memoria en op2
               mv[posMemA + j] = ((*pOp1)&0x000000FF);
               *pOp1 >>= 8;
            }

      }
      if (codOp == 3) { //codOP 3 = SWAP
         if (tOpB == 0) {

            for (int j = sizeB-1; j >= 0 ; j--){ //recorta y almacena las celdas de memoria en op2
               mv[posMemB + j] = ((*pOp2)&0x000000FF);
               *pOp2>>=8;
            }

         }
         else
            if (tOpB == 2) {
               switch (treg2)
               {
               //nada, es el registro completo EAX
               case 0b00: reg[aux2] = *pOp2;
                  break;
               //Low register, AL
               case 0b01:
                  reg[aux2] &= 0xFFFFFF00;
                  op2&=0x000000FF;
                  reg[aux2] |= op2;
               break;
               //High register, AH
               case 0b10:
                  reg[aux2] &= 0xFFFF00FF;
                  *pOp2   <<= 8;
                  op1&=0x0000FF00;
                  reg[aux2] |= *pOp2;
               break;
               //Mitad del registro, AX
               case 0b11:
                  reg[aux2] &= 0xFFFF0000;
                  op1&=0x0000FFFF;
                  reg[aux2] |= *pOp2;
               break;
               }
            }
      }
      if (breakpoint==1){ //sigue ejecutando el breakpoint
      int F = 15;
      funciones[48](&F,0);
      }
   }



    return 0;
}

void inicializaFunciones(void (*funciones[cantFunciones])(tppar,tppar)){
   
   funciones[0]=MOV;
   funciones[1]=ADD;
   funciones[2]=SUB;
   funciones[3]=SWAP;
   funciones[4]=MUL;
   funciones[5]=DIV;
   funciones[6]=CMP;
   funciones[7]=SHL;
   funciones[8]=SHR;
   funciones[9]=AND;
   funciones[10]=OR;
   funciones[11]=XOR;
   funciones[48]=SYS;
   funciones[49]=JMP;
   funciones[50]=JZ;
   funciones[51]=JP;
   funciones[52]=JN;
   funciones[53]=JNZ;
   funciones[54]=JNP;
   funciones[55]=JNN;
   funciones[56]=LDL;
   funciones[57]=LDH;
   funciones[58]=RND;
   funciones[59]=NOT;
   funciones[60]=PUSH;
   funciones[61]=POP;
   funciones[62]=CALL;
   funciones[240]=STOP;
   funciones[241]=RET;

}

void dissasambly(unsigned char cs[],int cantidadInstrucciones){

   char funciones[256][5]={0};
   char registros[16][4];
   tpar op1, op2;

   strcpy(funciones[0],"MOV");
   strcpy(funciones[1],"ADD");
   strcpy(funciones[2],"SUB");
   strcpy(funciones[3],"SWAP");
   strcpy(funciones[4],"MUL");
   strcpy(funciones[5],"DIV");
   strcpy(funciones[6],"CMP");
   strcpy(funciones[7],"SHL");
   strcpy(funciones[8],"SHR");
   strcpy(funciones[9],"AND");
   strcpy(funciones[10],"OR");
   strcpy(funciones[11],"XOR");
   strcpy(funciones[48],"SYS");
   strcpy(funciones[49],"JMP");
   strcpy(funciones[50],"JZ");
   strcpy(funciones[51],"JP");
   strcpy(funciones[52],"JN");
   strcpy(funciones[53],"JNZ");
   strcpy(funciones[54],"JNP");
   strcpy(funciones[55],"JNN");
   strcpy(funciones[56],"LDL");
   strcpy(funciones[57],"LDH");
   strcpy(funciones[58],"RND");
   strcpy(funciones[59],"NOT");
   strcpy(funciones[60],"push");
   strcpy(funciones[61],"pop");
   strcpy(funciones[62],"call");
   strcpy(funciones[240],"STOP");
   strcpy(funciones[241],"ret");

   strcpy(registros[0],"cs");
   strcpy(registros[1],"ds");
   strcpy(registros[2],"ks");
   strcpy(registros[3],"es");
   strcpy(registros[4],"ss");
   strcpy(registros[5],"IP");
   strcpy(registros[6],"sp");
   strcpy(registros[7],"bp");
   strcpy(registros[8],"CC");
   strcpy(registros[9],"AC");
   strcpy(registros[10],"EAX");
   strcpy(registros[11],"EBX");
   strcpy(registros[12],"ECX");
   strcpy(registros[13],"EDX");
   strcpy(registros[14],"EEX");
   strcpy(registros[15],"EFX");

   unsigned char codOp, tOpA, tOpB; //codigo de operacion, tipo op1, tipo op2
   int i = 0;
   char quereg1, quereg2; 

   while ( i < cantidadInstrucciones){

   printf("[%04X]  ", i);

      tOpA = (cs[i] >> 6) & 3; // asigna los bits 11000000
      tOpB = (cs[i] >> 4) & 3; // asigna los bits 00110000

      printf("%02X ",cs[i]);

      if (tOpB == 3) { // cod operacion uno o sin operandos

         if (tOpA == 3) {  // cod operacion 1111 sin operandos
         printf ("\t");
            codOp = cs[i++];
         }
         else { //cod op xx11 1 operando
            codOp = ((cs[i++]) & 63);
            // &00111111
         }
      }
      else { //cod op 2 operandos
            codOp = ((cs[i++])&15);
             //&00001111
      }

      if (tOpA == 0) { // operando apunta a direccion en memoria
         printf("%02X ",cs[i]);
         op1 = cs[i++];
         op1 = op1 <<  8;
         printf("%02X ",cs[i]);
         op1 = op1 | cs[i++];
         op1 = op1 << 8;
         printf("%02X ",cs[i]);
         op1 = op1 | cs[i++];


         //pOp1=&(ds[op1]);
      }
      else if (tOpA == 1){
         printf("%02X ",cs[i]);
         op1 = cs[i++];
         op1 = op1 <<  8;
         printf("%02X ",cs[i]);
         op1  = op1 | cs[i++];
         op1<<=16; op1 >>= 16;

         //pOp1=&op1;
      }
      else if (tOpA == 2){

         quereg1 = (cs[i] >> 4) & 3;
         op1 = cs[i++];
         printf("%02X ",op1); // ,cs[i]) decia
      }

        // pOp1=&(reg[op1]);


      if (tOpB == 0){
         printf("%02X ",cs[i]);
         op2 = cs[i++];
         op2 = op2 <<  8;
         printf("%02X ",cs[i]);
         op2 = op2 | cs[i++];
         op2 = op2 << 8;
         printf("%02X ",cs[i]);
         op2 = op2 | cs[i++];


        // pOp2=&(ds[op2]);
      }
      else if (tOpB == 1){
         printf("%02X ",cs[i]);
         op2 = cs[i++];
         op2 = op2 <<  8;
         printf("%02X ",cs[i]);
         op2 = op2 | cs[i++];
         op2<<=16; op2>>=16;



        // pOp2=&op2;
      }
      else if (tOpB == 2){
         quereg2 = (cs[i] >> 4) & 3;
         op2 = cs[i++];
         printf("%02X ",op2); // ,cs[i]) decia


        // pOp2=&(reg[op2]);

      }
      if ((codOp== 60 || codOp ==61) && tOpA == 2)
      printf("\t");
      if (tOpA+tOpB < 2)
         printf("\t | \t");
      else
         printf("\t\t |\t");
      printf ("%s \t\t",strlwr(funciones[codOp]));

      if (tOpA==0) {

         if ((op1>>22)&0b11 == 0b10) {
            printf("Instruccion invalida");
            exit(-20);   
         }

         char size;
         switch ((op1 >> 22) & 0b11) { // los dos primero bits indican long(l), word(w) o byte(b)
         case 0b00:
            size = 'l';
            break;
         case 0b10:
            size ='w';
            break;
         case 0b11:
            size = 'b';
            break;
         }

         if ((op1) & 0x00FFFF) {
            if ((op1)<<16>>16 < 0)
               printf("%c[%s%d], ",size,registros[(op1 >> 16) & 0xF],(op1)<<16>>16); //si es negativo, usa signo menos
            else
               printf("%c[%s+%d], ",size,registros[(op1 >> 16) & 0xF],(op1)&0x0000FFFF);
         if ((op1) & 0x00FFFF < 10 || (op1) & 0x00FFFF > -10) {}; // al pedo?
         
         }
         else
         printf("%c[%s],\t ",size,registros[(op1>>16)&0xF]);
      }

      else if (tOpA==1){
         if((codOp >=49 && codOp <=55) || codOp == 62 ) //es algun jump tiene q mostrar en hexa
            printf("%X\t",op1);
         else if (codOp==48 | codOp==60 || codOp == 61 || codOp == 62){
            printf("%d\t",op1);// es sys, muestra sin la coma
         } else
          printf("%d,\t",op1);

      }

      else
         if (tOpA == 2){
            switch (quereg1) {
               case 0: printf ("%s,\t",strlwr(registros[op1]));
                  break;
               case 1: printf ("%cl,\t",tolower(registros[op1 - 16][1]));
                  break;
               case 2: printf ("%ch,\t",tolower(registros[op1 - 32][1]));
                  break;
               case 3: printf ("%cx,\t",tolower(registros[op1 - 48][1]));
                  break;
            }
         //printf ("%s,\t",strlwr(registros[op1]));
         }
         printf("\t");

         if (tOpB==0){
            if ((op2>>22)&0b11 == 0b10){
               printf("Instruccion invalida");
               exit(-20);   
            }

            char size;
            switch ((op2>>22)&0b11){ // los dos primero bits indican long(l), word(w) o byte(b)
            case 0b00:
               size = 'l';
               break;
            case 0b10:
               size ='w';
               break;
            case 0b11:
               size = 'b';
               break;
            };
            if ((op2)&0x00FFFF){
               if ((op2)<<16>>16 < 0)
                  printf("%c[%s%d] ",size,registros[(op2 >> 16) & 0xF],(op2)<<16>>16); //si es negativo, usa signo menos
               else
                  printf("%c[%s+%d]\t",size,registros[(op2>>16)&0xF],(op2)&0x0000FFFF);
            }
            else
               printf("%c[%s]\t",size,registros[(op2>>16)&0xF]);
         }

      else if (tOpB==1){
         if(codOp >=50 && codOp <=55) //es algun jump, tiene q mostrar en hexa
          ;//  printf("%X,\t",op2);
         else
          printf("%d\t",op2);

      }

      else
         if (tOpB == 2){
            switch (quereg2) {
               case 0: printf ("%s\t",strlwr(registros[op2]));
                  break;
               case 1: printf ("%cl\t",tolower((registros[op2 - 16][1])));
                  break;
               case 2: printf ("%ch\t",tolower((registros[op2 - 32][1])));
                  break;
               case 3: printf ("%cx\t",tolower((registros[op2 - 48][1])));
                  break;
            }
         }


      printf("\n");





   }
}


void MOV(tppar op1,tppar op2){ ///0
   //printf("%s %04X\n ",__func__,reg[IP]);

    *op1=*op2;

}
void ADD(tppar op1,tppar op2){ ///1
 //printf("%s %04X\n ",__func__,reg[IP]);

     *op1+=*op2;

     if (*op1==0)
       reg[CC]=1;
    else if (*op1<0)
       reg[CC]=2;
      else
       reg[CC]=0;
 }
void SUB(tppar op1,tppar op2){ ///2
//printf("%s %04X\n ",__func__,reg[IP]);

    *op1 -= *op2;

   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;
   
   reg[CC]<<=30;

}
void SWAP(tppar op1,tppar op2){ ///3
//printf("%s %04X\n ",__func__,reg[IP]);
    tpar aux;

    aux  = *op1;
    *op1 = *op2;
    *op2 = aux;
}
void MUL(tppar op1,tppar op2) { ///4
//printf("%s %04X\n ",__func__,reg[IP]);

   (*op1) *= (*op2);

   if (*op1 == 0)
      reg[CC] = 1;
   else
      if (*op1 < 0)
         reg[CC] = 2;
      else
         reg[CC] =0;
   reg[CC]<<=30;
}
void DIV(tppar op1,tppar op2) { ///5
//printf("%s %04X\n ",__func__,reg[IP]);


   if (*op2 == 0) {
      printf("Error: Division por cero\n");
      exit(1);
   }

   if (*op2){
      reg[AC] = (*op1) % *op2; ///el resto de la division se guarda en el registro AC
      (*op1) /= (*op2);
   }

   if (*op1 == 0)  //modifica valores de Z y N
      reg[CC] = 1;
   else
      if (*op1 < 0)
         reg[CC]=2;
      else
         reg[CC]=0;
         reg[CC]<<=30;


}
void CMP(tppar op1,tppar op2) { ///6
//printf("%s %04X\n ",__func__,reg[IP]);



   tpar aux=*op1;
   aux -= *op2;


   if (aux==0)
      reg[CC]=1;
   else if (aux<0)
      reg[CC]=2;
   else
      reg[CC]=0;
      reg[CC]<<=30;
}
void SHL(tppar op1,tppar op2) { ///7
//printf("%s %04X\n ",__func__,reg[IP]);
   *op1 = *op1 << *op2;
}
void SHR(tppar op1,tppar op2) { ///8
//printf("%s %04X\n ",__func__,reg[IP]);
   *op1 = *op1 >> *op2;
}
void AND(tppar op1,tppar op2) { ///9
//printf("%s %04X\n ",__func__,reg[IP]);
   *op1 = (*op1) & (*op2);

   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;
      reg[CC]<<=30;
}
void OR(tppar op1,tppar op2) { ///10 o A
//printf("%s %04X\n ",__func__,reg[IP]);
    *op1 = (*op1) | (*op2);

   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;
      reg[CC]<<=30;
}
void XOR(tppar op1,tppar op2) { ///11 o B
//printf("%s %04X\n ",__func__,reg[IP]);
    *op1 = (*op1) ^ (*op2);


   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;
      reg[CC]<<=30;
}
void SYS(tppar op1,tppar op2) { ///48 
//printf("%s %04X\n ",__func__,reg[IP]);

   if (*op1 == 1)

      leeDeTeclado();

   else if(*op1 == 2)

      imprimePorPantalla();
   
   else if(*op1 == 3) 

      stringRead();

   else if (*op1 == 4)
      
      stringWrite();

   else if (*op1 == 7)
      
      clearScreen();

   else if(*op1 == 13) 

      accesoDisco();

   else if (*op1 == 14)
      
      gestionDinamicaSeg();
      
   else if (*op1 == 15) 

      breakpointDebugger();
   
   else {
      printf("\nError! Parametro invalido en funcion SYS\n");
      exit(-2);
   }

}
void JMP(tppar op1,tppar op2) { ///49
//printf("%s %04X\n ",__func__,reg[IP]);

   reg[IP]=*op1;
}
void JZ(tppar op1,tppar op2) { ///50
//printf("%s %04X\n ",__func__,reg[IP]);

   if (((reg[CC]>>30)&0b11)== 1)
      reg[IP]=*op1;
}
void JP(tppar op1,tppar op2) { ///51
//printf("%s %04X\n ",__func__,reg[IP]);

   if (((reg[CC]>>30)&0b11)==0)
      reg[IP]=*op1;
}
void JN(tppar op1,tppar op2) { ///52
//printf("%s %04X\n ",__func__,reg[IP]);

   if (((reg[CC]>>30)&0b11)==2)
      reg[IP]=*op1;
}
void JNZ(tppar op1,tppar op2) { ///53
//printf("%s %04X\n ",__func__,reg[IP]);

   if (((reg[CC]>>30)&0b11)!=1)
      reg[IP]=*op1;
}
void JNP(tppar op1,tppar op2) { ///54
//printf("%s %04X\n ",__func__,reg[IP]);

   if (((reg[CC]>>30)&0b11)!=0)
      reg[IP]=*op1;
}
void JNN(tppar op1,tppar op2) { ///55
//printf("%s %04X\n ",__func__,reg[IP]);

   if (((reg[CC]>>30)&0b11)!=2)
      reg[IP]=*op1;
}
void LDL(tppar op1,tppar op2) { //56
//printf("%s %04X\n ",__func__,reg[IP]);

   tpar aux=*op1;
   aux&=0x0000FFFF;
   reg[AC]&=0xFFFF0000;
   reg[AC]|=aux;

}
void LDH(tppar op1,tppar op2) { //57
//printf("%s %04X\n ",__func__,reg[IP]);

   tpar aux=*op1;
   aux&=0x0000FFFF;
   aux<<=16;
   reg[AC]&=0x0000FFFF;
   reg[AC]|=aux;

}
void RND(tppar op1,tppar op2) { //58
//printf("%s %04X\n ",__func__,reg[IP]);

reg[AC]=rand() % (*op1 + 1);
}
void NOT(tppar op1,tppar op2) { //59
//printf("%s %04X\n ",__func__,reg[IP]);

   *op1= ~*op1;

   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;
      reg[CC]<<=30;

}
void PUSH(tppar op1,tppar op2){
   //printf("%s %04X\n ",__func__,reg[IP]);

   reg[SP] -= 4;
   if (reg[SP] < reg[SS]) {
      printf("Error: Stack Overflow en %x",reg[IP]);
      exit(-1000);     
   }

   for (int i = 0; i< 4 ; i++) {
      mv[address(reg[SP]) + i] = (*op1)>> (24-i*8);
   }
}                                            
void POP(tppar op1,tppar op2){
   //printf("%s %04X\n ",__func__,reg[IP]);
      
   if (reg[SP] >= (reg[SS] + ((tdds[(reg[SS]) >> 16]) & 0x0000FFFF))){ // si la pila esta vacia
      printf("Error: Stack Underflow");
      exit(-999);     
   }


   for (int i = 0; i< 4; i++){
      (*op1)<<=8;
      (*op1) |= mv[address(reg[SP]) + i];
      
   }
   reg[SP]+=4;
}
void CALL(tppar op1,tppar op2){
   //printf("%s %04X\n ",__func__,reg[IP]);
   
   PUSH(reg+IP,0); //reg+IP porque se manda puntero 
   JMP(op1,0);
}
void STOP(tppar op1,tppar op2){
   //printf("%s %04X\n ",__func__,reg[IP]);

   exit(1); 


}
void RET(tppar op1,tppar op2){
   //printf("%s %04X\n ",__func__,reg[IP]);
   POP(&reg[IP],0);   
}
tpar address(tpar num) {

   //recibe puntero a memoria y devuelve la direccion
   //ejemplo recibe 0x00010020 -> busca ttds 0x0001 -> agarra la direccion del segmento y le suma el offset 0020
   tpar cualtdds= (num>>16)&0x0000FFFF;
   tpar posmem = ((tdds[cualtdds]>>16)&0x0000FFFF) + (num&0x0000FFFF);

   return posmem;
}
short calculaTamanoMV(){
   
   char i=0;
   short tamano=0;
   while (i < TDDS_SIZE){
      tamano+=(tdds[i++]&0x0000FFFF);
   }
   
 return tamano;
}
void leeArchivoBinario(unsigned char mv[], int *cantInstrucciones, char *nombre_archivo, int m, char* version){
   char vmx23[5];
   FILE* arch = fopen(nombre_archivo,"rb");
   if (arch == NULL){
   printf("no se pudo abrir el archivo %s",nombre_archivo);
   exit(-4);
   }
   short espacioCode;
   
   int i = 0;
   unsigned char byte;

   fread(&byte, sizeof(byte), 1, arch);
   vmx23[0]=byte;
   fread(&byte, sizeof(byte), 1, arch);
   vmx23[1]=byte;
   fread(&byte, sizeof(byte), 1, arch);
   vmx23[2]=byte;
   fread(&byte, sizeof(byte), 1, arch);
   vmx23[3]=byte;
   fread(&byte, sizeof(byte), 1, arch);
   vmx23[4]=byte;
   vmx23[5]='\0';
   if (strcmp(vmx23,"VMX23")){
      printf("Archivo no reconocido, no empieza con VMX23");
      exit(-90);
   }
   fread(&byte, sizeof(byte), 1, arch);
   *version=byte;
   

   if (*version==1){
      
      fread(&byte,sizeof(char),1,arch);
      espacioCode=byte;
      espacioCode<<=8;
      fread(&byte,sizeof(char),1,arch);
      espacioCode|=byte;

      tdds[0]   = 0x0000; //posicion del code segment
      tdds[0] <<= 16;
      
         tdds[0] |= espacioCode; //tamano del code segment
      // printf("code segment: %X \n",tdds[0]);

      tdds[1]   = espacioCode; // posicion del data segment
      tdds[1] <<= 16;
      tdds[1]  |= (MV_SIZE - espacioCode); //tamano del data segment

      if (espacioCode > m) {
         printf("Espacio en memoria insuficiente para cargar el CS, el espacio definido de memoria es %d",m);
         exit(-404);
         }

      while (fread(&byte, sizeof(byte), 1, arch)) {
            if(i+1>m){
               printf("Espacio en memoria insuficiente para cargar el CS, el espacio definido de memoria es %d",m);
               exit(-404);
            }
            mv[i++] = byte;

      }

      *cantInstrucciones=i;

   }
   
   else if (*version==2 || *version ==3 ){

      short memoriaOffset=0; 

      for (char k = 0; k<5; k++){ //inicializa los tdds
         fread(&byte,sizeof(char),1,arch);
         espacioCode = byte;
         espacioCode<<=8;
         fread(&byte,sizeof(char),1,arch);
         espacioCode |= byte;
         if (byte > 0) // si es mayor a cero es porque existe el segmento
            cantSegTot++;


         tdds[k]   = memoriaOffset; 
         tdds[k] <<= 16;
         
         
         tdds[k] |= espacioCode;
        

         memoriaOffset += espacioCode;
   }
         tdds[5]=tdds[6]=tdds[7]=((tdds[4]>>16)+(tdds[4]&0x0000FFFF))<<16;
     
      int espacioTotal=0;
      for (int k = 0;k<CANT_SEGMENTOS;k++){ //chequea espacio total
         espacioTotal+=(tdds[k]&0x0000FFFF);

      }
      
      if (espacioTotal>m){
         printf ("No hay memoria suficiente para cargar todos los segmentos. Memoria necesaria: %dB, memoria disponible:%dB",espacioTotal,m);
         exit(-40);
      }
      
      for (i=0;i<((tdds[0]&0xFFFF)+(tdds[1]&0xFFFF));i++){ //CS + KS

         fread(&byte, sizeof(byte), 1, arch); 

         mv[i]=byte;

         
      }

   *cantInstrucciones=(tdds[0]&0xFFFF);
   }

   else{
      printf("Numero de version no soportado");
      exit(23);
   }
    fclose(arch);
}
void inicializaRegistros(int tamano_mv, char version){

  if (version==1){
      reg[CS]=0x00000000;
      reg[DS]=0x00010000;
   }


   if (version == 2 || version == 3 ){
      reg[CS]=0x00000000;
      reg[KS]=0x00010000;
      reg[DS]=0x00020000;    
      reg[ES]=0x00030000;  
      reg[SS]=0x00040000;


      for (char k=0; k<5;k++){ //si seg no existe, reg[k] apunta a -1
         
         if ((tdds[(reg[k]>>16)&0xFFFF]&0xFFFF) <= 0){
            reg[k]=-1;
         }

      }

      for (char k=0; k<5;k++){ // si seg no existe, reacomoda a los de abajo
         char cantRecorridos=0;
         while ((tdds[k]&0xFFFF) <= 0 && cantRecorridos < 8){

            for(int j = k; j<5; j++){
               tdds[j]=tdds[j+1];
               for (int i=0;i<5;i++){  //si algun reg apuntaba a j+1, lo muevo a j
                 
                  if (((reg[i]>>16)&0xFFFF) == j+1){
                     reg[i]-=0x00010000;
                     break; 
                  }
               }
            }
            cantRecorridos++;
         }
      }
      reg[SP]=reg[SS] + (tdds[reg[SS]>>16] & 0x0000FFFF); //inicio del StackSegment + offset (el puntero va al final del segmento )
      reg[BP]=reg[SP];
      
   }


   
}

void leeDeTeclado() { //op1 = 1
   int i;
   int aux;
   int k;
      switch (reg[EAX]&0b0001111) {

         case 1: //interpreta decimal
            for (i=((tdds[reg[EDX]>>16]>>16)+reg[EDX]&0x0000FFFF);i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+(tdds[reg[EDX]>>16]>>16)+(reg[EDX]&0x0000FFFF);){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               printf("[%04X]: ", i-(tdds[reg[EDX]>>16]>>16));
               scanf("%d",&aux); //guarda en aux para despues recortar
               k=8*((reg[ECX]>>8)&0xFF)-8;
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  mv[i++]=((aux>>k)&0x000000FF);
                  k-=8;
               }
            }
         break;

         case 2: //intepreta caracter
            for (i=((tdds[reg[EDX]>>16]>>16)+reg[EDX]&0x0000FFFF);i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+(tdds[reg[EDX]>>16]>>16)+(reg[EDX]&0x0000FFFF);){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               printf("[%04X]: ", i-(tdds[reg[EDX]>>16]>>16));
               scanf("%c",&aux); //guarda en aux para despues recortar
               k=8*((reg[ECX]>>8)&0xFF)-8;
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  mv[i++]=(aux>>k)&0x000000FF;
                  k-=8;
               }
            }
         break;

         case 4: // interpreta octal
            for (i=((tdds[reg[EDX]>>16]>>16)+reg[EDX]&0x0000FFFF);i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+(tdds[reg[EDX]>>16]>>16)+(reg[EDX]&0x0000FFFF);){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               printf("[%04X]: ", i-(tdds[reg[EDX]>>16]>>16));
               scanf("%o",&aux); //guarda en aux para despues recortar
               k=8*((reg[ECX]>>8)&0xFF)-8;
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  mv[i++]=(aux>>k)&0x000000FF;
                  k-=8;
               }
            }
         break;

         case 8: // interpreta Hexa
            for (i = ((tdds[reg[EDX] >> 16] >> 16) + reg[EDX] & 0x0000FFFF) ; i < ((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+(tdds[reg[EDX]>>16]>>16)+(reg[EDX]&0x0000FFFF);){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               printf("[%04X]: ", i-(tdds[reg[EDX]>>16]>>16));
               scanf("%x",&aux); //guarda en aux para despues recortar
               k=8*((reg[ECX]>>8)&0xFF)-8;
               for (int j = 0 ; j < ((reg[ECX] >> 8) & 0xFF) ; j++){ //recorta y almacena
                  mv[i++] = (aux >> k) & 0x000000FF;
                  k-=8;
               }
            }
         break;

      default:
         printf("\nError, valor de operacion para SYS invalido\n");
         exit(-3);
         break;
      }

   }
void imprimePorPantalla(){ //op1 = 2
   int i,aux;
   for (i = ((tdds[reg[EDX] >> 16] >> 16) + (reg[EDX] & 0x0000FFFF)) ; i < ((reg[ECX] >> 8) & 0xFF) * (reg[ECX] & 0xFF) + (tdds[reg[EDX] >> 16] >> 16) + (reg[EDX] & 0x0000FFFF);){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH (cantidad de datos a leer)
         aux=0;
      printf("[%04X]:",i-(tdds[reg[EDX]>>16]>>16));
      if (reg[EAX]&0b10) //si tiene que imprimir caracter
            printf ("'");

         for (int j = 0; j < ((reg[ECX] >> 8) & 0xFF) ; j++){ //recorta y almacena
            
            if (reg[EAX] & 0b10){ //si tiene que imprimir caracter
               
               if (isprint(mv[i]))
                  printf("%c",mv[i]);
               else 
                  printf(".");
            }

            aux=aux<<8;
            aux=aux|mv[i++]; //guarda en aux para ir armando el int
         }
      if (reg[EAX] & 0b10) //si tiene que imprimir caracter
            printf (" ");

      if (reg[EAX] & 0b1000)
         printf("%%%X ",aux);
      if (reg[EAX] & 0b1) 
         printf("#%d ",aux);

      if (reg[EAX] & 0b100)
         printf("@%o ",aux);
     
      printf("\n");
      }
}
void stringRead(){ //op1=3
   short longitudd;
   int  i, j;
   char palabra[50]; // DEFINICION INICIAL!¡
   scanf("%s",palabra);
   longitudd = (reg[ECX] & 0x0000FFFF);

   i = (tdds[reg[EDX] >> 16] >> 16) + (reg[EDX] & 0x0000FFFF);
   j = 0;

   if (longitudd == -1) { //cantidad ilimitada de caracteres
      
      while (j <= strlen(palabra)) {
         mv[i] = palabra[j];
         i++;
         j++;
      }
   } 
   else { //cantidad limitada de caracteres
      while (j <= longitudd) {
         mv[i] = palabra[j];
         i++;
         j++;
      }
   }   
}
void stringWrite(){ //op1 = 4         
         short longitudd, i, j;
         longitudd = reg[ECX] & 0x0000FFFF;
         i = (tdds[reg[EDX] >> 16] >> 16) + (reg[EDX] & 0x0000FFFF);
         j = 0;

         if (longitudd == -1) { //imprimo hasta encontrar 0x00
            while (mv[i] != 0x00) {
               printf("%c",mv[i]);
               i++;
            }
         }
         else {
            while (j < longitudd && mv[i] != 0x00) { //cantidad limitada de caracteres
               printf("%c",mv[i]);
               i++;
               j++;
            }
         }
   
}
void clearScreen(){//op1 = 7
   system("cls");
}

void accesoDisco(){//op1 = 13 o D
   
   char static ultimoEstado;
   char operacion=getReg(reg[EAX],'h');
   unsigned char cantSectores=getReg(reg[EAX],'l');
   unsigned char cilindro=getReg(reg[ECX],'h');
   unsigned char cabeza=getReg(reg[ECX],'l');
   unsigned char sector=getReg(reg[EDX],'h');
   unsigned char disco=getReg(reg[EDX],'l');
   unsigned char cantidadCilindrosDisco;
   unsigned char cantidadCabezaDisco;
   unsigned char cantidadSectorDisco;
   int primerCeldaBuffer=reg[EBX];
   char cantSectoresTransferidos=0;
   unsigned int tamanoSector=0;
   unsigned char byte;
   int posicion;
   int cantidadDeBytesATransferir;
   
   FILE* arch;
   
   if (disco>discos.size-1){ //operacion invalida
      setReg(reg+EAX,'h',0x31);
      ultimoEstado=0x31;
      return;
   }
   
   //verifica que cilindro cabeza y sector sea valido
   arch = fopen(discos.discos[disco],"rb");
   if (arch == NULL){
      setReg(reg+EAX,'h',0xFF);
      return;
   }

   fseek(arch,33,SEEK_SET);
   
   //cilindro
   fread(&byte, sizeof(byte), 1, arch);
   cantidadCilindrosDisco=byte;
   if (cilindro>=byte){
      ultimoEstado=0x0B;
      setReg(reg+EAX,'h',0x0B);
      return;
   }
   //cabeza
   fread(&byte, sizeof(byte), 1, arch); 
   cantidadCabezaDisco=byte;
   if (cabeza>=byte){
      ultimoEstado=0x0C;
      setReg(reg+EAX,'h',0x0C);
      return;
   }
   //sector
   fread(&byte, sizeof(byte), 1, arch);
   cantidadSectorDisco=byte; 
   if (sector>=byte){
      ultimoEstado=0x0D;
      setReg(reg+EAX,'h',0x0D);
      return;
   }
   //tamanoDeCadaSector

   for (int i =0;i<4;i++){
      fread(&byte, sizeof(byte), 1, arch); ;
      tamanoSector|=byte<<(24-i*8);
   }
   fclose(arch);

   cantidadDeBytesATransferir = cantSectores*tamanoSector;

   posicion=512+(cilindro*cantidadCabezaDisco*cantidadSectorDisco*tamanoSector)+(cabeza*cantidadSectorDisco*tamanoSector)+(sector*tamanoSector);


   switch (operacion){

      case 0: //consulta ultimo estado       
         
         setReg(reg+EAX,'h',ultimoEstado);
         break; 
      
      case 2: //leer el disco
         
         arch = fopen(discos.discos[disco],"rb");

         fseek(arch, 0, SEEK_END);     // Ir al final del archivo
         long currSize = ftell(arch);  // Obtener el tamaño actual del archivo
         if (currSize < (posicion + cantidadDeBytesATransferir)) {    // Expandir el archivo hasta el byte posicion
            unsigned char zeroByte = 0;
            fseek(arch, 0, SEEK_END);  // Ir al final del archivo
            while (currSize < (posicion + cantidadDeBytesATransferir)) {
               fwrite(&zeroByte, sizeof(unsigned char), 1, arch);
               currSize++;
            }
         }
         fseek(arch,posicion,SEEK_SET);
         
         for (int i = 0;i <cantidadDeBytesATransferir; i++){
            
            if(fread(&byte, sizeof(byte), 1, arch)!=0){
               if ((primerCeldaBuffer&0xFFFF) >= (tamanoSegmento(primerCeldaBuffer))){ //segmentation fault
                  setReg(reg+EAX,'h',0x04);
                  ultimoEstado=0x04;
                  break;
               }
               mv[address(primerCeldaBuffer++)]=byte;
               cantSectoresTransferidos++;

            }
            else{
               setReg(reg+EAX,'h',0x04); // error en transferencia de lectura
               ultimoEstado=0x04;
               setReg(reg+EAX,'l',cantSectoresTransferidos);
               break;
            }

         }
         if (getReg(reg[EAX],'h') == 0x4){ //si fallo, sale de la funcion
            break;   
         }
         
         setReg(reg+EAX,'l',cantSectoresTransferidos);
         setReg(reg+EAX,'h',0);
         ultimoEstado=0;
         fclose(arch);
         break; 
      
      case 3: //escribir en el disco

         
         arch = fopen(discos.discos[disco],"rb+"); //modo rb+ permite editar sin crear archivo nuevo
         fseek(arch, 0, SEEK_END);     // Ir al final del archivo
         long currSize2 = ftell(arch);  // Obtener el tamaño actual del archivo
         if (currSize2 < (posicion + cantidadDeBytesATransferir)) {    // Expandir el archivo hasta el byte posicion
            unsigned char zeroByte = 0;
            fseek(arch, 0, SEEK_END);  // Ir al final del archivo
            while (currSize2 < (posicion + cantidadDeBytesATransferir)) {
               fwrite(&zeroByte, sizeof(unsigned char), 1, arch);
               currSize2++;
            }
         }
         fseek(arch,posicion,SEEK_SET);

         for (int i = 0;i <cantidadDeBytesATransferir; i++){           
               if ((primerCeldaBuffer&0xFFFF) >= (tamanoSegmento(primerCeldaBuffer))){ //segmentation fault
                  setReg(reg+EAX,'h',0xCC);
                  ultimoEstado=0xCC;
                  break;
               }
            byte = mv[address(primerCeldaBuffer++)];
            if(fwrite(&byte, sizeof(byte), 1, arch)!=0){
               cantSectoresTransferidos++;

            }
            else{
               setReg(reg+EAX,'h',0xCC); // error en transferencia de lectura
               ultimoEstado=0xCC;
               setReg(reg+EAX,'l',cantSectoresTransferidos);
               break;
            }
         }
         if (getReg(reg[EAX],'h') == 0xCC){ //si fallo, sale de la funcion
            break;   
         }
         setReg(reg+EAX,'l',cantSectoresTransferidos);
         setReg(reg+EAX,'h',0);
         ultimoEstado=0;
         fclose(arch);
         break; 
           
      case 8: // obtener los parametros del disco

         setReg(reg+ECX,'l',cantidadCilindrosDisco);
         setReg(reg+ECX,'l',cantidadCabezaDisco);
         setReg(reg+EDX,'h',cantidadSectorDisco);
         setReg(reg+EAX,'h',0);
         ultimoEstado=0;
         break; 

      
      default: //devuelve codigo 01 en AH, funcion invalida
         setReg(reg+EAX,'h',1);
         ultimoEstado=1;
         break;

   }

}

void gestionDinamicaSeg() {//op1 = 14 o E 
   char operacion=getReg(reg[EAX],'x');
   short tamSegmento=getReg(reg[ECX],'x');
   ///EBX es el puntero a la 1era celda del segmento
   int queSegmento = reg[EBX]>>16;
   int j = 0, k;

   if (operacion == 0) { //consulta segmento
      if (queSegmento < cantSegTot && queSegmento >= 0) { //el segmento existe
         setReg(reg+ECX,'x',(tdds[queSegmento] & 0x0000FFFF));
         setReg(reg+EAX,'x',0x0000); //operacion exitosa
      }
      else { //el segmento NO existe
         setReg(reg+EAX,'x',0x0031); //no existe el segmento
         setReg(reg+ECX,'x',0x0000);   
      }
   }
   else 
      if (operacion == 1) {
         if (tamSegmento + calculaTamanoMV() > m) {
            setReg(reg+EAX,'x',0x00CC); //no hay suficiente memoria
            reg[EBX] = -1;
         }
         else {
            if (cantSegTot < 8) {
               setReg(reg+EAX,'x',0x0000); //operacion exitosa
               while ((tdds[j]&0xFFFF)>0)
                  j++;

               tdds[j] |= tamSegmento; //guardo el tamaño
               reg[EBX]=j;
               reg[EBX]<<=16;

               for (k = j+1 ; k<8 ; k++)
                  tdds[k] = ((tdds[k] >> 16) + (tdds[j] & 0x0000FFFF)) << 16;
               cantSegTot++;   
            }
            else {
               reg[EBX] = -1;
               setReg(reg+EAX,'x',0xFFFF); //falla en la operacion
            }
         }   

      }
      else
         setReg(reg+EAX,'x',0x0001); //funcion invalida  
}

void breakpointDebugger(){ //op1 = 15 o F
   if (strcmp(debugger,"\0")){
      short tamanoMemoria = calculaTamanoMV(); // calcula el tamaño de la mv apartir del tdds
            
      FILE *arch = fopen(debugger,"wb");
      if (arch == NULL) {
         printf("Error al crear el archivo.\n");
            exit (-409);
         }
         char byte;
         byte='V';
         fwrite(&byte, sizeof(char), 1, arch);
         byte='M';
         fwrite(&byte, sizeof(char), 1, arch);
         byte='I';
         fwrite(&byte, sizeof(char), 1, arch);
         byte='2';
         fwrite(&byte, sizeof(char), 1, arch);
         byte='3';
         fwrite(&byte, sizeof(char), 1, arch);
         byte=1;
         fwrite(&byte, sizeof(char), 1, arch);
         byte=(tamanoMemoria>>8)&0x000000FF;
         fwrite(&byte,sizeof(char), 1, arch);
         byte=(tamanoMemoria)&0x000000FF;
         fwrite(&byte,sizeof(char), 1, arch);

      for (int i = 0 ; i < cantRegistros ;i++) { 
         for (int j=0; j<4; j++){
            byte=(reg[i]>>24-j*8)&0x0000FFFF;
            fwrite(&byte, sizeof(char), 1, arch); 
         }
      }
      for (int i = 0 ; i < TDDS_SIZE; i++ ) {
         for (int j=0; j<4 ;j++){
            byte=(tdds[i]>>24-j*8)&0x0000FFFF;
            fwrite(&byte, sizeof(char), 1, arch); 
         }
      }
      for (int i = 0 ; i<tamanoMemoria; i++ )
            fwrite(mv+i, sizeof(char), 1, arch); 

      fclose(arch);
      printf("Breakpoint alcanzado. Se generó el archivo de imagen.\n");

      // Esperar acciones del usuario
      char respuesta[256];
      
      printf("Presione 'q' para continuar o Enter para ejecutar la siguiente instrucción: \n");
      
      fgets(respuesta,256,stdin);
      while (strcmp(respuesta,"q\n") && strcmp(respuesta,"\n")){
         
         fgets(respuesta,256,stdin);
      }

         // Evaluar la acción del usuario
         if (!strcmp(respuesta,"q\n")) {
            breakpoint=0;
         } 
         else if(!strcmp(respuesta,"\n")) { // ingreso Enter
            breakpoint=1;
         }
 
   }  
}
char* creaArchivoDisco(char* param){

   FILE* arch = fopen(param,"rb"); // se fija si el archivo .vdd ya esta creado

      if(arch==NULL){ //si no esta creado, lo crea
         arch = fopen(param,"wb");
         unsigned char byte;
         struct tm *fecha;
         time_t tiempo = time(NULL);  // Obtiene el tiempo actual
         fecha = localtime(&tiempo);  // Convierte el tiempo en una estructura tm
         
         struct timeval segundos;
         gettimeofday(&segundos, NULL);
         
         //VDD0
         byte='V';
         fwrite(&byte, sizeof(char), 1, arch);
         byte='D';
         fwrite(&byte, sizeof(char), 1, arch);
         byte='D';
         fwrite(&byte, sizeof(char), 1, arch);
         byte='0';
         fwrite(&byte, sizeof(char), 1, arch);
         
         //version = 1
         byte=0;
         fwrite(&byte, sizeof(char), 1, arch);
         byte=0;
         fwrite(&byte, sizeof(char), 1, arch);
         byte=0;
         fwrite(&byte, sizeof(char), 1, arch);
         byte=1;
         fwrite(&byte, sizeof(char), 1, arch);
         
         //GUI (numero random identificador)
         for (char k=0; k<16;k++){
            byte=rand();
            fwrite(&byte, sizeof(char), 1, arch);
         }
        
        //fecha de creacion
         byte = 0x07; //el primer byte del año SIEMPRE va a ser 0x07 si estamos entre 1800 y 2050
         fwrite(&byte, sizeof(char), 1, arch);
         byte= fecha->tm_year+0x6C; //la funcion devuelve el año -6C (anda a saber por que)
         fwrite(&byte, sizeof(char), 1, arch);
         byte = fecha->tm_mon+1; //la funcion pone el mes entre 0 y 11
         fwrite(&byte, sizeof(char), 1, arch);
         byte = fecha->tm_mday;
         fwrite(&byte, sizeof(char), 1, arch);
         
         //hora de creacion
         byte = fecha->tm_hour;
         fwrite(&byte, sizeof(char), 1, arch);
         byte = fecha->tm_min;
         fwrite(&byte, sizeof(char), 1, arch);
         byte = fecha->tm_sec;
         fwrite(&byte, sizeof(char), 1, arch);
         byte = segundos.tv_usec/10000;
         fwrite(&byte, sizeof(char), 1, arch);

         //tipo
         byte = 1;
         fwrite(&byte, sizeof(char), 1, arch);

         //cantidad de cilindros
         byte =128;
         fwrite(&byte, sizeof(char), 1, arch);

         //cantidad de cabezas
         byte = 128;
         fwrite(&byte, sizeof(char), 1, arch);

         //cantidad de sectores
         byte = 128;
         fwrite(&byte, sizeof(char), 1, arch);

         
         //tamaño del sector en bytes //0x200 es 512
         byte = 0x00;
         fwrite(&byte, sizeof(char), 1, arch);
         fwrite(&byte, sizeof(char), 1, arch);
         byte = 0x2;
         fwrite(&byte, sizeof(char), 1, arch);
         byte = 0x00;
         fwrite(&byte, sizeof(char), 1, arch);
         
         
         // Buffer de ceros de 472 bytes
         unsigned char buffer[472] = {0};
         fwrite(buffer, sizeof(unsigned char), sizeof(buffer), arch);

     //    for (int y = 0; y< 262144 ;y++){ //le agrega 1gib -512 de espacio
       //     unsigned char buffer2[4096] = {0};
      //      fwrite(buffer, sizeof(unsigned char), sizeof(buffer2), arch);             
      //   }
        
      }
      fclose(arch);
   return param;
   }

void setReg(int* reg,char parte,int data){

   if (parte=='x'){
      data&=0xFFFF;
      *reg&=0xFFFF0000;
      *reg|=data;

   }
   else if (parte =='l'){
      data&=0xFF;
      *reg&=0xFFFFFF00;
      *reg|=data;

   }
   else if(parte=='h'){
      data<<=8;
      data&=0xFF00;
      *reg&=0xFFFF00FF;
      *reg|=data;
   }

}

int getReg(int reg,char parte){
   int respuesta;
   if (parte=='x'){
         respuesta=(reg&0xFFFF);
   }
   else if (parte =='l'){
      respuesta=(reg&0xFF);
   }
   else if(parte=='h'){
      respuesta= ((reg>>8)&0xFF);
   }
return respuesta;
}

int tamanoSegmento(int dir){ //recibe una direccion de memoria e informa el tamaño del segmento que apunta
return ((tdds[(dir>>16)&0xFFFF])&0xFFFF);


}