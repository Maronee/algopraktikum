/***********************************************************************
 Program: aufgabe1.3.b.c                                                  
 Author: Michael Czaja, Muttaki Aslanparcasi                                           
 matriclenumber: 4293033, 5318807                                            
 Assignment : 1                                                      
 Task: 3                                                             
 Parameters: yes -m (amount of numbers to generate)                                                      
 Environment variables: no                                           

 Description:                                                        

 Program generates a random set of number (depends on m-value), det the min-value,
 max-value and calcs the sum + avg-sum values of the set. After that, each node
 will share his results with the cluster and the overall result will be calculated
 and printed by the root-node.

 The communication is working over bin-tree pattern. 
 /************************************************************************/

#include "mpi.h"	// import of the MPI definitions
#include <stdio.h>  // import of the definitions of the C IO library
#include <string.h> // import of the definitions of the string operations
#include <unistd.h> // standard unix io library definitions and declarations
#include <errno.h>  // system error numbers
#include <time.h>

#include <stdlib.h>
#include <math.h>

#define MAX_BUFFER_SIZE 1000
int utilCheckParameters(int my_rank, int argc, char *argv[]);
int utilTerminateIfNeeded(int terminate);
int rootGetAndPrintNodeRandInRingMode(char tag[], int commLine, int myRank,
									  int myRand);

double taskLookForMin(double myInitNumbers[], int size);
double taskLookForMax(double myInitNumbers[], int size);
double taskCalcSum(double myInitNumbers[], int size);
double taskCalcAVG(double number, int amount);

int utilTerminateIfNeededSilently(int terminate);
int utilPrintHelp();
void utilOTPrint(int rankWhichPrints, int my_rank, char message[]);
void utilPrintArray(double array[], int size);
void utilOTPrintWRank(int rankWhichPrints, int my_rank, char message[]);

int world_size;
int mValue;
int commLine = 9999;
int my_rank; // rank of the process

int main(int argc, char *argv[])
{
	// -----------------------------------------------------------------[Init]--
	int namelen;							 // length of name
	MPI_Init(&argc, &argv);					 // initializing of MPI-Interface
	double mpi_programStart = MPI_Wtime();
	double mpi_timeInCom = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //get your rank

	// init call
	srand(time(NULL)); // should only be called once
	int mesCommline = 99999;
	int round = 4;
	int didISend = 0;


	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	char buffer[(MAX_BUFFER_SIZE * world_size) + MPI_BSEND_OVERHEAD];
	int bsize = sizeof(buffer);

	// ----------------------------------------------------------[ Para check ]--

	int needToTerminate = utilCheckParameters(my_rank, argc, argv);

	// ---------------------------------------------------------[ Data create ]--

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
	double mySUMValue = taskCalcSum(myInitNumbers, mValue);
	double myAVGValue = taskCalcAVG(mySUMValue, mValue);
	double resultAVG = -1;

	printf("Min for array <%lf>\n", myMinValue);
	printf("Max for array <%lf>\n", myMaxValue);
	printf("SUM for array <%lf>\n", mySUMValue);
	printf("AVG for array <%lf>\n", myAVGValue);

	dataToSend[0] = myMinValue;
	dataToSend[1] = myMaxValue;
	dataToSend[2] = mySUMValue;
	// -----------------------------------------------------------[ pre Init ]--

	char *c, proc_name[MPI_MAX_PROCESSOR_NAME + 1]; // hostname

	memset(proc_name, 0, MPI_MAX_PROCESSOR_NAME + 1);
	// initialize string with NULL-characters
	MPI_Get_processor_name(proc_name, &namelen);
	// finding out own computer name

	if ((c = strchr(proc_name, '.')) != NULL)
		*c = '\0';

	// ------------------------------------------------------------[Init Call]--

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	int roundsToPlay = taskCalcHowManyRound(world_size);

	// ------------------------------------------------------------[Para Part]--
	double mpi_loopStart = MPI_Wtime();
	for (int currentRound = roundsToPlay; currentRound > 0; currentRound--)
	{
		MPI_Barrier(MPI_COMM_WORLD);
		usleep(100);
		int second = (1 << currentRound - 1);
		int partner = my_rank ^ second;
		if (my_rank < partner)
		{
			if (didISend == 1)
			{
				// Nothing to do
			}
			// node is receiver.
			else
			{
				double mpi_comStart = MPI_Wtime();
				MPI_Recv(&dataToRev, 3, MPI_DOUBLE, partner, round, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				double mpi_comEnd = MPI_Wtime();
				mpi_timeInCom += (mpi_comEnd-mpi_comStart);
				double combinedMin[2] = {dataToSend[0], dataToRev[0]};
				double combinedMax[2] = {dataToSend[1], dataToRev[1]};
				double combinedSum = dataToSend[2] + dataToRev[2];
				dataToSend[0] = taskLookForMin(combinedMin, 2);
				dataToSend[1] = taskLookForMax(combinedMax, 2);
				dataToSend[2] = combinedSum;
				resultAVG = dataToSend[2] / mValue;
				printf("%d <-  %d\n", my_rank, partner);
			}
		}
		// node is sender.
		else
		{
			if (didISend == 0)
			{
				printf("%d ->  %d\n", my_rank, partner);
				double mpi_comStart = MPI_Wtime();
				MPI_Send(&dataToSend, 3, MPI_DOUBLE, partner, round, MPI_COMM_WORLD);
				double mpi_comEnd = MPI_Wtime();
				mpi_timeInCom += (mpi_comEnd-mpi_comStart);
				didISend = 1;
			}
			else
			{
				//Nothing to do. Was already a sender.
			}
		}
	}
	double mpi_loopEnd = MPI_Wtime();
	double mpi_programEnd = MPI_Wtime();
	
	double mpi_timeInLoop = mpi_loopEnd-mpi_loopStart;
	double mpi_programRuntime = mpi_programEnd -mpi_programStart ;


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
		
		printf("------------ MEASUREMENTS -----------\n");
		printf("Node totalProTime timeInPara timeForCom timeInSeq       |\n");
	}
	//-----------------------------------------------------------------[ END ]--
	MPI_Barrier(MPI_COMM_WORLD);
	usleep(100);
	MPI_Buffer_detach((void *)buffer, &bsize);
	MPI_Finalize(); // finalizing MPI interface
	
	//----------------------------------------------------------------[ Mes. ]--
	printf("<%d> <%lf> <%lf> <%1f> <%1f>         |\n", my_rank, mpi_programRuntime, mpi_timeInLoop, mpi_timeInCom, (mpi_programRuntime-mpi_timeInLoop));

	return 0; // end of progam with exit code 0
}

// ------------------------------------------------------------------[ TaskFunc. ]--

/**
 * @brief Check for the minimum in an array.
 * 
 * @param array The array.
 * @param size Size of array.
 * @return double The minimum of the array.
 */
double taskLookForMin(double array[], int size)
{
	double result = array[0];

	for (int i = 0; i < size; i++)
	{
		if (result > array[i])
			result = array[i];
	}

	return result;
}

/**
 * @brief Check for the maximum in an array.
 * 
 * @param array The array.
 * @param size Size of array.
 * @return double The maximum of the array.
 */
double taskLookForMax(double array[], int size)
{
	double result = array[0];

	for (int i = 0; i < size; i++)
	{
		if (result < array[i])
			result = array[i];
	}

	return result;
}


/**
 * @brief Checks if a number is a power of 2.
 * 
 * @param x Number
 * @return int 1 if so.
 */
int taskIspowerof2(unsigned int x)
{
	return x && !(x & (x - 1));
}

/**
 * @brief Calcs the communications-rounds.
 * 
 * @param sizeOfWorld Size of the world(# nodes).
 * @return int Number of rounds to communicate.
 */
int taskCalcHowManyRound(int sizeOfWorld)
{
	int rounds = log(sizeOfWorld) / log(2);
	return rounds;
}


/**
 * @brief Checks if node is sender or receiver.
 * 
 * @param myRank Rank of the node.
 * @param partnerRank Rank of the partner-node.
 * @return int 1 if node is sender.
 */
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
 * @brief Calcs the sum of a given array.
 * 
 * @param array The array.
 * @param size Size of array.
 * @return double The sum of the array.
 */
double taskCalcSum(double array[], int size)
{
	double result = 0;

	for (int i = 0; i < size; i++)
	{
		result += array[i];
	}

	return result;
}
/**
 * @brief  Calcs the avg.
 * 
 * @param number Number.
 * @param amount how many numbers are used.
 * @return double The avg.
 */
double taskCalcAVG(double number, int amount)
{
	return number / amount;
}

// ------------------------------------------------------------------[ UTILS ]--

/**
 * @brief Prints and array.
 * 
 * @param array The array.
 * @param size Size of array.
 */
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
 * Checks if all parameters are given and sets them globally for the following execution.
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

	if (mValue < 1)
	{
		terminate = 1;
		utilOTPrint(0, my_rank, "God bless, nothing to do. Thanks to you!\n");
	}
	if (taskIspowerof2(world_size) != 1)
	{
		terminate = 1;
		utilOTPrint(0, my_rank, "ERROR -----------------\n");
		utilOTPrint(0, my_rank, "The number of nodes isn't a power of 2. Please check the -n parameter.\n");
		utilPrintHelp();
	}

	if (terminate == 1)
	{
		exit(0);
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

/**
 * @brief Prints message. Message contains the node-rank.
 * 
 * @param rankWhichPrints Node, which should print.
 * @param my_rank rank of the node.
 * @param message  message to print.
 */
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
 * Aborts the program-execution if needed.
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
/**
 * @brief Prints the help-message.
 * 
 * @return int 0 if successful.
 */
int utilPrintHelp()
{
	utilOTPrint(0, my_rank, "\n");
	utilOTPrint(0, my_rank, "-----------------------------------------------------[Help]--\n");
	utilOTPrint(0, my_rank, "\n");
	utilOTPrint(0, my_rank, "Program is optimized for less then  99998 given nodes. \n");
	utilOTPrint(0, my_rank, "Every parameter marked with <*> is required!\n");

	utilOTPrint(0, my_rank, "Parameter* -m <number>: Specifies the amount of randome numbers each process need to generate.\n");
	utilOTPrint(0, my_rank, "\n");
	utilOTPrint(0, my_rank, "\n");
	return 0;
}
