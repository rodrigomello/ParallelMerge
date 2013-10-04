//Adicionado bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "timer.h"
 
void MERGESORT(int *A,int e, int d);
 
/* merge two sorted arrays v1, v2 of lengths n1, n2, respectively */
int * merge(int * arr1, int tam1, int * arr2, int tam2){

  int * res; 
  int i = 0;//indice arr1
  int j = 0;//indice arr2
  int k;//indice res

  // res = tamanho 1 e 2 combinados
  res = (int *)malloc((tam1 + tam2) * sizeof(int));

  for (k = 0; k < tam1 + tam2; k++) {
    if (i >= tam1) {//caso o arr1 tenha terminado, complete com  o arr2
      res[k] = arr2[j];
      j++;
    }else if (j >= tam2) {//caso o arr2 tenha terminado, complete com  o arr1
      res[k] = arr1[i];
      i++;
    }else if (arr1[i] < arr2[j]) { // caso contrario, intercale os dois
      res[k] = arr1[i];
      i++;
    }else{
      res[k] = arr2[j];
      j++;
    }
  }

  return res;
}


//Função Principal
 
int main(){
    int *A, *B;// B eh o vetor local e A o original
    int N, N_local, tam;//quantidade de numeros a serem ordenados
    int i, tam2, *aux;
    float start, finish; //variaveis timer
    int my_rank, comm_sz; // variaveis MPI
    MPI_Status status;
    
    //Start MPI
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    do{
        //Caso seja o processo 0 lê a quantidade de numeros e os valores 
        if(my_rank == 0){ 
            scanf("%d",&N);
            if(N == 0)
                break;
        
            //calcula o tamanho do vetor para cada processo para alocar memoria suficiente            
            N_local = N/comm_sz; 
            if (N%comm_sz) 
                N_local++;           

            //alocando memoria para o vetor de valores
            A = (int*) malloc(sizeof(int) * (N_local*comm_sz));

            //lendo os valores
            for(i = 0; i < N; i++){
                scanf("%d",&A[i]);  
            }

            //preenchedo o final com 0, caso necessario
            for (i = N; i < N_local*comm_sz; i++){
                A[i] = 0;
            }
        }                 
        
        //marca o tempo de inicio
        GET_TIME(start);

        // envia por broadcast o tamanho do vetor
        MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);                 

        //calcula o tamanho do vetor para cada processo
        N_local = N/comm_sz; 
        if (N%comm_sz) 
            N_local++;

        //Alocando o vetor local
        B = (int*) malloc(sizeof(int) * (N));
        MPI_Scatter(A, N_local, MPI_INT, B, N_local, MPI_INT, 0, MPI_COMM_WORLD);

        //calcula o tamanho da sua parte
        if(N >= N_local * (my_rank + 1)){
            tam = N_local;
        }else{
            tam = N - N_local * my_rank;
        }  
        
        MERGESORT(B,0,tam-1);
        printf("processo- %d - tam: %d\n", my_rank, tam);
        for (i = 0; i < tam; i++){
            printf("%d ",B[i]);
        }

        //Junta os vetores locais
        for(i = 1; i < comm_sz; i = 2 * i){
            if (my_rank % (2 * i) != 0) {
                // quando não for multiplo de 2*i envia para o rank-i e sai 
                MPI_Send(B, tam, MPI_INT, my_rank-i, 0, MPI_COMM_WORLD);
                break;
            }
            
            // quando for multiplo junta com rank + i (se existir)
            if (my_rank + i < comm_sz) {

                // calcula o tamanho a ser recebido
                if(N >= tam * (my_rank + 2 * i)){
                    tam2 = tam * i;
                }else{
                    tam2 = N - (tam * (my_rank + i));
                }
                // receive other chunk
                aux = (int *)malloc(tam2 * sizeof(int));
                printf("\nReceive TAM %d\n",tam2 );
                
                
                MPI_Recv(aux, tam2, MPI_INT, my_rank + i, 0, MPI_COMM_WORLD, &status);
                
                // merge and free memory
                A = merge(B, tam, aux, tam2);
                //free(B);
                free(aux);
                B = A;
                tam = tam + tam2;
            }
               
        }
        if(my_rank == 0){
            for(i = 0; i < N; i++)
                printf("%d ",A[i]); 
           
            printf("\n");
            //marca o tempo de fim   
            GET_TIME(finish);
            printf("tempo de execucao: %f\n", finish - start);   
        }
    }while(N != 0);
 
       
    free(A);//liberando A
    MPI_Finalize(); //finalizando MPI

    return 0;
}
 
 //função mergesort
void MERGESORT(int *A,int e, int d){
    int m;
    int *B;//veotr auxiliar
    int i;
    int j;
    int k;
    int inverso;
       
    B = (int*) malloc((d+1)* sizeof(int));
    if(d > e ){
               
        m = (e + d)/2;//calculando o meio do vetor
        MERGESORT(A,e,m);//metade esquerda
        MERGESORT(A,m+1,d);//metade direita
                              
      
        //intercalando
        for(i = e; i <= m; i++)
            B[i] = A[i];
                       
                
                       
        for(j = m+1;j <= d;j++){
            inverso = d + m + 1 - j ;
            B[inverso] = A[j];//preenchendo inversamente
                        
        }    
                       
        i = e;//inicio do vetor
        j = d;//fim do vetor
               
               
        for(k = e; k <= d;k++){
            if(B[i] < B[j]){
                A[k] = B[i];
                i++;                
            }else{
                A[k] = B[j];
                j--;                       
            }                 
        }
 
               
    }
        
    free(B);//liberando B
}
