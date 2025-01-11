#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double trapezio(double x1, double y1, double y2){
    //double radianos = (x1*x1 + y1*y1) * M_PI/180.0;
    double g1 = sin(x1*x1 + y1*y1);
    //radianos = (x1*x1 + y2*y2) * M_PI/180.0;
    double g2 = sin(x1*x1 + y2*y2);

    return ((g1 + g2) * (y2 - y1)/2.0);
}

int main(int argc, char **argv){
    omp_set_num_threads(8);

    double soma_total = 0;
    double soma_parcial = 0;
    double x1, x2;

    const double X_final = 1.5;
    const double X_inicial = 0;
    const long X_intervalos = 1000;
    const double X_incremento = (X_final - X_inicial) / X_intervalos;

    const double Y_final = 1.5;
    const double Y_inicial = 0;
    const long Y_intervalos = 1000;
    const double Y_incremento = (Y_final - Y_inicial) / Y_intervalos;

#pragma omp parallel private(soma_parcial, x1, x2) shared(X_final, X_inicial, X_incremento, Y_final, Y_inicial, Y_incremento)
    {
#pragma omp for
        for(long i=0; i<X_intervalos; i++){
            soma_parcial = 0;
            x1 = X_inicial + (i * X_incremento);
            x2 = X_inicial + (i * X_incremento) + X_incremento;
            for(long j=0; j<Y_intervalos; j++){
                soma_parcial += trapezio(x1, Y_inicial + (j * Y_incremento), Y_inicial + (j * Y_incremento) + Y_incremento);
                soma_parcial += trapezio(x2, Y_inicial + (j * Y_incremento), Y_inicial + (j * Y_incremento) + Y_incremento);
            }
            #pragma omp critical
            {
                soma_total += (soma_parcial * (x2 - x1))/2.0;
            }
        }
    }

    printf("Total: %f\n", soma_total);
    return soma_total;
}
