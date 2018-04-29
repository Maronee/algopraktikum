/***********************************************************************
 Program: hellomp.c                                                  
 Author: Michael Czaja, xx                                           
 matriclenumber: 4293033                                             
 Assignment : 1                                                      
 Task: 2                                                             
 Parameters: no                                                      
 Environment variables: no                                           
                                                                     
 Description:                                                        
                                                                     
 Implements the odd-even tranposition sort - algorithm               
 First every computer will generate a random number. Then            
 the the number will be sorted step by step and relative to their    
 ranks                                                               
/************************************************************************/

#include "mpi.h"	// import of the MPI definitions
#include <stdio.h>  // import of the definitions of the C IO library
#include <string.h> // import of the definitions of the string operations
#include <unistd.h> // standard unix io library definitions and declarations
#include <errno.h>  // system error numbers

#include <stdlib.h>

int checkParameters(int my_rank, int argc, char *argv[]);
int terminateIfNeeded(int terminate);
int rootGetAndPrintNodeRandInRingMode(int ringCallID, int comChannel, int my_rank, int myRand);
int world_size;

int main(int argc, char *argv[])
{

	// ----------------------------------------------[Init]--
	int namelen;							 // length of name
	int my_rank;							 // rank of the process
	MPI_Init(&argc, &argv);					 // initializing of MPI-Interface
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //get your rank

	// init call
	srand(time(NULL)); // should only be called once
	int r = rand();	// returns a pseudo-random integer between 0 and RAND_MAX

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// ----------------------------------------------[Para check]--

	int needToTerminate = checkParameters(my_rank, argc, argv);
	terminateIfNeeded(needToTerminate);

	// ----------------------------------------------------[Main]--

	char *c, proc_name[MPI_MAX_PROCESSOR_NAME + 1]; // hostname

	memset(proc_name, 0, MPI_MAX_PROCESSOR_NAME + 1);
	// initialize string with NULL-characters
	MPI_Get_processor_name(proc_name, &namelen);
	// finding out own computer name

	if ((c = strchr(proc_name, '.')) != NULL)
		*c = '\0';

	int myRand = world_size - my_rank;

	int numberFormOtherP = -1;
	int numberToSendBack = -1;

	/*if (my_rank == 0)
	{
		rootGetAndPrintNodeRandInRingMode(0, 98, my_rank, myRand);
	}*/
	for (int round = 0; round < world_size; round++)
	{
		numberFormOtherP = 0;
		numberToSendBack = 0;

		//even-phase
		if (round % 2 == 0)
		{
			if (my_rank % 2 == 0)
			{
				if (my_rank + 1 >= world_size)
				{
					//Do Nothing no right neighbour
				}
				else
				{
					MPI_Recv(&numberFormOtherP, 1, MPI_INT, my_rank + 1, round, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					if (myRand >= numberFormOtherP)
					{
						numberToSendBack = myRand;
						myRand = numberFormOtherP;
					}
					MPI_Send(&numberToSendBack, 1, MPI_INT, my_rank + 1, round, MPI_COMM_WORLD);
					printf("Round <%d> | From <%d> to <%d> | data <%d>\n", round, my_rank, my_rank + 1, numberFormOtherP);
				}
			}
			if (my_rank % 2 == 1)
			{
				MPI_Send(&myRand, 1, MPI_INT, my_rank - 1, round, MPI_COMM_WORLD);
				MPI_Recv(&numberFormOtherP, 1, MPI_INT, my_rank - 1, round, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				myRand = numberFormOtherP;
				printf("Round <%d> | <%d> to <%d> | data <%d>\n", round, my_rank, my_rank - 1, round);
			}
		}
		//odd-phase
		else
		{
			if (my_rank % 2 == 1)
			{
				if (my_rank + 1 >= world_size)
				{
					//Do Nothing no right neighbour
				}
				else
				{
					MPI_Recv(&numberFormOtherP, 1, MPI_INT, my_rank + 1, round, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					if (myRand >= numberFormOtherP)
					{
						numberToSendBack = myRand;
						myRand = numberFormOtherP;
					}
					MPI_Send(&numberToSendBack, 1, MPI_INT, my_rank + 1, round, MPI_COMM_WORLD);
					printf("Round <%d> | From <%d> to <%d> | data <%d>\n", round, my_rank, my_rank + 1, numberFormOtherP);
				}
			}
			if (my_rank % 2 == 0)
			{
				if (my_rank == 0)
				{
					//Do Nothing
				}
				else
				{
					MPI_Send(&myRand, 1, MPI_INT, my_rank - 1, round, MPI_COMM_WORLD);
					MPI_Recv(&numberFormOtherP, 1, MPI_INT, my_rank - 1, round, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					myRand = numberFormOtherP;
					printf("Round <%d> | <%d> to <%d> | data <%d>\n", round, my_rank, my_rank - 1, round);
				}
			}
		}
		printf("Round: %d, Me: %d, Content:%d \n", round, my_rank, myRand);
		/*
		int ringCallID = 1;
		int firstPrint = 0;
		if (my_rank == 0)
		{

			if (firstPrint == 0)
			{
				printf("----- Root - Starting ring-call (%d)-----\n", ringCallID);
				printf("Root got message from 0:%d\n", myRand);
				firstPrint++;
			}
			for (int listenToRank = 1; listenToRank < world_size; listenToRank++)
			{
				int data;
				printf("Root waiting for %d!\n", listenToRank);
				MPI_Recv(&data, 1, MPI_INT, listenToRank, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				//dataholder[0] ="hallo";
				printf("Root got message from  %d:%d\n", listenToRank, data);
			}
			printf("finish\n");
		}
		else
		{
			MPI_Send(&myRand, 1, MPI_INT, 0, 99, MPI_COMM_WORLD);
		}
		*/
	}
	// -------------------------------------------------[Para Part]--

	printf("Node|Number - %d|%d\n",my_rank,myRand);
	MPI_Finalize(); // finalizing MPI interface
	return 0;		// end of progam with exit code 0
}

int rootGetAndPrintNodeRandInRingMode(int ringCallID, int comChannel, int my_rank, int myRand)
{
	int firstPrint = 0;
	//int ringCallID = 0;
	if (my_rank == 0)
	{

		if (firstPrint == 0)
		{
			printf("----- Root - Starting ring-call (%d)-----\n", ringCallID);
			printf("Root got message from 0:%d\n", myRand);
			firstPrint++;
		}
		//MPI_Recv(void *data, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm communicator, MPI_Status *status)
		for (int listenToRank = 1; listenToRank < world_size; listenToRank++)
		{
			int data;
			printf("Root waiting for %d!\n", listenToRank);
			MPI_Recv(&data, 1, MPI_INT, listenToRank, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			//dataholder[0] ="hallo";
			printf("Root got message from  %d:%d\n", listenToRank, data);
		}
		printf("finish\n");
	}
	else
	{
		MPI_Send(&myRand, 1, MPI_INT, 0, 99, MPI_COMM_WORLD);
	}
	return 0;
}
/**
	 * @brief  
	 * Checks if all parameters are given and sets them globally for the flowing execution. 
	 * If -h tag detacted then a help-message will be printend and the execution will be stoped.
	 * 
	 * Returns 1 if -h is found.
	 * 
	 * @param my_rank Rank of the processor-
	 * @param argc Number of args.
	 * @param argv Args.
	 * @return int 1 - if -h tag or mismatch of parameters. 0 - otherwise.
	 */
int checkParameters(int my_rank, int argc, char *argv[])
{
	int terminate = 0;
	for (int i = 0; i < argc; i++)
	{
		// printf("Arguments %d : %s\n", i, argv[i]);
		if (strcmp(argv[i], "-h") == 0)
		{
			if (my_rank == 0)
			{
				printf("\n");
				printf("-----------------------------------------------------[Help]--\n");
				printf("\n");
				printf("No specific parameters needed. Just start the app like usual.\n");
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
	return terminate;
}

/**
 * @brief 
 * Canceled the program-execution if needed.
 * 
 * @param terminate 1- to stop execution.
 */
int terminateIfNeeded(int terminate)
{
	if (terminate == 1)
	{
		printf("Execution will be canceled");
		exit(0);
	}
}
