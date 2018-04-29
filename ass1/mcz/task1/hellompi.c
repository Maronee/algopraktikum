/************************************************************************/
/* Program: hellomp.c                                                   */
/* Author: Michael Czaja, xx                                            */
/* matriclenumber: 4293033                                              */
/* Assignment : 1                                                       */
/* Task: 1                                                              */
/* Parameters: no                                                       */
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

#include "mpi.h"	// import of the MPI definitions
#include <stdio.h>  // import of the definitions of the C IO library
#include <string.h> // import of the definitions of the string operations
#include <unistd.h> // standard unix io library definitions and declarations
#include <errno.h>  // system error numbers

#include <stdlib.h>

int main(int argc, char *argv[])
{

	int namelen;							 // length of name
	int my_rank;							 // rank of the process
	MPI_Init(&argc, &argv);					 // initializing of MPI-Interface
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //get your rank

	int terminate = 0;

	int i;
	for (i = 0; i < argc; i++)
	{
		// printf("Arguments %d : %s\n", i, argv[i]);
		if (strcmp(argv[i], "-h") == 0)
		{
			if (my_rank == 0)
			{
				printf("\n");
				printf("-----------------------------------------------------[Help]--\n");
				printf("\n");
				printf("No sepcific parameters needed. Just start the app like usual.\n");
				printf("\n");
				printf("\n");
				terminate = 1;
				break;
			}
			else
			{
				terminate = 1;
			}
		}
	}

	if (terminate == 1)
	{
		if (terminate == 1)
		{
			exit(0);
		}
	}
	// normal execution space
	else
	{

		char *c, proc_name[MPI_MAX_PROCESSOR_NAME + 1]; // hostname

		memset(proc_name, 0, MPI_MAX_PROCESSOR_NAME + 1);
		// initialize string with NULL-characters
		MPI_Get_processor_name(proc_name, &namelen);
		// finding out own computer name

		int world_rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
		int world_size;
		MPI_Comm_size(MPI_COMM_WORLD, &world_size);

		if ((c = strchr(proc_name, '.')) != NULL)
			*c = '\0';

		int tokenChar;
		char *dataholder[world_size + 1];
		if (my_rank == 0)
		{
			//MPI_Recv(void *data, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm communicator, MPI_Status *status)
			for (int listenToRank = 1; listenToRank < world_size; listenToRank++)
			{
				char data[100];
				printf("Root waiting for %d!\n", listenToRank);
				MPI_Recv(&data, 100, MPI_BYTE, listenToRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				//dataholder[0] ="hallo";
				printf("Root got message from  %d:%s\n", listenToRank, data);
			}
			printf("finish\n");
		}
		else
		{

			char data[100];
			sprintf(data, "%03d: process running on %s!", my_rank, proc_name);
			MPI_Send(&data, 100, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
		}

		MPI_Finalize(); // finalizing MPI interface
		return 0;		// end of progam with exit code 0
	}
}
