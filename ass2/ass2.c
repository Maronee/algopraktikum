/*
 ============================================================================
 Name        : ass2.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
//#include "mpi.h"	// import of the MPI definitions
#include <string.h> // import of the definitions of the string operations
#include <unistd.h> // standard unix io library definitions and declarations
#include <errno.h>  // system error numbers
#include<math.h>

/* Define the function to be integrated here: */
double f(double x){
  return x*x;
}

/*Function definition to perform integration by Trapezoidal Rule */
double trapezoidal(double f(double x), double a, double b, int n){
  double x,h,sum=0,integral;
  int i;
  h=fabs(b-a)/n;
  for(i=1;i<n;i++){
    x=a+i*h;
    sum=sum+f(x);
  }
  integral=(h/2)*(f(a)+f(b)+2*sum);
  return integral;
}


/*Program begins*/
int main(int argc, char *argv[])
{

		int i=2;
	  double a,b,eps,integral,integral_new;

	  /*Ask the user for necessary input */
	  printf("\nEnter the initial limit: ");
	  scanf("%lf",&a);
	  printf("\nEnter the final limit: ");
	  scanf("%lf",&b);
	  printf("\nEnter the desired accuracy: ");
	  scanf("%lf",&eps);
	  integral_new=trapezoidal(f,a,b,i);

	  /* Perform integration by trapezoidal rule for different number of sub-intervals until they converge to the given accuracy:*/
	  do{
	    integral=integral_new;
	    i++;
	    integral_new=trapezoidal(f,a,b,i);
	  }while(fabs(integral_new-integral)>=eps);

	  /*Print the answer */
	  printf("The integral is: %lf\n with %d intervals",integral_new,i);

	}
