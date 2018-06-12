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

double calcCondition(double *x, double *x_1, int size){
	double sum = 0;
	for(int i = 0; i < size; i++){
		sum += fabs(x_1[i] - x[i]);
	}
	return sum;
}

double calc(double *x, double *a, double b, int i, int actualRow){
	double sum = 0;
	int rowcount = sizeof(a);
	
/*if(i >3){
	printf("row %i, b %lf, rowcount %i \n", i, b, rowcount);
}*/
//
	for(int j = 0; j < rowcount; j++){
		if(j != actualRow){			
			sum += a[i * rowcount + j] * x[j];
			//printf("[%i] (%i.%i) \n",actualRow, i, j);
			//printf("[%i,%i] %lf * %lf = %lf | sum = %lf \n", i,j, a[i*rowcount + j], x[j], a[i*rowcount + j] * x[j], sum);
		}
	}
	//printf("\n");
//	printf("[%i] 1/a * (b - sum) = 1 / %lf * ( %lf - %lf ) = %lf \n", i, a[i*rowcount+i], b, sum, (1/a[i*rowcount+i]) * (b - sum));
	return (1/a[i * rowcount + actualRow]) * (b - sum);
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
	double *vecx_2 = malloc(bcount * sizeof(double));
	double *bcastBuffer = malloc(bcount * sizeof(double));
	
	for(int i = 0; i < bcount; i++){
		vecx_2[i] = 1;
	}

/*	if(my_rank!= 0){
	for(int i= 0; i < acount; i++){
		printf("[%i] a%i%i = %lf \n", my_rank, i / rows, i % rows ,buffer[i]);
	}
    }
	for(int i= 0; i < bcount; i++){
		printf("[%i] b%i %lf \n", my_rank, i, bbuffer[i]);
	}
*/

/*
	for(int i= 0; i < bcount; i++){
		printf("[%i] x%i %lf \n", my_rank, i, vecx[i]);
	}
*/
	int iteration = 0;
	int interval = rows / world_size;
	double diff = 0;
	do {
		//iteration++;
		memcpy(vecx, vecx_2, bcount * sizeof(double));

		/*for(int i = 0; i < bcount; i++){
			printf("Iteration: %i v%i = %lf \n", iteration, i, vecx[i]);
		}*/
		
		
		for(int i = 0; i < interval; i++){
			int rowpos = my_rank * interval + i;
			vecx_2[rowpos] = calc(vecx, buffer, bbuffer[rowpos], i, rowpos);
			printf("[%i]result row %i:  %lf \n", my_rank, rowpos, vecx_2[rowpos]);
		}
		for(int j = 0; j < world_size; j++){
			if(j == my_rank){
				memcpy(bcastBuffer, vecx_2, bcount * sizeof(double));
			}
			MPI_Bcast(bcastBuffer, bcount, MPI_DOUBLE, j, MPI_COMM_WORLD);
			printf( "after broadcast\n");
			for(int i = j * interval; i < j * interval + interval; i++){
				//printf("[%i] %lf \n", my_rank, bcastBuffer[i]);
				vecx_2[i] = bcastBuffer[i];
			}	
		}
		//	if(iteration > 4) { exit(0);}
	} while(calcCondition(vecx, vecx_2, bcount) > 0.4);

	free(bcastBuffer);
	free(vecx);
	free(buffer);
	free(bbuffer);

	for(int i = my_rank * interval; i < my_rank*interval + interval; i++){
		printf("[%i] x%i %lf \n", my_rank, i, vecx_2[i]);
	}

	free(vecx_2);
	MPI_File_close(&filehandle);
    MPI_Finalize();		            // finalizing MPI interface 
	return 0;						// end of progam with exit code 0 
}


