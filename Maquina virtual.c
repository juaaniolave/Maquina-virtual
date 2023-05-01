#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define dsSize 1024*16 //1 KiB * 16 ) KiB
#define csSize 1024*16
#define cantRegistros 16
#define tddsSize 8
#define cantFunciones 255
#define CS 0
#define DS 1
#define IP 5
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

void leeArchivoBinario(unsigned char[], int* ,char*);
void inicializaRegistros(unsigned char[], unsigned char[], unsigned char[]);

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
void STOP(tppar,tppar);


int reg[cantRegistros]={0}; //registros, variable global para que cualquier funcion pueda modificarlos
unsigned char ds[dsSize]; // datasegment
int  tdds[tddsSize]; ///

int main(int argc, char *argv[]) {

   //char *nombre_archivo = argv[1]; // descomentar para que funcione la lectura por consola
   char *nombre_archivo = "fibo.vmx"; //comentar para que funcion la lectura por consola

   unsigned int tdds[tddsSize]; //Tabla de descriptores de segmentos
   unsigned char cs[csSize]; // codesegment
   int cantInstrucciones=0; //tama�o logico de cs

   void (*funciones[cantFunciones])(tppar,tppar)={NULL};
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
   funciones[240]=STOP;


   leeArchivoBinario(cs, &cantInstrucciones, nombre_archivo); // lee el archivo binario y carga las instrucciones en el codesegment

   //if (argc>2)// descomentar para que funcione la lectura por consola
   // if (!strcmp(argv[2],"[-d]")) //descomentar para que funcione la lectura por consola
         dissasambly(cs, cantInstrucciones);


   //programa principal
   short posMemA;
   tpar auxA;
   int aux;
   char treg1, treg2; //0 si los valores de registros son completos por ej EFX, otro valor si son EL, EC, o EX
   unsigned char codOp, tOpA,tOpB; //codigo de operacion, tipo op1, tipo op2
   tpar op1,op2; //operandos
   tppar pOp1, pOp2; //punteros a los operandos

   printf("\n");

   while (reg[IP] <= cantInstrucciones){

      treg1=treg2=op1=op2=aux=0;

  //    printf("[%04X]  ",reg[IP]);

      tOpA=(cs[reg[IP]]>>6)&3; // asigna los bits 1100000
      tOpB=(cs[reg[IP]]>>4)&3; //asigna los bits 00110000

    //  printf("%02X ",cs[reg[IP]]);

      if (tOpB ==3){ // cod operacion uno o sin operandos

         if (tOpA ==3){  // cod operacion 1111 sin operandos
            codOp=cs[reg[IP]++];
            funciones[codOp](&cantInstrucciones,0);
            exit(1);
         }
         else { //cod op xx11 1 operando
            codOp=cs[reg[IP]++]&63; // &00111111



         }
      }
      else{ //cod op 2 operandos
            codOp=cs[reg[IP]++]&15; //&00001111


      }

      //asigno valores a los operandos dependiendo el tama�o

      if (tOpA == 0){  // operando apunta a direccion en memoria


      //   printf("%02X ",cs[reg[IP]]);

         auxA = (reg[cs[reg[IP]++]]); //valor del registro, por ejemplo lo que esta contenido en DS (0)

         posMemA = cs[reg[IP]++];
         posMemA <<= 8;
      //   printf("%02X ",cs[reg[IP]]);

         posMemA |= cs[reg[IP]++];
       //  printf("%02X ",cs[reg[IP]]);

         posMemA+=auxA; // offset += valor del reg, por ej [DS+10]

          if (!(-1 < posMemA && posMemA < ((tdds[1] & 0x00FF) - 4) )) { //En la segunda parte puede que sea modificado (el 4)
             printf("Error: Fallo de segmento\n");
             exit(1);
          }

         op1=0; //limpio
         for (int j = 0;j<4;j++){ //recorta y almacena las 4 celdas de memoria en op1
            op1<<4;
            op1|=ds[posMemA+j]; // primero es ds[posMem + 0]
         }

         //pOp1=&(ds[op1]);

         pOp1=&op1;
      }


      else if (tOpA == 1){
       //  printf("%02X ",cs[reg[IP]]);
         op1 = cs[reg[IP]++];
         op1 = op1 <<  8;
     //    printf("%02X ",cs[reg[IP]]);
         op1 = op1 | cs[reg[IP]++];

         pOp1=&op1;

      }
      else if (tOpA == 2){
      //   printf("%02X ",cs[reg[IP]]);
         treg1 = (cs[reg[IP]] >> 4) & 0b0011;
         op1   = cs[reg[IP]++] & 0b1111;

         aux=op1;

         switch (treg1) {
            //nada, es el completo, EAX
            case 0b00: op1 = reg[op1];
               break;
            //Low register, AL
            case 0b01: op1 = (reg[op1] & 0b0000000000001111);
               break;
            //High register, AH
            case 0b10: op1 = ((reg[op1] & 0x0000000011110000) >> 8);
               break;
            //Mitad del registro, AX
            case 0b11: op1 = (reg[op1] & 0x000000011111111);
               break;
         }



         pOp1=&op1;
      }

      if (tOpB == 0){
         int auxB; short posMemB;
      //   printf("%02X ",cs[reg[IP]]);

         auxB = (reg[cs[reg[IP]++]]); //valor del registro, por ejemplo lo que esta contenido en DS (0)

         posMemB = cs[reg[IP]++];
         posMemB <<= 8;
       //  printf("%02X ",cs[reg[IP]]);

         posMemB |= cs[reg[IP]++];
       //  printf("%02X ",cs[reg[IP]]);

         posMemB+=auxB; // offset += valor del reg, por ej [DS+10]

         op2=0; //limpio
         for (int j = 0;j<4;j++){ //recorta y almacena las 4 celdas de memoria en op1
            op2<<4;
            op2|=ds[posMemB+j]; // primero es ds[posMem + 0]
         }

         //pOp1=&(ds[op1]);

         pOp2=&op2;
      }
      else if (tOpB == 1){
       //  printf("%02X ",cs[reg[IP]]);
         op2 = cs[reg[IP]++];
         op2 = op2 <<  8;
      //   printf("%02X ",cs[reg[IP]]);
         op2 = op2 | cs[reg[IP]++];


         pOp2=&op2;
      }
      else if (tOpB == 2){
       //  printf("%02X ",cs[reg[IP]]);
         treg2= (cs[reg[IP]]>>4)&0b0011;
         op2 = cs[reg[IP]++]&0b1111;

         switch (treg2) {
            //nada, es el completo, EAX
            case 0b00: op2 = (reg[op2]);
               break;
            //Low register, AL
            case 0b01: op2 = (reg[op2] & 0x000000000001111);
               break;
            //High register, AH
            case 0b10: op2 = ((reg[op2] & 0x0000000011110000) >> 8);
               break;
            //Mitad del registro, AX
            case 0b11: op2 = (reg[op2] & 0x00000000111111111);
               break;
         }
         pOp2=&op2;

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
            case 0b00: reg[aux] = *pOp1;
               break;
            //Low register, AL
            case 0b01:
               reg[aux] &= 0xFFF0;
               reg[aux] |= op1;
            break;
            //High register, AH
            case 0b10:
               reg[aux] &= 0xFF0F;
               *pOp1   <<= 8;
               reg[aux] |= *pOp1;
            break;
            //Mitad del registro, AX
            case 0b11:
               reg[aux] &= 0xFF00;
               reg[aux] |= *pOp1;
            break;
         }
   }
   else
      if (tOpA == 0)

         { // tOpA == 0, de memoria
         //mem1 = (char)(0xFF & *pOp1);

         ds[posMemA]     = (char)(0xFF000000 & *pOp1);
         ds[posMemA + 1] = (char)(0x00FF0000 & *pOp1);
         ds[posMemA + 2] = (char)(0x0000FF00 & *pOp1);
         ds[posMemA + 3] = (char)(0x000000FF & *pOp1);
      }

  // printf("\n");
   }


    return 0;
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

   strcpy(registros[0],"CS");
   strcpy(registros[1],"DS");
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

      //asigno valores a los operandos dependiendo el tama�o

      // for (int j=tOpA;j<3;j++){
      //    printf("%02X ",cs[i++]);
      // }
      // for (int j=tOpB;j<3;j++){
      //    printf("%02X ",cs[i++]);
      // }

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
      printf ("%s \t",strlwr(funciones[codOp]));

      if (tOpA==0){
         // if((op1)&255!=0)
         //    printf("[%s + %d]\t",strlwr(registros[(op1>>16)&15]), (op1)&255);
         // else
         //    printf("[%s]\t",strlwr(registros[(op1>>16)&15]));
         printf("[%d],\t",(op1)&255);
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

            if (tOpB==0){
         // if((op1)&255!=0)
         //    printf("[%s + %d]\t",strlwr(registros[(op1>>16)&15]), (op1)&255);
         // else
         //    printf("[%s]\t",strlwr(registros[(op1>>16)&15]));
         printf("[%d]\t",(op2)&255);
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
   // printf("%s",__func__);

   //no son partes cortas de registros
    *op1=*op2;

}

 void ADD(tppar op1,tppar op2){ ///1
 // printf("%s",__func__);

     *op1+=*op2;

     if (*op1==0)
       reg[CC]=1;
    else if (*op1<0)
       reg[CC]=2;
      else
       reg[CC]=0;
 }

void SUB(tppar op1,tppar op2){ ///2
// printf("%s",__func__);

    *op1 -= *op2;

   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;

}
void SWAP(tppar op1,tppar op2){ ///3
// printf("%s",__func__);


    tpar aux;
    aux=*op1;
    *op1=*op2;
    *op2=aux;

}
void MUL(tppar op1,tppar op2) { ///4
// printf("%s",__func__);

   (*op1) *= (*op2);

   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;
}
void DIV(tppar op1,tppar op2) { ///5
// printf("%s",__func__);


   if (*op2 == 0) {
      printf("Error: Division por cero\n");
      exit(1);
   }

   if (*op2){
      reg[AC] = (*op1) % *op2; ///el resto de la division se guarda en el registro AC
      (*op1) /= (*op2);
   }

   if (*op1==0)  //modifica valores de Z y N
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;


}
void CMP(tppar op1,tppar op2) { ///6
// printf("%s",__func__);



   tpar aux=*op1;
   aux -= *op2;


   if (aux==0)
      reg[CC]=1;
   else if (aux<0)
      reg[CC]=2;
   else
      reg[CC]=0;
}
void SHL(tppar op1,tppar op2) { ///7
// printf("%s",__func__);
   *op1 = *op1 << *op2;
}
void SHR(tppar op1,tppar op2) { ///8
// printf("%s",__func__);
   *op1 = *op1 >> *op2;
}
void AND(tppar op1,tppar op2) { ///9
// printf("%s",__func__);
   *op1 = (*op1) & (*op2);

   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;
}
void OR(tppar op1,tppar op2) { ///10 o A
// printf("%s",__func__);
    *op1 = (*op1) | (*op2);

   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;
}
void XOR(tppar op1,tppar op2) { ///11 o B
// printf("%s",__func__);
    *op1 = (*op1) ^ (*op2);


   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;
}


void SYS(tppar op1,tppar op2) { ///48  terminado?
// printf("%s",__func__);
 int i;
 unsigned int aux,k;


   if (*op1 == 1){ //scanf
   printf("\ningrese algun valor: ");
      switch (reg[EAX]&0b0000011) {

         case 1: //interpreta decimal
            for (i=reg[EDX];i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+reg[EDX];){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               scanf("%d",&aux); //guarda en aux para despues recortar
               k=24;
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  ds[i++]=((aux>>k)&0x000000FF);
                  k-=8;
               }
            }
         break;

         case 2: //intepreta caracter
            for (i=reg[EDX];i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+reg[EDX];){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               scanf("%c",&aux); //guarda en aux para despues recortar
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  ds[i++]=(aux>>k)&0x000000FF;
                  k-=8;
               }
            }
         break;

         case 4: // interpreta octal
            for (i=reg[EDX];i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+reg[EDX];){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               scanf("%o",&aux); //guarda en aux para despues recortar
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  ds[i++]=(aux>>k)&0x000000FF;
                  k-=8;
               }
            }
         break;

         case 8: // interpreta Hexa
            for (i=reg[EDX];i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+reg[EDX];){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH*CL+EDX (direccion de memoria final)
               scanf("%x",&aux); //guarda en aux para despues recortar
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  ds[i++]=(aux>>k)&0x000000FF;
                  k-=8;
               }
            }
         break;

      default:
         printf("\nError, valor de operacion para SYS invalido\n op1 vale %d y al vale %d",*op1,reg[EAX]&0b0000011);
         exit(-3);
         break;
      }

   }

   else if(*op1==2){ //printf
      printf("\n\n");

      switch (reg[EAX]&3) {

         case 1: //interpreta decimal
            for (i=reg[EDX];i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+reg[EDX];){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH (cantidad de datos a leer)
              aux=0;
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  aux=aux<<4;
                  aux=aux|ds[i++]; //guarda en aux para despues recortar
               }
             printf("%d",aux);
            }
         break;

         case 2: //interpreta decimal
            for (i=reg[EDX];i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+reg[EDX];){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH (cantidad de datos a leer)
              aux=0;
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  aux=aux<<4;
                  aux=aux|ds[i++]; //guarda en aux para despues recortar
               }
             printf("%c",aux);
            }
         break;

         case 4: // interpreta octal
            for (i=reg[EDX];i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+reg[EDX];){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH (cantidad de datos a leer)
              aux=0;
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  aux=aux<<4;
                  aux=aux|ds[i++]; //guarda en aux para despues recortar
               }
             printf("%o",aux);
            }
         break;

         case 8: // interpreta Hexa
            for (i=reg[EDX];i<((reg[ECX]>>8)&0xFF)*(reg[ECX]&0xFF)+reg[EDX];){ //i=EDX (direccion de memoria) aumenta en CL (cantidad de celdas por dato) hasta CH (cantidad de datos a leer)
              aux=0;
               for (int j = 0;j<((reg[ECX]>>8)&0xFF);j++){ //recorta y almacena
                  aux=aux<<4;
                  aux=aux|ds[i++]; //guarda en aux para despues recortar
               }
             printf("%x",aux);
            }
         break;

      default:
         printf("\nError, valor de operacion para SYS invalido\n op1 vale %d y al vale %d",*op1,reg[EAX]&0b0000011);
         exit(-3);
         break;
      }
   }

   else {
      printf("\nError! Parametro invalido en funcion SYS\n");
      exit(-2);
   }

}
void JMP(tppar op1,tppar op2) { ///49
// printf("%s",__func__);

   reg[IP]=*op1;
}
void JZ(tppar op1,tppar op2) { ///50
// printf("%s",__func__);

   if (reg[CC]== 1)
      reg[IP]=*op1;
}
void JP(tppar op1,tppar op2) { ///51
// printf("%s",__func__);

   if (reg[CC]==0)
      reg[IP]=*op1;
}
void JN(tppar op1,tppar op2) { ///52
// printf("%s",__func__);

   if (reg[CC]==2)
      reg[IP]=*op1;
}
void JNZ(tppar op1,tppar op2) { ///53
// printf("%s",__func__);

   if (reg[CC]!=1)
      reg[IP]=*op1;
}
void JNP(tppar op1,tppar op2) { ///54
// printf("%s",__func__);

   if (reg[CC]!=0)
      reg[IP]=*op1;
}
void JNN(tppar op1,tppar op2) { ///55
// printf("%s",__func__);

   if (reg[CC]!=2)
      reg[IP]=*op1;
}
void LDL(tppar op1,tppar op2) { //56
// printf("%s",__func__);

}
void LDH(tppar op1,tppar op2) { //57
// printf("%s",__func__);

}
void RND(tppar op1,tppar op2) { //58
// printf("%s",__func__);

reg[AC]=rand();
}
void NOT(tppar op1,tppar op2) { //59
// printf("%s",__func__);

   *op1= ~*op1;

   if (*op1==0)
      reg[CC]=1;
   else if (*op1<0)
      reg[CC]=2;
   else
      reg[CC]=0;

}
void STOP(tppar op1,tppar op2){
   // printf("%s",__func__);

   reg[IP] = *op1+1; //pone el ip en cant instrucciones +1


}

void leeArchivoBinario(unsigned char cs[], int *cantInstrucciones, char *nombre_archivo){




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
  //    printf("%d ",byte);

    printf("\n \n");
    tdds[0]   = 0x0000; //posicion del code segment
    tdds[0] <<= 16;
    fread(&espacioCode, sizeof(short), 1, arch);
    //  printf("%x ",*cantInstrucciones); --> quedo de antes (no lo saco por las dudas)
    tdds[0] |= espacioCode; //tamano del code segment
   // printf("code segment: %X \n",tdds[0]);

    tdds[1]   = 0x0001; //posicion del data segment
    tdds[1] <<= 16;
    tdds[1]  |= (csSize - espacioCode); //tamano del data segment
  //  printf("data segment: %X \n\n",tdds[1]);

    while (fread(&byte, sizeof(byte), 1, arch)) {

        cs[i++]=byte;

       // printf("%x ",CS[i++]);
    }
   *cantInstrucciones=i;
 //   printf("%d \n",*cantInstrucciones);
    fclose(arch);
}

//void inicializaRegistros(unsigned char CS[], unsigned char TDDS[], unsigned char Reg[]){



//}

//    else{ //aca empieza el bardo
//       switch (treg1)
//       {
//          case 0: //el primero no es registro o no esta cortado = EAX
//             switch (treg2) //el segundo reg  es la parte baja = EL
//             {
//                case 1:
//                   *op1=*op2&255;
//                break;
//                case 2:  //el segundo reg es la parte alta = EH
//                   *op1=(*op2>>8)&0x000000FF;
//                break;
//                case 3: //el segundo reg es toda la mitad = EX
//                      *op1=*op2&0x0000FFFF;
//                break;
//             }
//          break;
//          case 1: //treg1
//             *op1&=0xFFFFFF00; // limpia los dos bytes menos significativos y luego hace OR al final con el valor que corresponda en op2 (000000FF)
//             switch (treg2)
//             {
//                case 0:
//                   *op2&=0x0000000FF;// trunca
//                break;

//                case 1: //el segundo es la parte baja EC
//                   *op2&=0x0000000FF;  //limpia
//                break;

//                case 2:  //el segundo es la parte alta EL
//                   (*op2>>8)&0x0000FF00; // y lo corro
//                break;

//                case 3: //el segundo es toda la mitad EX
//                   *op2&=0x0000000FF; // trunca
//                break;
//             }

//             *op1|=*op2;
//          break;

//          case 2: //treg1

//             *op1&=0xFFFF00FF; // limpia el byte y luego hace OR al final con el valor que corresponda en op2 (0000FF00)

//             switch (treg2){
//                case 0:
//                   *op2&=0x0000000FF; // trunca
//                   *op2<<= 8;
//                break;

//                case 1: //el segundo es la parte baja EL
//                   *op2&=0x0000000FF; // limpia
//                   *op2<<= 8;
//                break;

//                case 2:  //el segundo es la parte alta EH
//                   (*op2)&=0x0000FF00;
//                break;

//                case 3: //el segundo es toda la mitad EX
//                   *op2&=0x0000000FF; // trunca
//                   *op2<<= 8;
//                break;
//             }
//             *op1|=*op2;
//          break;

//          case 3: //treg1

//             switch (treg2){
//                case 0:
//                   *op2&=0x00000FFFF; // trunca
//                break;

//                case 1: //el segundo es la parte baja EL
//                   *op2&=0x0000000FF; // limpia
//                break;

//                case 2:  //el segundo es la parte alta EH
//                   (*op2)&=0x0000FF00;
//                   *op2>>=8;
//                break;

//                case 3: //el segundo es toda la mitad EX
//                   *op2&=0x00000FFFF; // trunca
//                break;
//             }
//             *op1|=*op2;
//          break;
//       }
//    }
// }
