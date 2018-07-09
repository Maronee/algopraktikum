/************************************************************************/
/* Program: a4_1.c                                                   	*/
/* Author: Hans-Joachim Hammer, Sebastian Voinea                        */
/*  s4727687@stud.uni-frankfurt.de, sebastian.voinea@gmx.de             */
/* matriclenumber:5245679, 4498171                                      */
/* Assignment : 4                                                       */
/* Task: 1                                                              */
/* Parameters: -help -debug -m  										*/
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
	int count_total =size;
	int columns = 1280;
	int rows= 960;
	int my_rows = rows / world_size;
	int my_count = count_total / world_size;

	//int count=rows;
	//int blocklength=my_count/rows;
	//int stride=columns;
	//MPI_Type_vector(count,blocklength,stride,MPI_UNSIGNED_CHAR,&column_t);
	//MPI_Type_commit(&column_t);
	//int vec_size;
	//MPI_Aint realsize, lb;
	//MPI_Type_get_extent(column_t, &lb, &realsize);


	//MPI_Type_size(column_t, &vec_size);

	//MPI_File_set_view(filehandle, 0, MPI_UNSIGNED_CHAR,column_t,"native",MPI_INFO_NULL);

	unsigned char *char_buffer = malloc(my_count);

	MPI_File_read_ordered(filehandle, char_buffer, my_count, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);


	//MPI_File_read_ordered(filehandle, char_buffer,my_count, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);
	MPI_File_close(&filehandle);
	//printf("test\n");
	double *result_buffer = (double*) malloc(my_count * sizeof(double));
	double *calc_buffer = (double*) malloc(my_count * sizeof(double));

	for (int i=0; i<my_count; i++){
		calc_buffer[i]=(double)(char_buffer[i]);
		//printf("[%lf]\n",calc_buffer[i]);
	}

	int choice=0;
	if (my_rank==0){
	printf("Enter 0 for Blur, 1 for Sharpen, 2 for Relief and 3 for Edge Detection\n");
	scanf("%i", &choice);
	}
	MPI_Bcast(&choice, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if (debug){
	printf("Size status: %i , size: %lli , elements: %i, rows: %i\n", err, size,my_count, rows);
	}



	 double *blur = (double*) malloc(5*5*sizeof(double));
	//declare blur matrix
	blur[0]=0 * (1/37);
	blur[1]=0 * (1/37);
	blur[2]=1 * (1/37);
	blur[3]=0 * (1/37);
	blur[4]=0 * (1/37);
	blur[5]=0 * (1/37);
	blur[6]=2 * (1/37);
	blur[7]=4 * (1/37);
	blur[8]=2 * (1/37);
	blur[9]=0 * (1/37);
	blur[10]=1 * (1/37);
	blur[11]=4 * (1/37);
	blur[12]=9 * (1/37);
	blur[13]=4 * (1/37);
	blur[14]=1 * (1/37);
	blur[15]=0 * (1/37);
	blur[16]=2 * (1/37);
	blur[17]=4 * (1/37);
	blur[18]=2 * (1/37);
	blur[19]=0 * (1/37);
	blur[20]=0 * (1/37);
	blur[21]=0 * (1/37);
	blur[22]=1 * (1/37);
	blur[23]=0 * (1/37);
	blur[24]=0 * (1/37);

	//declare sharpen matrix
	double *sharpen = (double*) malloc(5*5*sizeof(double));
	for (int i=0; i<25; i++){

		if (i!=7 | i != 11 | i!=12 | i!=13 | i!=17)
		{
			sharpen[i]=0;
		}
	}
	sharpen[7] = -1;
	sharpen[11] = -1;
	sharpen[12] = 5;
	sharpen[13] = -1;
	sharpen[17] = -1;

	// declare relief array
	double *relief = (double*) malloc(5*5*sizeof(double));
	for (int i=0; i<25; i++){
		if (i==6 | i== 7 | i==11 | i == 12 | i == 13| i==17 | i==18){
		continue;

		}
		else {
		relief[i]=0;
		}
	}
	relief[6]=-2;
	relief[7]=-1;
	relief[11]=-1;
	relief[12]=1;
	relief[13]=1;
	relief[17]=1;
	relief[18]=2;

	// edge detection array
	double *edge = (double*) malloc(5*5*sizeof(double));
	for (int i=0; i< 25; i++){
		if (i==6|i==7|i==8|i==11|i==12|i==13|i==16|i==17|i==18){
		continue;
		}
		edge[i]=0;
		}
	edge[6]=1 * 0.25;
	edge[7]=2 * 0.25;
	edge[8]=1 * 0.25;
	edge[11]=2 * 0.25;
	edge[12]=-12 * 0.25;
	edge[13]=2 * 0.25;
	edge[16]=1 * 0.25;
	edge[17]=2 * 0.25;
	edge[18]=1 * 0.25;

	MPI_Comm My_Comm;
	int dims = 1;
	int dim_size[1] = { world_size };
	int wrap [1] = { 0 };	//o = false, 1 = true for wrapping
	int reorder = 0;

	MPI_Cart_create(MPI_COMM_WORLD, dims, dim_size, wrap, reorder, &My_Comm);

	if(debug == 1){
		int coords[1] ={0};
		MPI_Cart_coords(My_Comm, my_rank, dims, coords);
		printf("[%i] coords: %i \n", my_rank, coords[0]);
	}


	int right, left;
	MPI_Cart_shift(My_Comm, 0, 1, &left, &right);
	// where target = my_rank + 1 and source = my_rank - 1; value -1 for processes that dont exist

	double *recvBot = (double*) malloc(columns * 2 * sizeof(double));
	double *recvTop = (double*) malloc(columns * 2 * sizeof(double));

	// MPI_Request sr, sl, rr, rl;
	MPI_Request *rrr = (MPI_Request *)malloc(4 * sizeof(MPI_Request));
	double *botRow = (double*) malloc(columns * 2 * sizeof(double));
	double *topRow = (double*) malloc(columns * 2 * sizeof(double));

	err = MPI_Send_init(botRow, columns * 2, MPI_DOUBLE, right, 0, My_Comm, &rrr[0]);
	err = MPI_Send_init(topRow, columns * 2, MPI_DOUBLE, left, 0, My_Comm, &rrr[1]);
	err = MPI_Recv_init(recvBot, columns * 2, MPI_DOUBLE, right, 0, My_Comm, &rrr[2]);
	err = MPI_Recv_init(recvTop, columns * 2, MPI_DOUBLE, left, 0, My_Comm, &rrr[3]);

	//calculations
	for(int k = 0; k < m; k++){
		//prereq
		memcpy(result_buffer, calc_buffer, my_count * sizeof(double));

		for(int i = 0; i < 2 * columns; i++){
			if(my_rank != 0){
				topRow[i] = result_buffer[i];
			} else {
				topRow[i] = 0;
			}
		}

		for(int i = 0; i < 2 * columns; i++){
			if(my_rank != world_size - 1){
				botRow[i] = result_buffer[(my_rows - 2) * columns + i];
			} else {
				botRow[i] = 0;
			}
		}

		//communicate missing rows
		MPI_Startall(4, rrr);
		MPI_Waitall(4, rrr, MPI_STATUS_IGNORE);

		if(debug == 1){
			printf("[%i] local 0,1 top %lf recv top %lf\n", my_rank, topRow[1], recvTop[1]);
			printf("[%i] local 1,1 top %lf recv top %lf\n", my_rank, topRow[1 * columns + 1], recvTop[1 * columns + 1]);
			printf("[%i] local 0,1 bot %lf recv bot %lf\n", my_rank, botRow[1], recvBot[1]);
			printf("[%i] local 1,1 bot %lf recv bot %lf\n", my_rank, botRow[1 * columns + 1], recvBot[1 * columns + 1]);
		}

		if(right != -1){
			memcpy(botRow, recvBot, 2 * columns * sizeof(double));
		}
		if(left != -1){
			memcpy(topRow, recvTop, 2 * columns * sizeof(double));
		}
		
		for(int i = 0; i < my_count; i++){
			double sum = 0;
			for(int u = 0; u < 5; u++){
				for(int v = 0; v < 5; v++){
					int filterindex = u * 5 + v;
					// u-2 und v-2 sind zur verschiebung gedacht, somit ist 2,2 die mitte der matrix
					int col_index = i % columns + v - 2;
					int row_index = i / columns + u - 2;
					double array_val, filter_val;
					int arrayindex;
					// alle werte ausserhalb des array sind 0 und ergeben im produkt 0
					//if(col_index >= 0 && row_index >= 0){
					if( col_index >= 0 && col_index < columns && row_index < 0 ){
						arrayindex = (row_index + 2) * columns + col_index;
						array_val = topRow[arrayindex];
						// if(i == arrayindex && debug == 1){
						// 	printf(" (%i,%i) \n", row_index + 2, col_index);
						// }
						
					}
					else if( col_index >= 0 && col_index < columns && row_index >= my_rows ){
						arrayindex = (row_index - my_rows) * columns + col_index;
						array_val = botRow[arrayindex];
						// if(i == row_index * columns + col_index - 2 * columns  && debug == 1){
						// 	printf(" (%i,%i) \n", row_index - my_rows, col_index);
						// }
						//printf("%i %i \n", col_index, row_index);
					}
					else if( col_index >= 0 && col_index < columns && row_index >= 0 && row_index < my_rows){
						arrayindex = row_index * columns + col_index;
						array_val = result_buffer[arrayindex];
					}

					switch(choice){
						case 0:
							filter_val = blur[filterindex];
							break;
						case 1:
							filter_val = sharpen[filterindex];
							break;
						case 2:
							filter_val = relief[filterindex];
							break;
						case 3:							
							filter_val = edge[filterindex];							
							break;
					}

					sum += filter_val * array_val;
				}
			}
			
			//printf("[%i] sum before %lf\n", my_rank, sum);

			if(sum < 0){
				sum = 0;
			} else if(sum > 255){
				sum = 255;
			}
			calc_buffer[i] = sum;
			//printf("[%i] sum after %lf\n", my_rank, sum);
			//exit(0);
		}
	}
	free(result_buffer);

	for (int i=0; i<my_count; i++){
		char_buffer[i]=(unsigned char)(calc_buffer[i]);
	}

	MPI_File_open(My_Comm, "result_img.gray", MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &filehandle);
	MPI_File_write_ordered(filehandle, char_buffer, my_count, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);
	MPI_File_close(&filehandle);

	free(char_buffer);
	free(calc_buffer);
	//free my allocations
	free(recvBot);
	free(recvTop);
	free(botRow);
	free(topRow);
	MPI_Request_free(rrr);
	// MPI_Request_free(&sr);
	// MPI_Request_free(&sl);
	// MPI_Request_free(&rr);
	// MPI_Request_free(&rl);
	MPI_Finalize();		            // finalizing MPI interface
	return 0;						// end of progam with exit code 0
}