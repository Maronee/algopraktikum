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
	int world_size;	


	MPI_Init(&argc, &argv);		 	// initializing of MP:I-Interface
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 	//get your rank

	double epsilon=0;
	int flag=0;
	char *vector_name;
	char *matrix_name;
	for (int i=1; i<argc; i++){
		if (strcmp(argv[i],"-help")==0 && my_rank==0){	
		printf("Fuehren Sie das Programm mit mpiexec -f <hostfile> -p<Anzahl Prozesse> ./<Programmname> -e <Epsilon> -m <Matrixname> -v <Vektorname> aus, um es auf dem Cluster zu verwenden  \n");
		exit(0);
		}
		if (strcmp(argv[i], "-e")==0){
			epsilon= atof(argv[i+1]);
			if (epsilon<0 && my_rank==0){
			printf("Epsilon muss positiv sein.\n");
			exit(0);
			}
			flag++;
			}
		if (strcmp(argv[i], "-m")==0){
			matrix_name = argv[i+1];
			flag++;
			}
		if (strcmp(argv[i],"-v")==0){
			vector_name=argv[i+1];
			flag++;
			}
		
		}
	if ((my_rank==0) & (flag !=3)){
	 		printf("Fuehren Sie das Programm mit mpiexec -f <hostfile> -p<Anzahl Prozesse> ./<Programmname> -e <Epsilon> -m <Matrixname> -v <Vektorname> aus, um es auf dem Cluster zu verwenden  \n");
	
	}

	MPI_File filehandle;
	int err;
	MPI_Offset size;
	
	//open matrix a
	err = MPI_File_open(MPI_COMM_WORLD, matrix_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &filehandle);
	printf("File open status: %i \n", err);		
	err = MPI_File_get_size(filehandle, &size);
	int count_total = size / sizeof(double);
	int rows = sqrt(count_total);
	// +2 to leave the last spot for the value of vector b and x -> 
	// Array is supposed to look like this(Matrix A   | b_i | x_j)
	int interval = rows / world_size;
	int scatterCount = (count_total / world_size) + interval;
	double *scatterMatrix = (double*)malloc(scatterCount * sizeof(double));
	printf("Size status: %i , size: %lli , elements: %i, rows: %i\n", err, size, count_total, rows);
	printf("Scatter count: %i \n", scatterCount);
	

	double *aMatrix = (double*) malloc(count_total * sizeof(double));
	if(my_rank ==  0){
		MPI_File_read(filehandle, aMatrix, count_total, MPI_DOUBLE, MPI_STATUS_IGNORE);
	}
	MPI_File_close(&filehandle);

	//open vector b
	err = MPI_File_open(MPI_COMM_WORLD, vector_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &filehandle);
	printf("File open status: %i \n", err);

	err = MPI_File_get_size(filehandle, &size);
	int bcount = (size / sizeof(double));
	double *bVec = malloc(bcount * sizeof(double));

	if(my_rank == 0) {	
		MPI_File_read(filehandle, bVec, bcount, MPI_DOUBLE, MPI_STATUS_IGNORE);
	}
	
	MPI_File_close(&filehandle);
	
	double *transform = (double*) malloc((count_total + rows) * sizeof(double));
	if(my_rank == 0) {
		
		int count = 0;
		for(int i = 0; i < rows; i++){
			for(int j = 0; j < rows; j++){
				transform[i * rows + j + i] = aMatrix[j * rows + i];
				count++;
				//printf("%i a(%i,%i) = %lf\n",i * rows + j + i,  i, j, transform[i * rows + j + i]);
			}			
			transform[i * rows + rows + i] = bVec[i];
			count++;
			//printf("%i a(%i,%i) = %lf = b \n",i * rows + rows + i, i ,rows , transform[i * rows + rows + i]);
		}
		//printf("%i\n", scatterCount);
	}
	MPI_Scatter(transform, scatterCount, MPI_DOUBLE, scatterMatrix, scatterCount, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	
	free(aMatrix);
	free(bVec);
	free(transform);

	double sum, diff, diffTotal, local_sum;
	double *vecx = (double*) malloc(interval * sizeof(double));
	double *vecx_2 = (double*) malloc(interval * sizeof(double));
	//printf("I get here %i \n", my_rank);
	//double *local_sum = (double*)malloc(interval * sizeof(double));
	//init value	
	for(int i = 0; i < interval; i++){
		vecx_2[i] = 1.0;
	//	local_sum[i] = 0;
	}

	local_sum = 0;
	/*for(int i = 0; i < scatterCount; i++){
		printf("[%i] a(%i) = %lf\n", my_rank, i + my_rank * scatterCount, scatterMatrix[i]);
	}*/

	//MPI_Barrier(MPI_COMM_WORLD);
	
	do{
		memcpy(vecx, vecx_2, interval * sizeof(double));
		
		diff = 0;		
		
		for(int i = 0; i < rows; i++){	
			int procIndex = i - my_rank * interval;
			int sumIndex = i % interval;
			//summe ueber matrix  bilden
			for(int j = 0; j < interval; j++){
				if(i != (j + interval*my_rank)){
					local_sum += scatterMatrix[j * (rows + 1) + sumIndex] * vecx[j];
					//printf("[%i] %i,%i, array index %i, %lf, sum = %lf \n", my_rank, j, i, j * (rows + 1) + sumIndex, scatterMatrix[j * (rows + 1) + sumIndex], local_sum);
					printf("%i,%i val %lf \n", i, j , scatterMatrix[j * (rows + 1) + sumIndex]);
				}
			}

			//berechnung von jacobi fuer x[i]
			if(i / interval == my_rank){
				//printf("[%i] row %i head honcho\n", my_rank, i);
				MPI_Reduce(&local_sum, &sum, 1, MPI_DOUBLE, MPI_SUM, my_rank, MPI_COMM_WORLD);				
				vecx_2[procIndex] = (1/scatterMatrix[procIndex * rows + i + procIndex]) * (scatterMatrix[procIndex * rows + rows + procIndex] - sum);
				//printf("[%i] row %i -> vecx_2[%i] , rowcount %i, diagonal index %i, b index %i, do I understand indices?\n", my_rank,i, i - my_rank * interval, rows, procIndex * rows + i + procIndex, procIndex * rows + rows + procIndex);
				//printf("[%i] row %i -> vecx_2[%i] , rowcount %i, diagonal index %i, b index %i, do I understand indices?\n", my_rank,i, i - my_rank * interval, rows, i * rows + i + i, i * rows + rows + i);
				//printf("[%i] vecx %lf vecx 2 %lf   diff = %lf\n", i, vecx[procIndex], vecx_2[procIndex], fabs(vecx_2[procIndex] - vecx[procIndex]));
				//printf("[%i] 1 / %lf * ( %lf - %lf ) = %lf \n", i, scatterMatrix[procIndex * rows + i + procIndex], scatterMatrix[procIndex * rows + rows + procIndex], sum, vecx_2[procIndex]);
				diff += fabs(vecx_2[procIndex] - vecx[procIndex]);		
			}else{
				MPI_Reduce(&local_sum, &sum, 1, MPI_DOUBLE, MPI_SUM, i / interval, MPI_COMM_WORLD);
			}
			local_sum = 0;
			sum = 0;		
		}
		//differenz berechnung ueber map reduce
		//printf("[%i] diff %lf\n", my_rank, diff);
		MPI_Allreduce(&diff, &diffTotal, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
		exit(0);
		//printf("[%i] difftotal %lf\n", my_rank, diffTotal);
		
	}while(diffTotal > epsilon);
	/*	
	for(int i = 0; i < interval; i++){
		printf("[%i] Result: x[%i] = %lf\n", my_rank, i + my_rank*interval, vecx_2[i]);
	}
	*/

	double *result = (double*) malloc(rows * sizeof(double));
	MPI_Gather(vecx_2, interval, MPI_DOUBLE, result, interval, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	if(my_rank == 0){
		printf("Gather receive:\n");
		for(int i = 0; i < rows; i++){
			printf("x[%i] = %lf\n", i, result[i]);	
		}
	}

	MPI_File fh;
	MPI_File_open(MPI_COMM_WORLD, "vecx_result",MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

	err= MPI_File_write(fh, vecx_2, rows, MPI_DOUBLE, MPI_STATUS_IGNORE);
	MPI_File_close(&fh);
	
	free(vecx);
	free(vecx_2);
	//free(transform); // so late because earlier it fails x_x
    MPI_Finalize();		            // finalizing MPI interface 
	return 0;						// end of progam with exit code 0 
}


