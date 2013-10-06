//Adicionado bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void sort(int *vector, int begin, int end){
    int m;
    int *B;
    int i;
    int j;
    int k;
    int inverso;
    B = (int*) malloc((end+1)* sizeof(int));
    if(end > begin ){
        m = (begin + end)/2;
        sort(vector,begin,m);
        sort(vector,m+1,end);
        for(i = begin; i <= m; i++){
            B[i] = vector[i];
        }
        for(j = m+1;j <= end;j++){
            inverso = end + m + 1 - j ;
            B[inverso] = vector[j];
        }    
        i = begin;
        j = end;
        for(k = begin; k <= end;k++){
            if(B[i] < B[j]){
                vector[k] = B[i];
                i++;                
            }else{
                vector[k] = B[j];
                j--;                       
            }                 
        }
    }       
    free(B);
}

/*Merge de dois vetores de mesmo tamanho*/
int * merge(int * vector1, int * vector2, int size){
    int * res;
    int i = 0;
    int j = 0;
    int k = 0;
    res = (int *) malloc((2 * size) * sizeof(int));
    while((i < size) && (j < size)){
        if(vector1[i] < vector2[j]){
            res[k] = vector1[i];
            i++;
            k++;
        }else{
            res[k] = vector2[j];
            j++;
            k++;
        }
    }
    while(i < size){
        res[k] = vector1[i];
        i++;
        k++;
    }
    while(j <size){
        res[k] = vector2[j];
        j++;
        k++;
    }
    return res;
}

int main (int argc, char* argv[]){
    int my_rank, procs;
    double startwtime = 0.0;
    double endwtime;
    int i, n, *vector;//Entradas
    int j, k;//índices de laços
    int *localVector, *localVector2;//Vetores locais
    int localSize, blocks, flag;
    /*Leitura do arquivo de entrada pode ser feito fora do MPI - SQN*/
    scanf("%d",&i);
    MPI_Init (&argc, &argv);/* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);/* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &procs);/* get number of processes */
    
        /*Lendo tamanho do vetor e o vetor*/
        if(my_rank == 0){
            scanf("%d", &n);
            vector = (int *) malloc(n *sizeof(int));
            for(j = 0; j < n; j++){
                scanf("%d", &vector[j]);
            }
        }
        
        /*Realiza i ordenações*/
        //for(k = 0; k < i; k++){            
            startwtime = MPI_Wtime();//Inicia o relógio
            localSize = n/procs;
            localVector = (int *) malloc(localSize * sizeof(int));
            MPI_Scatter(vector, localSize, MPI_INT, localVector, localSize, MPI_INT, 0, MPI_COMM_WORLD);
            sort(localVector, 0, localSize-1);
            /*Início Merge*/
            flag = 1;
            for(blocks = 1; blocks < procs; blocks = blocks * 2){
                if(flag){
                    if((my_rank/blocks) % 2){
                        MPI_Send(&localVector, blocks * localSize, MPI_INT, my_rank - blocks, 1, MPI_COMM_WORLD);
                        free(localVector);
                        flag = 0;
                    } else{
                        localVector2 = (int *) malloc((blocks * localSize) * sizeof(int));
                        MPI_Recv(&localVector2, blocks * localSize, MPI_INT, my_rank + blocks, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        localVector = merge(localVector, localVector2, blocks * localSize);
                        free(localVector2);
                    }
                }
            }
            /*Fim do Merge*/
            /*Exibindo resultados*/
            if(my_rank == 0){
                endwtime = MPI_Wtime();
                for(j = 0; j < n; j++){
                    printf("%d ", localVector[j]);
                }
                printf("\n");
                printf("%.3f\n", (endwtime - startwtime)*1000);
                scanf("%d", &i);
            }
            /*Fim dos resultados*/   
        //}
    
    MPI_Finalize();
    return 0;
}