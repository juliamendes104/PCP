#include <mpi.h> //Biblioteca para comunicacao MPI
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//Calcular a area do trapezio entre dois pontos
double trapezio(double x1, double y1, double y2){
	//g(x,y) = sin(x^2 + y^2)
	double g1 = sin(x1*x1 + y1*y1);
	double g2 = sin(x1*x1 + y2*y2);
	
	//Retornar a area conforme regra dos trapezios
	return ((g1 + g2) * (y2 - y1)/2.0);
}

int main(int argc, char **argv){
	int rank, size;
	
	double soma_total = 0;//Processo raiz armazena soma total
	double soma_parcial = 0;//Cada processo armazena a sua soma parcial
	double x1, x2;
	
	//Intervalos e incrementos de X e Y
	const double X_final = 1.5;
	const double X_inicial = 0;
	const long X_intervalos = 1000;
	const double X_incremento = (X_final - X_inicial) / X_intervalos;
	
	const double Y_final = 1.5;
	const double Y_inicial = 0;
	const long Y_intervalos = 10000;
	const double Y_incremento = (Y_final - Y_inicial) / Y_intervalos;
	
	//Delimitar o escopo da comunicacao
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);//Retorna o id do processo executando na variavel rank
	MPI_Comm_size(MPI_COMM_WORLD, &size);//Retorna o numero de processos de um escopo na variavel size
	
	//Definir intervalo de indices para cada processo
	long inicial = rank * ((X_intervalos*Y_intervalos)/size);
	long final = inicial + ((X_intervalos*Y_intervalos)/size);
	
	//Laco que distribui os trabalhos
	for(long i=inicial; i<final; i++){
		double soma_local = 0;
		
		//Indices para X e Y
		long indice_x = i/Y_intervalos;
		long indice_y = i%Y_intervalos;
		
		//Valores de X para trapezio
		x1 = X_inicial + (indice_x * X_incremento);
		x2 = X_inicial + (indice_x * X_incremento) + X_incremento;
		
		//Soma local da area dos trapezios
		soma_local += trapezio(x1, Y_inicial + (indice_y * Y_incremento), Y_inicial + (indice_y * Y_incremento) + Y_incremento);
		soma_local += trapezio(x2, Y_inicial + (indice_y * Y_incremento), Y_inicial + (indice_y * Y_incremento) + Y_incremento);
		
		//Adiciona a soma parcial
		soma_parcial += (soma_local * X_incremento)/2.0;
	}
	
	//Reduz as somas parciais de todos os processos para a soma total no processo raiz com a operacao de soma
	MPI_Reduce(&soma_parcial, &soma_total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	
	//Processo raiz imprime valor da integral
	if(rank == 0){
		printf("Total: %f\n", soma_total);
	}
	
	//Delimita o escopo
	MPI_Finalize();
	return 0;
}
