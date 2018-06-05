/************************************************************************/
/* Program: hellomp.c                                                   */ 
/* Author: Hans-Joachim Hammer, Sebastian Voinea                        */ 
/*  s4727687@stud.uni-frankfurt.de, sebastian.voinea@gmx.de             */
/* matriclenumber:5245679, 4498171                                      */
/* Assignment : 2                                                       */	
/* Task: 1                                                              */
/* Parameters: -h                                                       */
/* Environment variables: no                                            */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/* hellompi is a simple example program for MPI                         */
/*                                                                      */
/* For each CPU, where the program is running,                          */
/* the hostname will be printed.                                        */
/*                                                                      */
/************************************************************************/ 

#include "mpi.h" 	    // import of the MPI definitions
#include <stdio.h> 	    // import of the definitions of the C IO library
#include <string.h>     // import of the definitions of the string operations
#include <unistd.h>	    // standard unix io library definitions and declarations
#include <errno.h>	    // system error numbers
#include <stdlib.h>
#include <math.h>

double calc(double* x, double* a, double b, int i){
	int sum1, sum2;
	int rowcount = sqrt(sizeof(a) / sizeof(double));
	sum1 = 0;
	sum2 = 0;
	for(int j = 0; j < i-1; i++){
		sum1 += a[i*rowcount + j] * x[j];
	}
	for(int j = i + 1; j < rowcount; j++){
		sum2 += a[i*rowcount + j] * x[j];
	}
	return (1/a[i*rowcount+i]) * (b - sum1 - sum2);
}

int main(int argc, char* argv[ ]) 
{ 
    
	int my_rank; 					// rank of the process
	int flag = 0;
	int world_size;
	flag=getopt(argc,argv,"h");
	if (flag !=-1) {
		printf("Fuehren Sie das Programm mit mpiexec -f <hostfile> -p<Anzahl Prozesse> ./<Programmname> aus, um es auf dem Cluster zu verwenden  \n");
		exit(0);
	}


	MPI_Init(&argc, &argv);		 	// initializing of MP:I-Interface
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 	//get your rank

	MPI_File filehandle;
	int err;
	//open matrix a
	err = MPI_File_open(MPI_COMM_WORLD, "Matrix_A_8x8", MPI_MODE_RDONLY, MPI_INFO_NULL, &filehandle);
	printf("File open status: %i \n", err);
	MPI_Offset size;
	err = MPI_File_get_size(filehandle, &size);
	int count_total = size / sizeof(double);
	int rows = sqrt(count_total);
	int acount = count_total / world_size;

	printf("Size status: %i , size: %lli , elements: %i, rows: %i\n", err, size, acount, rows);
	
	double *buffer = (double*) malloc(acount*sizeof(double));
	MPI_File_read_ordered(filehandle, buffer, acount, MPI_DOUBLE, MPI_STATUS_IGNORE);
	MPI_File_close(&filehandle);

	//open vector b
	err = MPI_File_open(MPI_COMM_WORLD, "Vector_b_8x", MPI_MODE_RDONLY, MPI_INFO_NULL, &filehandle);
	printf("File open status: %i \n", err);

	err = MPI_File_get_size(filehandle, &size);
	int bcount = (size / sizeof(double));
	double *bbuffer = malloc(bcount * sizeof(double));
	MPI_File_read(filehandle, bbuffer, bcount, MPI_DOUBLE, MPI_STATUS_IGNORE);
	MPI_File_close(&filehandle);

	//init vector x
	double *vecx = malloc(bcount * sizeof(double));
	for(int i = 0; i < bcount; i++){
		vecx[i] = 1;
	}

	for(int i= 0; i < acount; i++){
		printf("[%i] a%i%i = %lf \n", my_rank, i / rows, i % rows ,buffer[i]);
	}
    
	for(int i= 0; i < bcount; i++){
		printf("[%i] b%i %lf \n", my_rank, i, bbuffer[i]);
	}

	for(int i= 0; i < bcount; i++){
		printf("[%i] x%i %lf \n", my_rank, i, vecx[i]);
	}

	int iteration = 0;
	int interval = rows / world_size;
	printf("%i\n", interval);
	for(int i = 0; i < interval; i++){
		vecx[interval * my_rank + i] = calc(vecx, buffer, bbuffer[interval * my_rank + i], interval * my_rank + i);
	}

	for(int i= 0; i < bcount; i++){
		printf("[%i] x%i %lf \n", my_rank, i, vecx[i]);
	}
	free(buffer);
	free(bbuffer);
	MPI_File_close(&filehandle);
    MPI_Finalize();		            // finalizing MPI interface 
	return 0;						// end of progam with exit code 0 
}

