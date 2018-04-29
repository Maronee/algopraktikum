//#include "mpi.h"	// import of the MPI definitions
#include <stdio.h>  // import of the definitions of the C IO library
#include <string.h> // import of the definitions of the string operations
#include <unistd.h> // standard unix io library definitions and declarations
#include <errno.h>  // system error numbers

#include <stdlib.h>

int* getRandom();
void mpirun();
int world_size;

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);					 // initializing of MPI-Interface
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //get your rank

	// init call
	 srand(time(NULL)); // should only be called once
	 int r = rand();	// returns a pseudo-random integer between 0 and RAND_MAX
	 int namelen;							 // length of name
	 int my_rank;							 // rank of the process
	 MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	 char *c, proc_name[MPI_MAX_PROCESSOR_NAME + 1]; // hostname

	 memset(proc_name, 0, MPI_MAX_PROCESSOR_NAME + 1);
	 // initialize string with NULL-characters
	 MPI_Get_processor_name(proc_name, &namelen);
	 // finding out own computer name

//	int *a;
//	a=getRandom();
	 mpirun();



}

int* getRandom() {
			int m= rand();
			int r[m];
			int i;
			int *b;
			b=(int*)malloc(4*sizeof(int));
			int max=0;
			int min;


			/* set the seed */

			for ( i = 0; i < m; ++i) {
				  r[i] = rand();
				  b[2]+=r[i];
				  if (r[i] > max)
				              {
				            	max = r[i];
				            	b[0]=max;
				              }
				  if (r[i] < min)
				              {
				        	 	min = r[i];
				        	 	b[1]=r[i];
				              }
				printf( "r[%d] = %d\n", i, r[i]);

			}
			b[3]=b[2]/m;

			printf( "b[%d] = %d\n", 0, b[0]);
			printf( "b[%d] = %d\n", 1, b[1]);
			printf( "b[%d] = %d\n", 2, b[2]);
			printf( "b[%d] = %d\n", 3, b[3]);
			return b;
}


void mpirun(){
	int a;
	int *b;
	b=getRandom();
	for( a = 0; a <= world_size; a = a + 1 ){
		if (world_size==a){
			MPI_Recv(&b, 4, MPI_INT, my_rank -1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Send(&b, 4, MPI_INT, 0, 1, MPI_COMM_WORLD);
			printf("von world_size erhalten");

			}
		if (world_size==a){
					MPI_Recv(&b, 4, MPI_INT, world_size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					MPI_Send(&b, 4, MPI_INT, my_rank+1, 1, MPI_COMM_WORLD);
			printf("von 0 erhalten");
		}
		else{
			MPI_Recv(&b, 4, MPI_INT, my_rank -1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Send(&b, 4, MPI_INT, my_rank +1, 1, MPI_COMM_WORLD);
			printf("von node erhalten");
		}
	}

}

















