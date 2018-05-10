/***********************************************************************
 Program: hellomp.c                                                  
 Author: Michael Czaja, xxxxxxx                                           
 matriclenumber: 4293033                                             
 Assignment : 1                                                      
 Task: 2                                                             
 Parameters: no                                                      
 Environment variables: no                                           

 Description:                                                        

 Implements the odd-even tranposition sort - algorithm.               
 First, every computer will generate a random number. After that,
 the number of sequence will be sorted by the network due a simple parallel 
 strategy.

 Program uses buffered communication.
 /************************************************************************/

#include "mpi.h"	// import of the MPI definitions
#include <stdio.h>  // import of the definitions of the C IO library
#include <string.h> // import of the definitions of the string operations
#include <unistd.h> // standard unix io library definitions and declarations
#include <errno.h>  // system error numbers
#include <time.h>

#include <stdlib.h>

#define MAX_BUFFER_SIZE 1000

int utilCheckParameters(int my_rank, int argc, char *argv[]);
int utilTerminateIfNeeded(int terminate);
int rootGetAndPrintNodeRandInRingMode(char tag[], int commLine, int myRank,
									  int myRand);

double taskLookForMin(double myInitNumbers[], int size);
double taskLookForMax(double myInitNumbers[], int size);
double taskCalcMax(double myInitNumbers[], int size);
double taskCalcAVG(double number, int amount);

int utilTerminateIfNeededSilently(int terminate);
int utilPrintHelp();
void utilOTPrint(int rankWhichPrints, int my_rank, char message[]);
void utilPrintArray(double array[], int size);
void utilOTPrintWRank(int rankWhichPrints, int my_rank, char message[]);

int world_size;
int mValue;
int commLine = 9999;

int main(int argc, char *argv[])
{
	clock_t begin = clock();
	// -----------------------------------------------------------------[Init]--
	int namelen;							 // length of name
	int my_rank;							 // rank of the process
	MPI_Init(&argc, &argv);					 // initializing of MPI-Interface
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //get your rank

	// init call
	srand(time(NULL)); // should only be called once
	int mesCommline = 99999;
	int round = 4;

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	char buffer[(MAX_BUFFER_SIZE * world_size) + MPI_BSEND_OVERHEAD];
	int bsize = sizeof(buffer);

	// ----------------------------------------------------------[ Para check ]--

	int needToTerminate = utilCheckParameters(my_rank, argc, argv);

	// ---------------------------------------------------------[ Data Create ]--

	double myInitNumbers[mValue];

	// This will be sent to the other pc.
	// formate( min, max , sum, avg)
	double myNumbers[mValue];
	double dataToSend[3];
	double dataToRev[3];

	double testArray[] = {10, 3, 2, 1};

	for (int i = 0; i < mValue; i++)
	{
		double myRand = rand() % (100 + 1 - 1) + 1;

		char szBuffer[4];
		sprintf(szBuffer, "%d", myRand);
		myInitNumbers[i] = myRand;
	}

	utilPrintArray(myInitNumbers, mValue);

	double myMinValue = taskLookForMin(myInitNumbers, mValue);
	double myMaxValue = taskLookForMax(myInitNumbers, mValue);
	double mySUMValue = taskCalcMax(myInitNumbers, mValue);

	double myAVGValue = taskCalcAVG(mySUMValue, mValue);
	double resultAVG = -1;

	printf("Min for array <%lf>\n", myMinValue);
	printf("Max for array <%lf>\n", myMaxValue);
	printf("SUM for array <%lf>\n", mySUMValue);
	printf("AVG for array <%lf>\n", myAVGValue);

	dataToSend[0] = myMinValue;
	dataToSend[1] = myMaxValue;
	dataToSend[2] = mySUMValue;
	// ----------------------------------------------------------------[ Main ]--

	char *c, proc_name[MPI_MAX_PROCESSOR_NAME + 1]; // hostname

	memset(proc_name, 0, MPI_MAX_PROCESSOR_NAME + 1);
	// initialize string with NULL-characters
	MPI_Get_processor_name(proc_name, &namelen);
	// finding out own computer name

	if ((c = strchr(proc_name, '.')) != NULL)
		*c = '\0';

	// ------------------------------------------------------------[Init Call]--

	double starttime = MPI_Wtime();
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// ------------------------------------------------------------[Para Part]--
	if (!(world_size < 2))
	{

		if (my_rank == 0)
		{
			double starttime1 = MPI_Wtime();
			MPI_Send(&dataToSend, 3, MPI_DOUBLE, my_rank + 1, round, MPI_COMM_WORLD);
			MPI_Recv(&dataToRev, 3, MPI_DOUBLE, world_size - 1, round, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			double endtime1 = MPI_Wtime();

			double combinedMin[2] = {dataToSend[0], dataToRev[0]};
			double combinedMax[2] = {dataToSend[1], dataToRev[1]};
			double combinedSum = dataToSend[2] + dataToRev[2];
			dataToSend[0] = dataToRev[0];
			dataToSend[1] = dataToRev[1];
			dataToSend[2] = dataToRev[2];
			resultAVG = dataToSend[2] / mValue;

			printf("Node(%d) - Communication time: %lf\n", my_rank, endtime1 - starttime1);
			//TODO Bug letztes element wir doppelt gezählt check woran das liegt.
		}
		else if (my_rank == world_size - 1)
		{

			double starttime2 = MPI_Wtime();
			MPI_Recv(&dataToRev, 3, MPI_DOUBLE, my_rank - 1, round, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			double endtime2 = MPI_Wtime(); // endeTime2
			// time2= endeTime2-startTime2

			double combinedMin[2] = {dataToSend[0], dataToRev[0]};
			double combinedMax[2] = {dataToSend[1], dataToRev[1]};
			double combinedSum = dataToSend[2] + dataToRev[2];
			dataToSend[0] = taskLookForMin(combinedMin, 2);
			dataToSend[1] = taskLookForMax(combinedMax, 2);
			dataToSend[2] = combinedSum;
			resultAVG = dataToSend[2] / mValue;

			double starttime3 = MPI_Wtime();
			MPI_Send(&dataToSend, 3, MPI_DOUBLE, 0, round, MPI_COMM_WORLD);
			double endtime3 = MPI_Wtime(); //endeTime3
			double resulttime3 = (endtime2 - starttime2) + (endtime3 - starttime3);

			printf("Node(%d) - Communication time: %lf\n", my_rank, resulttime3);
		}
		else
		{
			double starttime4 = MPI_Wtime();
			MPI_Recv(&dataToRev, 3, MPI_DOUBLE, my_rank - 1, round, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			double endtime4 = MPI_Wtime();
			double combinedMin[2] = {dataToSend[0], dataToRev[0]};
			double combinedMax[2] = {dataToSend[1], dataToRev[1]};
			double combinedSum = dataToSend[2] + dataToRev[2];
			dataToSend[0] = taskLookForMin(combinedMin, 2);
			dataToSend[1] = taskLookForMax(combinedMax, 2);
			dataToSend[2] = combinedSum;
			double starttime5 = MPI_Wtime();
			MPI_Send(&dataToSend, 3, MPI_DOUBLE, my_rank + 1, round, MPI_COMM_WORLD);
			double endtime5 = MPI_Wtime();
			double resulttime4 = (endtime4 - starttime4) + (endtime5 - starttime5);
			printf("Node(%d) - Communication time: %lf\n", my_rank, resulttime4);
		}
	}
	else
	{
		printf("Yeah less then 2 node\n");
	}
	// final avg calc.
	resultAVG = dataToSend[2] / mValue;

	/*
	*/
	double endtime;
	endtime = MPI_Wtime();
	double resulttime = endtime - starttime;

	// ----------------------------------------------------------[Result Call]--
	MPI_Barrier(MPI_COMM_WORLD);
	usleep(100);
	if (my_rank == 0)
	{
		printf("------------------ RESULT -----------\n");
		printf("Min for arrays                            <%lf>\n", dataToSend[0]);
		printf("Max for arrays                            <%lf>\n", dataToSend[1]);
		printf("SUM for arrays                            <%lf>\n", dataToSend[2]);

		printf("AVG for arrays (dep. m-value)             <%lf>\n", resultAVG);
		printf("AVG for arrays (dep. m-value & worldsize) <%lf>\n", resultAVG / world_size);
	}
	//-----------------------------------------------------------------[ END ]--

	printf("Node (%d) - elapsed Time: %lf\n", my_rank, resulttime);

	MPI_Buffer_detach((void *)buffer, &bsize);
	MPI_Finalize(); // finalizing MPI interface
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Node (%d) - Seq. elapsed Time: %lf\n", my_rank, time_spent);
	return 0; // end of progam with exit code 0
}

// ------------------------------------------------------------------[ TaskFunc. ]--
/**
 * @brief 
 * 
 * @param myInitNumbers 
 * @param size 
 * @return double 
 */
double taskLookForMin(double myInitNumbers[], int size)
{
	double result = myInitNumbers[0];

	for (int i = 0; i < size; i++)
	{
		if (result > myInitNumbers[i])
			result = myInitNumbers[i];
	}

	return result;
}

/**
 * @brief 
 * 
 * @param myInitNumbers 
 * @param size 
 * @return double 
 */
double taskLookForMax(double myInitNumbers[], int size)
{
	double result = myInitNumbers[0];

	for (int i = 0; i < size; i++)
	{
		if (result < myInitNumbers[i])
			result = myInitNumbers[i];
	}

	return result;
}

int taskcheckIfPowerOfTwo(int number)
{
}

int taskCalcHowManyRound(int sizeOfWorld)
{
}

int taskCalcMyPartnerRank(int myRank, int round)
{
}

int taskAmISender(int myRank, int partnerRank)
{
	int send = -1;
	if (myRank > partnerRank)
		send = 1;
	else
		send = 0;
	return send;
}

/**
 * @brief 
 * 
 * @param myInitNumbers 
 * @param size 
 * @return double 
 */
double taskCalcMax(double myInitNumbers[], int size)
{
	double result = 0;

	for (int i = 0; i < size; i++)
	{
		result += myInitNumbers[i];
	}

	return result;
}
/**
 * @brief 
 * 
 * @param number 
 * @param amount 
 * @return double 
 */
double taskCalcAVG(double number, int amount)
{
	return number / amount;
}

// ------------------------------------------------------------------[ UTILS ]--

void utilPrintResultBySendingThemToRoot(int rank, int array[])
{
	MPI_Barrier(MPI_COMM_WORLD);
	usleep(100);
	if (rank == 0)
	{
		printf("Result: ");
	}
	for (int i = 0; i < world_size; i++)
	{
		MPI_Barrier(MPI_COMM_WORLD);
		if (rank == i)
			utilPrintArray(array, mValue);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	usleep(100);
	printf("\n");
}

void utilPrintArray(double array[], int size)
{
	int index;
	for (index = 0; index < size; index++)
	{
		printf("<%lf>\n", array[index]);
	}
}

/**
 * @brief
 * Checks if all parameters are given and sets them globally for the flowing execution.
 * If -h tag is detacted then a help-message will be printend and the execution will be stopped.
 *
 * Returns 1 if -h is found.
 *
 * @param my_rank Rank of the processor-
 * @param argc Number of args.
 * @param argv Args.
 * @return int  1 if  termination is needed.
 */
int utilCheckParameters(int my_rank, int argc, char *argv[])
{
	int terminate = 0;
	int parameterMFound = 0;
	for (int i = 0; i < argc; i++)
	{
		if ((strcmp(argv[i], "-m") == 0))
		{
			if (argv[i + 1])
			{
				mValue = atoi(argv[i + 1]);
				parameterMFound = 1;
			}
			else
			{
				if (my_rank == 0)
				{
					printf(
						"Problem occurs while trying to cast input m <%s> to int",
						argv[i + 1]);
					utilPrintHelp();
				}
				terminate = 1;
			}
		}
		if (strcmp(argv[i], "-h") == 0)
		{
			if (my_rank == 0)
			{
				utilPrintHelp();
				terminate = 1;
				break;
			}
			else
			{
				terminate = 1;
			}
		}
	}
	if (parameterMFound != 1)
	{
		if (my_rank == 0)
		{
			printf(
				"Problem occurs couldn't find parameter m. Checkout help-section.");
			utilPrintHelp();
		}
		terminate = 1;
	}
	if (terminate == 1)
	{
		exit(0);
	}

	if (mValue < 1)
	{
		utilOTPrint(0, my_rank, "God bless, nothing to do. Thanks to you!\n");
	}
	return terminate;
}

/**
 * @brief Prints a messag to the screen only onces.  * 
 * @param rankWhichPrints  Which rank should print the message.
 * @param my_rank Rank of the machine.
 * @param message Message to print.
 */
void utilOTPrint(int rankWhichPrints, int my_rank, char message[])
{
	if (my_rank == rankWhichPrints)
	{
		printf("%s ", message);
	}
}

void utilOTPrintWRank(int rankWhichPrints, int my_rank, char message[])
{
	if (my_rank == rankWhichPrints)
	{
		printf("%d : %s ", my_rank, message);
	}
}

/**
 * @brief 
 * Cancels the program-execution if needed.
 * 
 * @param terminate If 1 then stop execution.
 */
int utilTerminateIfNeeded(int terminate)
{
	if (terminate == 1)
	{
		printf("Execution will be canceled");
		exit(0);
	}
}

/**
 * @brief 
 * Canceled the program-execution if needed.
 * 
 * @param terminate 1- to stop execution.
 */
int utilTerminateIfNeededSilently(int terminate)
{
	if (terminate == 1)
	{
		exit(0);
	}
}

int utilPrintHelp()
{
	printf("\n");
	printf("-----------------------------------------------------[Help]--\n");
	printf("\n");
	printf("Program is optimized for less then  99998 given nodes. \n");
	printf("Every parameter marked with <*> is required!\n");

	printf(
		"Parameter* -m <number>: Specifies the amount of randome numbers each process need to generate.\n");
	printf("\n");
	printf("\n");
}
