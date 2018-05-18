/*
 ============================================================================
 Name        : ass2_romberg.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double F(double x);
double G(double x);
double P(double x);
void romberg(double f(double), double a, double b, int n, double **R);

double F(double x) {
	return (1.0 / (1.0 + x));
}

double G(double x) {
	return (log(7 * x) / x);
}

double P(double x) {
	double xx = (3 * x) + 2;
	double result = sqrt(xx);
	return result;
}

void romberg(double f(double), double a, double b, int n, double **R) //Hier MPI_Bcast f�r die Grenzen l(a)-r(b)
{
	int i, j, k;
	double h, sum;

	h = b - a;
	R[1][1] = 0.5 * h * (f(a) + f(b));
	printf(" R[1][1] = %f\n", R[1][1]);

	for (i = 2; i <= n; i++) {

		h = (b-a)*pow(2,1-i);
//		h *= 0.5;
		sum = 0;
		for (k = 1; k <= pow(2, i-2); k += 1) {
			sum += f(a + ((2*k)-1) * h);
		}
		R[i][1] = 0.5 * R[i - 1][1] + sum * h; //Inidizes vertauscht?!
		for (j = 1; j <= i; j++) {
			R[i][j+1] = R[i][j] + (R[i][j] - R[i - 1][j])
					/ (pow(4, j) - 1);
			printf("Ende -> R[%d][%d] = %f\n", i, j, R[i][j]);
		}
		printf("{ Give That to other process %d / %d} R[%d][0] = %1f\n", i, n,
				i, R[i][1]);
	}
	printf("My result for integral: R[%d][%d] -- %f\n\n", n, n, R[n][n]);

	/*int ns = 10;
	double myA = 0;
	double myB = 1;
	double **T;
	for (int var = 0; var < ns; ++var) {
		for (int b = 0; b < ns; ++b) {
			R[var][b] = 0;
		}
	}

	T = calloc((ns + 1), sizeof(double *));
	for (i = 0; i <= ns; i++)
		T[i] = calloc((ns + 1), sizeof(double));
	for (int myI = 0; myI < ns; ++myI) {
		for (int myJ = 0; myJ < ns; ++myJ) {
			printf("T[%d][%d] -- %f\n", myI, myJ, T[myI, myJ]);
		}
	}
	*/

}

int main(void) {
	int n = 10;
	int i;
	double **R;
	double F(double), G(double), P(double);

	R = calloc((n + 1), sizeof(double *));
	for (i = 0; i <= n; i++)
		R[i] = calloc((n + 1), sizeof(double));
	printf("The first function is F(x) = 1/(1 + x)\n");
	romberg(F, 0.0, 2.0, 3, R);
	printf("The second function is G(x) = log(7x)/x\n");
	romberg(G, 0.1, 1.0, 9, R);
	printf("The third function is P(x) = sqrt(3x+2)\n");
	romberg(P, 0.0, 1.0, 7, R);
	return 0;
}
