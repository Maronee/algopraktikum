/************************************************************************/
/* Program: a3_1.c                                                      */
/* Author: Hans-Joachim Hammer, Sebastian Voinea                        */ 
/*  s4727687@stud.uni-frankfurt.de, sebastian.voinea@gmx.de             */
/* matriclenumber:5245679, 4498171                                      */
/* Assignment : 3                                                       */	
/* Task: 1                                                              */
/* Parameters: -help -debug -e -m -v 					                */
/* Environment variables: no                                            */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/* jacobi2 implements the parallelized jacobi method                    */
/* it reads from a vector and matrix file and calculates a result vector*/
/************************************************************************/

#include "mpi.h" 	    // import of the MPI definitions
#include <stdio.h> 	    // import of the definitions of the C IO library
#include <string.h>     // import of the definitions of the string operations
#include <unistd.h>	    // standard unix io library definitions and declarations
#include <errno.h>	    // system error numbers
#include <stdlib.h>
#include <math.h>
int main(int argc, char* argv[ ]) 
{ 
    
	int my_rank; 					// rank of the process
	int world_size;

	MPI_Init(&argc, &argv);		 	// initializing of MP:I-Interface
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 	//get your rank
	int m=1;
	int debug=0;
	double epsilon=0;
	int flag=0;
	char *vector_name;
	char *matrix_name;
	for (int i=1; i<argc; i++){
		if (strcmp(argv[i],"-help")==0 && my_rank==0){	
		printf("Fuehren Sie das Programm mit mpiexec -f <hostfile> -p<Anzahl Prozesse> ./<Programmname> -e <Epsilon> -m <Matrixname> -v <Vektorname> aus, um es auf dem Cluster zu verwenden  \n");
		printf("Andere Optionen: -debug");
		exit(0);
		}
		if (strcmp(argv[i], "-m")==0){
			m= atof(argv[i+1]);
			if (m<0 && my_rank==0){
			printf("m muss positiv sein.\n");
			exit(0);
			}
			flag++;
			}

		
		if (strcmp(argv[i], "-debug")==0){
			debug=1;
			}
		}

	MPI_File filehandle;
	int err;
	err = MPI_File_open(MPI_COMM_WORLD, "ffm_1280x960.gray", MPI_MODE_RDONLY, MPI_INFO_NULL, &filehandle);
	if (debug){
	printf("File open status: %i \n", err);
	}
	if (err!=0 && my_rank==0){
		printf("Couldnt open Matrix\n");
		exit(0);
		}
	MPI_Offset size;
	err = MPI_File_get_size(filehandle, &size);
	int count_total = size / sizeof(unsigned char);
	int columns = 1280;
	int rows  = count_total / columns;
	int count_local = count_total / world_size;

	MPI_Datatype column_t;
	MPI_Type_vector(rows, columns / world_size, columns , MPI_UNSIGNED_CHAR, &column_t);
	MPI_Type_commit(&column_t);

	unsigned char *buffer = (unsigned char*) malloc(count_local*sizeof(unsigned char));
	MPI_File_read_ordered(filehandle, buffer, count_local, column_t, MPI_STATUS_IGNORE);	
	printf("rbef \n");
	MPI_File_close(&filehandle);

	printf("%i %i \n", rows, columns);
	double *proc_column = (double*) malloc(count_local * sizeof(double));

	for(int i = 0; i < rows; i++){
		for(int j = 0; j < columns; j++){
			proc_column[j + i * rows] = (double)buffer[j + i * rows];
		}
	}
	for(int i = 0; i < rows; i++){
		for(int j = 0; j < columns; j++){
			printf("%lf ", proc_column[j + i * rows]);
		}
		printf("\n");
	}

	printf("rbef \n");
	if (debug){
	    printf("Size status: %i , size: %lli , elements: %i, rows: %i\n", err, size, count_total, rows);
	}

 
	 MPI_Finalize();		            // finalizing MPI interface 
	return 0;						// end of progam with exit code 0 
}
