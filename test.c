int main (){
    for (int i=0;i<10;i++)
    prueba();

    return 0;
}


void prueba(){

    int static counter=0;

    printf("%d ", counter++);

}