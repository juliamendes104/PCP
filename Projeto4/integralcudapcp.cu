#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

__global__ void calculoTrapezios(long X_intervalos, double X_inicial, double X_incremento, long Y_intervalos, double Y_inicial,
                                 double Y_incremento, double *resultados, long IntervaloPorThread){

    //Declarar uma variável compartilhada para soma total dentro de cada bloco
    __shared__ double soma_total_bloco[512];
    double x1, x2, soma_parcial = 0.0;

    if (threadIdx.x < 512) {
        soma_total_bloco[threadIdx.x] = 0.0;
    }

    //Intervalo de i para esta thread
    long iInicial = (threadIdx.x + blockIdx.x * 512) * IntervaloPorThread;
    long iFinal = min((X_intervalos*Y_intervalos), iInicial + IntervaloPorThread);

    //Sincronizar todos os threads antes de começar a computacao
    __syncthreads();

    for(long i=iInicial; i<iFinal; i++){
        soma_parcial = 0.0;
        long indice_x = i / Y_intervalos;
        long indice_y = i % Y_intervalos;

        x1 = X_inicial + (indice_x * X_incremento);
        x2 = X_inicial + (indice_x * X_incremento) + X_incremento;

        double Y1 = Y_inicial + (indice_y * Y_incremento);
        double Y2 = Y_inicial + (indice_y * Y_incremento) + Y_incremento;
        
        double g1 = sin(x1*x1 + Y1*Y1);
        double g2 = sin(x1*x1 + Y2*Y2);
        soma_parcial += (g1 + g2) * (Y2 - Y1)/2.0;

        g1 = sin(x2*x2 + Y1*Y1);
        g2 = sin(x2*x2 + Y2*Y2);
        soma_parcial += (g1 + g2) * (Y2 - Y1)/2.0;

     
        soma_total_bloco[threadIdx.x] += (soma_parcial * X_incremento)/2.0;
    }
    

    __syncthreads();

    // A thread 0 vai escrever o valor final na posição do vetor de resultados
    if (threadIdx.x == 0) {
        double soma_bloco = 0.0;
        for(long i=0; i<512; i++){
            soma_bloco += soma_total_bloco[i];
        }
        resultados[blockIdx.x] = soma_bloco;
    }
}

int main(){

    double soma_total = 0.0;
    double X_final = 1.5;
    double X_inicial = 0;
    long X_intervalos = 100000;
    double X_incremento = (X_final - X_inicial) / X_intervalos;

    double Y_final = 1.5;
    double Y_inicial = 0;
    long Y_intervalos = 100000;
    double Y_incremento = (Y_final - Y_inicial) / Y_intervalos;

    //Configuracao do kernel
    dim3 numBlocos(1000);//Quantidade de blocos
    dim3 threadsPorBloco(512);//Quantidade de threads por bloco

    double *resultados_host = (double *)malloc(numBlocos.x * sizeof(double));
    //Inicializando os resultados_host com 0.0
    memset(resultados_host, 0, numBlocos.x * sizeof(double));

    long IntervaloPorThread = (X_intervalos * Y_intervalos + (numBlocos.x * threadsPorBloco.x - 1)) / (numBlocos.x * threadsPorBloco.x);
    printf("Intervalo: %ld\n", IntervaloPorThread);

    //Alocar memoria para os resultados
    double *resultados;
    cudaMalloc(&resultados, numBlocos.x * sizeof(double));
    cudaMemcpy(resultados, resultados_host, numBlocos.x * sizeof(double), cudaMemcpyHostToDevice);

    //Lancar o kernel
    calculoTrapezios<<<numBlocos, threadsPorBloco>>>(X_intervalos, X_inicial, X_incremento, Y_intervalos, Y_inicial,
                                                     Y_incremento, resultados, IntervaloPorThread);

    cudaMemcpy(resultados_host, resultados, numBlocos.x * sizeof(double), cudaMemcpyDeviceToHost);

    for (long i=0; i<numBlocos.x; i++) {
        soma_total += resultados_host[i];
    }

    printf("A soma total da integral é: %f\n", soma_total);
    //Liberar memoria
    cudaFree(resultados);

    return 0;
}