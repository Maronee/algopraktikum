/***********************************************************************
 Program: hellomp.c                                                  
 Author: Michael Czaja, Muttaki Aslanparcasi                                           
 matriclenumber: 4293033, 5318807                                             
 Assignment : 1                                                      
 Task: 3                                                             
 Parameters: no                                                      
 Environment variables: no                                           
                                                                     
 Description:                                                        
                                                                     
Calculates the minimum, maximum, sum and arithmetical average of
a random set of integers. Each process gets an array and recalculates the results
and forwards it.
 Program uses buffered communication.
/************************************************************************/

#include "mpi.h"	// import of the MPI definitions
#include <stdio.h>  // import of the definitions of the C IO library
#include <string.h> // import of the definitions of the string operations
#include <unistd.h> // standard unix io library definitions and declarations
#include <errno.h>  // system error numbers

#include <stdlib.h>

int main(int argc, char *argv[])
{
	int namelen; // length of name

	MPI_Init(&argc, &argv);					 // initializing of MPI-Interface
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //get your rank
	// init call
	srand(time(NULL)); // should only be called once
	int r = rand();	// returns a pseudo-random integer between 0 and RAND_MAX
					   // rank of the process
	double starttime = MPI_Wtime();
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	char *c, proc_name[MPI_MAX_PROCESSOR_NAME + 1]; // hostname

	memset(proc_name, 0, MPI_MAX_PROCESSOR_NAME + 1);
	// initialize string with NULL-characters
	MPI_Get_processor_name(proc_name, &namelen);
	//-------------------------------------------------------------[Data make]--

	double myInitData[4];
	for (int i = 0; i < mValue; i++)
	{
		int myRand = rand() % (100 + 1 - 1) + 1;

		char szBuffer[4];
		sprintf(szBuffer, "%d", myRand);
		myNumbers[i] = myRand;
	}

	//--------------------------------------------------------------[Till END]--
	double endtime;
	endtime = MPI_Wtime();
	double resulttime = endtime - starttime;
	printf("elapsed Time: %lf\n", resulttime);
	MPI_Finalize(); // finalizing MPI interface

	return 0;
}

double calculateSum(double array[], int size) //calculates the Summary of the Integers
{
	double sum = 0;
	for (int i = 0; i < size; i++)
	{
		sum += array[i];
	}
	return sum;
}

double calcMinimumOfArray(double array[], int size) //calculates the minimum
{
	double personalMini = -1;

	for (int i = 0; i < size; i++)
	{
		if (i == 0)
			personalMini = array[0];
		else
		{
			if (personalMini <= array[i])
			{ // nothing
			}
			else
			{
				personalMini = array[i];
			}
		}
	}
	return personalMini;
}
double calcMaxOfArray(double array[], int size) //calculates the maximum
{
	double personalMax = -1;

	for (int i = 0; i < size; i++)
	{
		printf("CHECKING <%d> with <%d>\n", personalMax, array[i]);
		if (personalMax >= array[i])
		{ // nothing
		}
		else
		{
			personalMax = array[i];
		}
	}
	return personalMax;
}
