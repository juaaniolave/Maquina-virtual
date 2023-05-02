#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define DS_SIZE 1024*16 //1 KiB * 16 ) KiB
#define CS_SIZE 1024*16
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



unsigned char* mv;
int reg[cantRegistros]={0}; //registros, variable global para que cualquier funcion pueda modificarlos
unsigned int tdds[TDDS_SIZE]; 

int main(int argc, char *argv[]) {

   char* extension=".vmx";
   char *nombre_archivo = "fibo.vmx";
   char version=0;
   int cantInstrucciones=0;

   void (*funciones[cantFunciones])(tppar,tppar)={NULL};
   inicializaFunciones(funciones);

   for (int i = 1; i < argc; i++) {
      char *param = argv[i];
      if (strstr(param, extension) != NULL) {
         nombre_archivo = param;
         break;
      }
   }
   if (nombre_archivo == NULL){
   printf("Por favor indique el nombre del archivo");
   exit(-4);
   }

   //RECUPERO TAMAÑO DEL MV

   int m = MV_SIZE;
   for (int i = 1; i < argc; i++) { // El primer argumento es el nombre del programa
       if (strncmp(argv[i], "m=", 2) == 0) {
           m = atoi(argv[i] + 2); // Extracción del valor de M como un entero
           
           break;
       }
   }
   //ASIGNO TAMAÑO AL MV
   mv = (char*)malloc(sizeof(char)*m);
   memset(mv, 0, m); //inicializa en 0 todo mv
   

   leeArchivoBinario(mv, &cantInstrucciones, nombre_archivo,m,&version); // lee el archivo binario y carga las instrucciones en el codesegment

// BUSCA PARAMETRO -d
   for (int i = 1; i < argc; i++) { // El primer argumento es el nombre del programa
       if (strcmp(argv[i], "-d") == 0) {
           dissasambly(mv, cantInstrucciones); // Ejecuta dissasambly
           break;
       }
   }


   inicializaRegistros(m,version);

   //programa principal
   unsigned short posMemA, posMemB;
   tpar auxA;
   int aux1,aux2;
   char treg1, treg2; //0 si los valores de registros son completos por ej EFX, otro valor si son EL, EC, o EX
   unsigned char codOp, tOpA,tOpB; //codigo de operacion, tipo op1, tipo op2
   tpar op1,op2; //operandos
   tppar pOp1, pOp2; //punteros a los operandos

   printf("\n");

   while (reg[IP] <= cantInstrucciones){

      treg1=treg2=op1=op2=aux1=0;

  //    printf("[%04X]  ",reg[IP]);

      tOpA = (mv[reg[IP]] >> 6) & 3; // asigna los bits 1100000
      tOpB = (mv[reg[IP]] >> 4) & 3; //asigna los bits 00110000

    //  printf("%02X ",cs[reg[IP]]);

      if (tOpB == 3){ // cod operacion uno o sin operandos

         if (tOpA == 3){  // cod operacion 1111 sin operandos
            codOp = mv[reg[IP]++];
            funciones[codOp](&cantInstrucciones,0);
            exit(1);
         }
         else { //cod op xx11 1 operando
            codOp = mv[reg[IP]++] & 63; // &00111111



         }
      }
      else{ //cod op 2 operandos
            codOp = mv[reg[IP]++] & 15; //&00001111


      }

      //asigno valores a los operandos dependiendo el tama�o

      if (tOpA == 0){  // operando apunta a direccion en memoria


      //   printf("%02X ",cs[reg[IP]]);

         auxA = ((tdds[((reg[mv[reg[IP]]])>>16)&0x0000FFFF])>>16)&0x0000FFFF; //valor del registro, por ejemplo lo que esta contenido en DS (0x00010000)
         auxA +=((reg[mv[reg[IP]++]])&0x0000FFFF);

         posMemA = mv[reg[IP]++];
         posMemA <<= 8;
      //   printf("%02X ",cs[reg[IP]]);

         posMemA |= mv[reg[IP]++];
       //  printf("%02X ",cs[reg[IP]]);

         posMemA += auxA; // offset += valor del reg, por ej [DS+10]

          if (!((tdds[1])>>16)&0x0000FFFF <= posMemA && posMemA < ((tdds[1]>>16)&0x0000FFFF+((tdds[1] & 0x0000FFFF)) - 4) ) { //En la segunda parte puede que sea modificado (el 4)
             printf("Error de segmentacion en DS\n");
             exit(-420);
          }

         op1 = 0; //limpio

         for (int j = 0; j < 4 ; j++){ //recorta y almacena las 4 celdas de memoria en op1
            op1 <<= 8;
            op1 |= mv[posMemA + j]; // primero es ds[posMem + 0]
         }

         //pOp1=&(ds[op1]);

         pOp1 =& op1;
      }


      else if (tOpA == 1){
       //  printf("%02X ",cs[reg[IP]]);
         op1 = mv[reg[IP]++];
         op1 = op1 <<  8;
     //    printf("%02X ",cs[reg[IP]]);
         op1 = op1 | mv[reg[IP]++];
         op1<<=16; op1>>=16;

         pOp1 =& op1;

      }
      else if (tOpA == 2){
      //   printf("%02X ",cs[reg[IP]]);
         treg1 = (mv[reg[IP]] >> 4) & 0b0011;
         op1   = mv[reg[IP]++] & 0b1111;

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
      //   printf("%02X ",cs[reg[IP]]);

         auxB = ((tdds[((reg[mv[reg[IP]]])>>16)&0x0000FFFF])>>16)&0x0000FFFF; //valor del registro, por ejemplo lo que esta contenido en DS (0x00010000)
          auxB +=((reg[mv[reg[IP]++]])&0x0000FFFF);

         posMemB = mv[reg[IP]++];
         posMemB <<= 8;
       //  printf("%02X ",cs[reg[IP]]);

         posMemB |= mv[reg[IP]++];
       //  printf("%02X ",cs[reg[IP]]);

         posMemB += auxB; // offset += valor del reg, por ej [DS+10]
         
          if (!((tdds[1])>>16)&0x0000FFFF <= posMemB && posMemB < ((tdds[1]>>16)&0x0000FFFF+((tdds[1] & 0x0000FFFF)) - 4) ) { //En la segunda parte puede que sea modificado (el 4)
             printf("Error de segmentacion en DS\n");
             exit(-420);
          }


         op2=0; //limpio
         for (int j = 0; j < 4 ; j++){ //recorta y almacena las 4 celdas de memoria en op1
            op2 <<= 8;
            op2 |= mv[posMemB + j]; // primero es ds[posMem + 0]
         }

         //pOp1=&(ds[op1]);

         pOp2 =& op2;
      }
      else if (tOpB == 1){
       //  printf("%02X ",cs[reg[IP]]);
         op2 = mv[reg[IP]++];
         op2 = op2 <<  8;
      //   printf("%02X ",cs[reg[IP]]);
         op2 = op2 | mv[reg[IP]++];
         op2<<=16; op2>>=16;


         pOp2 =& op2;
      }
      else
         if (tOpB == 2){
            //printf("%02X ",cs[reg[IP]]);
            treg2 = (mv[reg[IP]] >> 4) & 0b0011;
            op2   = mv[reg[IP]++] & 0b1111;
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

   //  printf("\t | \t");

   if(funciones[codOp] == NULL)
   {
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
               op1&=0x000000FF;
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
      if (tOpA == 0)

         { // tOpA == 0, de memoria
         //mem1 = (char)(0xFF & *pOp1);

         // ds[posMemA]     = (char)(0xFF000000 & *pOp1);
         // ds[posMemA + 1] = (char)(0x00FF0000 & *pOp1);
         // ds[posMemA + 2] = (char)(0x0000FF00 & *pOp1);
         // ds[posMemA + 3] = (char)(0x000000FF & *pOp1);
         mv[posMemA]     = (0xFF000000 & *pOp1)>>24;
         mv[posMemA + 1] = (0x00FF0000 & *pOp1)>>16;
         mv[posMemA + 2] = (0x0000FF00 & *pOp1)>>8;
         mv[posMemA + 3] = (0x000000FF & *pOp1);

      }
   if (codOp == 3) { //codOP 3 = SWAP
      if (tOpB == 0) {
         // ds[posMemB]     = (char)(0xFF000000 & *pOp2);
         // ds[posMemB + 1] = (char)(0x00FF0000 & *pOp2);
         // ds[posMemB + 2] = (char)(0x0000FF00 & *pOp2);
         // ds[posMemB + 3] = (char)(0x000000FF & *pOp2);
         mv[posMemB]     = (0xFF000000 & *pOp2)>>24;
         mv[posMemB + 1] = (0x00FF0000 & *pOp2)>>16;
         mv[posMemB + 2] = (0x0000FF00 & *pOp2)>>8;
         mv[posMemB + 3] = (0x000000FF & *pOp2);




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


  // printf("\n");
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
   char funciones[256][5];
   char registros[16][4];
   tpar op1,op2;


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
   strcpy(funciones[240],"STOP");

   strcpy(registros[0],"cs");
   strcpy(registros[1],"ds");
   //strcpy(registros[2],"");
   //strcpy(registros[3],"");
   //strcpy(registros[4],"");
   strcpy(registros[5],"IP");
   //strcpy(registros[6],"");
   //strcpy(registros[7],"");
   strcpy(registros[8],"CC");
   strcpy(registros[9],"AC");
   strcpy(registros[10],"EAX");
   strcpy(registros[11],"EBX");
   strcpy(registros[12],"ECX");
   strcpy(registros[13],"EDX");
   strcpy(registros[14],"EEX");
   strcpy(registros[15],"EFX");





   unsigned char codOp, tOpA,tOpB; //codigo de operacion, tipo op1, tipo op2
   int i=0;
   char quereg1,quereg2; ///AAAAAAAAAAA

   while ( i < cantidadInstrucciones){

   printf("[%04X]  ", i);

      tOpA=(cs[i]>>6)&3; // asigna los bits 1100000
      tOpB=(cs[i]>>4)&3; //asigna los bits 00110000

      printf("%02X ",cs[i]);

      if (tOpB ==3){ // cod operacion uno o sin operandos

         if (tOpA ==3){  // co d operacion 1111 sin operandos
         printf ("\t");
            codOp=cs[i++];
         }
         else { //cod op xx11 1 operando
            codOp=((cs[i++])&63);
            // &00111111



         }
      }
      else{ //cod op 2 operandos
            codOp=((cs[i++])&15);
             //&00001111


      }

      if (tOpA == 0){  // operando apunta a direccion en memoria
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
         op1 = op1 | cs[i++];
         op1<<=16; op1>>=16;

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

      if (tOpA+tOpB < 2)
         printf("\t | \t");
      else
         printf("\t\t |\t");
      printf ("%s \t\t",strlwr(funciones[codOp]));

      if (tOpA==0){
         // if((op1)&255!=0)
         //    printf("[%s + %d]\t",strlwr(registros[(op1>>16)&15]), (op1)&255);
         // else
         //    printf("[%s]\t",strlwr(registros[(op1>>16)&15]));
         if ((op1)&0x00FFFF){
         printf("[%s+%d], ",registros[(op1>>16)&0xFF],(op1)&0x00FFFF);
         if ((op1)&0x00FFFF < 10 || (op1)&0x00FFFF > -10){};
         
         }
         else
         printf("[%s],\t ",registros[(op1>>16)&0xFF]);
      }

      else if (tOpA==1){
         if(codOp >=49 && codOp <=55) //es algun jump tiene q mostrar en hexa
            printf("%X\t",op1);
         else if (codOp==48){
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
         // if((op1)&255!=0)
         //    printf("[%s + %d]\t",strlwr(registros[(op1>>16)&15]), (op1)&255);
         // else
         //    printf("[%s]\t",strlwr(registros[(op1>>16)&15]));
            if ((op2)&0x00FFFF)
         printf("[%s+%d],\t",registros[(op2>>16)&0xFF],(op2)&0x00FFFF);
            else
         printf("[%s],\t",registros[(op2>>16)&0xFF]);
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


      // if (tOpA==0){
      //    printf("[%s + %d] \t",registros[op1]);
      // }

      printf("\n");





   }
}


void MOV(tppar op1,tppar op2){ ///0
   //printf("%s",__func__);

   //no son partes cortas de registros
    *op1=*op2;

}
void ADD(tppar op1,tppar op2){ ///1
 //printf("%s",__func__);

     *op1+=*op2;

     if (*op1==0)
       reg[CC]=1;
    else if (*op1<0)
       reg[CC]=2;
      else
       reg[CC]=0;
 }
void SUB(tppar op1,tppar op2){ ///2
//printf("%s",__func__);

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
//printf("%s",__func__);
    tpar aux;

    aux  = *op1;
    *op1 = *op2;
    *op2 = aux;
}
void MUL(tppar op1,tppar op2) { ///4
//printf("%s",__func__);

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
//printf("%s",__func__);


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
//printf("%s",__func__);



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
//printf("%s",__func__);
   *op1 = *op1 << *op2;
}
void SHR(tppar op1,tppar op2) { ///8
//printf("%s",__func__);
   *op1 = *op1 >> *op2;
}
void AND(tppar op1,tppar op2) { ///9
//printf("%s",__func__);
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
//printf("%s",__func__);
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
//printf("%s",__func__);
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
//printf("%s",__func__);
 int i;
 unsigned int aux,k;

   if (*op1 == 1){ //scanf
      switch (reg[EAX]&0b0001111) {

         case 1: //interpreta decimal
            for (i=((tdds[reg[EDX]>>16]>>16)+reg[EDX]&0x0000FFFF);i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+(tdds[reg[EDX]>>16]>>16)+(reg[EDX]&0x0000FFFF);){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               scanf("%d",&aux); //guarda en aux para despues recortar
               k=24;
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  mv[i++]=((aux>>k)&0x000000FF);
                  k-=8;
               }
            }
         break;

         case 2: //intepreta caracter
            for (i=((tdds[reg[EDX]>>16]>>16)+reg[EDX]&0x0000FFFF);i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+(tdds[reg[EDX]>>16]>>16)+(reg[EDX]&0x0000FFFF);){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               scanf("%c",&aux); //guarda en aux para despues recortar
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  mv[i++]=(aux>>k)&0x000000FF;
                  k-=8;
               }
            }
         break;

         case 4: // interpreta octal
            for (i=((tdds[reg[EDX]>>16]>>16)+reg[EDX]&0x0000FFFF);i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+(tdds[reg[EDX]>>16]>>16)+(reg[EDX]&0x0000FFFF);){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               scanf("%o",&aux); //guarda en aux para despues recortar
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  mv[i++]=(aux>>k)&0x000000FF;
                  k-=8;
               }
            }
         break;

         case 8: // interpreta Hexa
            for (i=((tdds[reg[EDX]>>16]>>16)+reg[EDX]&0x0000FFFF);i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+(tdds[reg[EDX]>>16]>>16)+(reg[EDX]&0x0000FFFF);){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               scanf("%x",&aux); //guarda en aux para despues recortar
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  mv[i++]=(aux>>k)&0x000000FF;
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

   else if(*op1==2){ 

            for (i=((tdds[reg[EDX]>>16]>>16)+reg[EDX]&0x0000FFFF);i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+(tdds[reg[EDX]>>16]>>16)+(reg[EDX]&0x0000FFFF);){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH (cantidad de datos a leer)
              aux=0;
            printf("[%04X]:",i-(tdds[reg[EDX]>>16]>>16)+reg[EDX]&0x0000FFFF);
            if (reg[EAX]&0b10) //si tiene que imprimir caracter
                  printf ("'");

               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  
                  if (reg[EAX]&0b10){ //si tiene que imprimir caracter
                     
                     if (isprint(mv[i]))
                        printf("%c",mv[i]);
                     else 
                        printf(".");
                  }

                  aux=aux<<8;
                  aux=aux|mv[i++]; //guarda en aux para ir armando el int
               }
            if (reg[EAX]&0b10) //si tiene que imprimir caracter
                  printf (" ");
            if (reg[EAX]&0b1) 
             printf("#%d ",aux);

            if (reg[EAX]&0b100)
             printf("@%o ",aux);

            if (reg[EAX]&0b1000)
             printf("%%%X",aux);
            
            printf("\n");
            }
   }

   else {
      printf("\nError! Parametro invalido en funcion SYS\n");
      exit(-2);
   }

}
void JMP(tppar op1,tppar op2) { ///49
//printf("%s",__func__);

   reg[IP]=*op1;
}
void JZ(tppar op1,tppar op2) { ///50
//printf("%s",__func__);

   if (((reg[CC]>>30)&0b11)== 1)
      reg[IP]=*op1;
}
void JP(tppar op1,tppar op2) { ///51
//printf("%s",__func__);

   if (((reg[CC]>>30)&0b11)==0)
      reg[IP]=*op1;
}
void JN(tppar op1,tppar op2) { ///52
//printf("%s",__func__);

   if (((reg[CC]>>30)&0b11)==2)
      reg[IP]=*op1;
}
void JNZ(tppar op1,tppar op2) { ///53
//printf("%s",__func__);

   if (((reg[CC]>>30)&0b11)!=1)
      reg[IP]=*op1;
}
void JNP(tppar op1,tppar op2) { ///54
//printf("%s",__func__);

   if (((reg[CC]>>30)&0b11)!=0)
      reg[IP]=*op1;
}
void JNN(tppar op1,tppar op2) { ///55
//printf("%s",__func__);

   if (((reg[CC]>>30)&0b11)!=2)
      reg[IP]=*op1;
}
void LDL(tppar op1,tppar op2) { //56
//printf("%s",__func__);

   tpar aux=*op1;
   aux&=0x0000FFFF;
   reg[AC]&=0xFFFF0000;
   reg[AC]|=aux;

}
void LDH(tppar op1,tppar op2) { //57
//printf("%s",__func__);

   tpar aux=*op1;
   aux&=0x0000FFFF;
   aux<<=16;
   reg[AC]&=0x0000FFFF;
   reg[AC]|=aux;

}
void RND(tppar op1,tppar op2) { //58
//printf("%s",__func__);

reg[AC]=rand() % (*op1 + 1);
}
void NOT(tppar op1,tppar op2) { //59
//printf("%s",__func__);

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
}
void POP(tppar op1,tppar op2){
}
void CALL(tppar op1,tppar op2){
}

void STOP(tppar op1,tppar op2){
   //printf("%s",__func__);

   reg[IP] = *op1+1; //pone el ip en cant instrucciones +1


}
void RET(tppar op1,tppar op2){
}
void leeArchivoBinario(unsigned char mv[], int *cantInstrucciones, char *nombre_archivo, int m, char* version){




    FILE* arch= fopen(nombre_archivo,"rb");
    if (arch==NULL){
      printf("no se pudo abrir el archivo %s",nombre_archivo);
      exit(-4);
    }
    short espacioCode;

    int i = 0;
    unsigned char byte;

    fread(&byte, sizeof(byte), 1, arch);
    //  printf("%c",byte);
    fread(&byte, sizeof(byte), 1, arch);
    //  printf("%c",byte);
    fread(&byte, sizeof(byte), 1, arch);
   //   printf("%c",byte);
    fread(&byte, sizeof(byte), 1, arch);
   //   printf("%c",byte);
    fread(&byte, sizeof(byte), 1, arch);
   //   printf("%c ",byte);
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
   
    //  printf("%x ",*cantInstrucciones); --> quedo de antes (no lo saco por las dudas)
    tdds[0] |= espacioCode; //tamano del code segment
   // printf("code segment: %X \n",tdds[0]);

    tdds[1]   = espacioCode; // posicion del data segment
    tdds[1] <<= 16;
    tdds[1]  |= (DS_SIZE - espacioCode); //tamano del data segment
  //  printf("data segment: %X \n\n",tdds[1]);

   if (espacioCode>m){
      printf("Espacio en memoria insuficiente para cargar el CS, el espacio definido de memoria es %d",m);
      exit(-404);
      }

   while (fread(&byte, sizeof(byte), 1, arch)) {
         if(i+1>m){
            printf("Espacio en memoria insuficiente para cargar el CS, el espacio definido de memoria es %d",m);
            exit(-404);
         }
         mv[i++]=byte;

       // printf("%x ",CS[i++]);
   }
   if (i>espacioCode){
     printf("Error de segmentacion en CS");
     exit(-3);
   }
   *cantInstrucciones=i;
 //   printf("%d \n",*cantInstrucciones);
   }
   
   else if (*version==2){

      short memoriaOffset=0; 

      for (char k=0; k<5; k++){ //inicializa los tdds
         fread(&byte,sizeof(char),1,arch);
         espacioCode=byte;
         espacioCode<<=8;
         fread(&byte,sizeof(char),1,arch);
         espacioCode|=byte;

         tdds[k]  = memoriaOffset; 
         tdds[k] <<= 16;
         
         
         tdds[k] |= espacioCode;
        

         memoriaOffset+=espacioCode;
   }

      if (espacioCode>m){
         printf("Espacio en memoria insuficiente para cargar el CS, el espacio definido de memoria es %d",m);
         exit(-404);
         }

      while (fread(&byte, sizeof(byte), 1, arch)) {
            if(i+1>m){
               printf("Espacio en memoria insuficiente para cargar el CS, el espacio definido de memoria es %d",m);
            exit(-404);
         }
         mv[i++]=byte;

       // printf("%x ",CS[i++]);
   }
   if (i>espacioCode){
     printf("Error de segmentacion en CS");
     exit(-3);
   }
   *cantInstrucciones=i;
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


   else if (version == 2){
      reg[CS]=0x00000000;
      reg[KS]=0x00010000;
      reg[DS]=0x00020000;    
      reg[ES]=0x00030000;  
      reg[SS]=0x00040000;

   if (tdds[0]&0xFFFF<=0){ //si el tamaño del segmento es menor a 0, el reg que corresponde queda con -1 y se corren todos los tdds para arriba
      reg[CS]=-1;
      reg[KS]-=0x00010000;
      reg[DS]-=0x00010000;    
      reg[ES]-=0x00010000;  
      reg[SS]-=0x00010000;

      tdds[0]=tdds[1];
      tdds[1]=tdds[2];
      tdds[2]=tdds[3];
      tdds[3]=tdds[4];
   }

   if (tdds[1]&0xFFFF<=0){ //si el tamaño del segmento es menor a 0, el reg que corresponde queda con -1 y se corren todos los tdds para arriba
      reg[KS]=-1;
      reg[DS]-=0x00010000;    
      reg[ES]-=0x00010000;  
      reg[SS]-=0x00010000;

      tdds[1]=tdds[2];
      tdds[2]=tdds[3];
      tdds[3]=tdds[4];
   }
   if (tdds[2]&0xFFFF<=0){ //si el tamaño del segmento es menor a 0, el reg que corresponde queda con -1 y se corren todos los tdds para arriba
      reg[DS]=-1;
      reg[ES]-=0x00010000;  
      reg[SS]-=0x00010000;

      tdds[2]=tdds[3];
      tdds[3]=tdds[4];
   }
   if (tdds[3]&0xFFFF<=0){ //si el tamaño del segmento es menor a 0, el reg que corresponde queda con -1 y se corren todos los tdds para arriba
      reg[ES]=-1;
      reg[SS]-=0x00010000;

      tdds[3]=tdds[4];
   }

   if (tdds[4]&0xFFFF<=0) //si el tamaño del segmento es menor a 0, el reg que corresponde queda con -1 y se corren todos los tdds para arriba
      reg[SS]=-1;  
   }

      reg[SP]=reg[SS] + tdds[reg[SS]>>16]&0xFFFF; //inicio del StackSegment + offset (el puntero va al final del segmento )
      reg[BP]=reg[SP];
}