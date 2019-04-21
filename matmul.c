#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

int **allocate_mat(int rows, int cols) {
    int *data = (int *)malloc(rows*cols*sizeof(int));
    int **array= (int **)malloc(rows*sizeof(int*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}
int calc_pow(int a, int b)
{
	int temp = 1;
	while(b > 0)
	{
		temp = temp * a;
		b--;
	}
	return temp;
}

int string_to_num(char *str)
{
    int end = strlen(str) - 1;
    int i = 0;
    int total = 0;
    while(end >= 0)
    {
        char ch = str[end--];
        int digit = ch - '0';
		int temp_pow = calc_pow(10, i);
        total = (temp_pow * digit) + total;
        i++;
    }
	return total;
}

void mat_from_line(int ***mat_p, char *line, int line_no)
{
	char *ptr = line, *word, temp[5];
	int digits = 0;
	int word_count = 0;
	int **mat = *mat_p;
	while(1){
		digits = 0;

		while((*ptr != ' ') && (*ptr != '\n'))
		{
			if (digits == 0)
			{
				int i = 0;
				for (i = 0; i < 5; i++)
				{
					temp[i] = '\0';
				}
				word = &temp[0];
			}
			*word = *ptr;
			word++;ptr++;
			digits++;
		}
		if ((*ptr == ' ') || (*ptr == '\n'))
		{
			*word = '\0';
			word++;
			mat[line_no][word_count] = string_to_num(temp);
			word_count++;
			//free(temp);
			if (*ptr == '\n')
			{
				//free(temp);
				return;
			}
			else if (*ptr == ' ')
			{
				ptr++;
				//free(temp);	
			}
		}
			
	}
	return;	
}


void get_rows_cols(char *line, int *NRA, int *NCA, int *NCB)
{
	char *ptr = line, *word, temp[5];
	int digits = 0;
	int word_count = 0;
	while(1) {
		digits = 0;	
		while((*ptr != ' ') && (*ptr != '\n'))
		{
			if (digits == 0)
			{
				int i = 0;
				for (i = 0; i < 5; i++)
				{
					temp[i] = '\0';
				}
				word = &temp[0];
			}
			*word = *ptr;
			word++;ptr++;
			digits++;
		}
		if ((*ptr == ' ') || (*ptr == '\n')) {
			ptr++;
			*word = '\0';
			word++;
			if (word_count == 0) {
				*NRA = string_to_num(temp);
			}
			else if (word_count == 2) {
				*NCA = string_to_num(temp);
			}
			else if (word_count == 3) {
				*NCB = string_to_num(temp);
				break;
			}
			word_count++;
			if (*ptr == '\n')
			{
				break;
			}

			
		}

	}
	return;	
}
int main (int argc, char *argv[])
{
int	numtasks,/* number of tasks in partition */
	NRA,/*number of rows in matrix A*/
	NCA,/*number of rows in matrix A*/
	NCB,/*number of rows in matrix A*/        
	taskid,                /* a task identifier */
	numworkers,            /* number of worker tasks */
	source,                /* task id of message source */
	dest,                  /* task id of message destination */
	mtype,                 /* message type */
	rowz,                  /* rows of matrix A sent to each worker */
	averow, extra, offset, /* used to determine rows sent to each worker */
	i, j, k, rc,row_no,col_no;           /* misc */
int	**a,           /* matrix A to be multiplied */
	**b,           /* matrix B to be multiplied */
	**c;           /* result matrix C */
size_t bufsize = 100;
MPI_Status status;

	MPI_Init(NULL,NULL);
	MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
	
	char *fname = argv[1];
	FILE *fp = fopen(fname, "r");

	char *buffer = (char*)malloc(sizeof(char) * 100);
	int chars = getline(&buffer,&bufsize, fp);
	printf("%s", buffer);
	get_rows_cols(buffer, &NRA, &NCA, &NCB);
	//printf("\nNRA=%d NCA=%d NRB=%d NCB=%d....\n", NRA, NCA, NCA, NCB);	
	free(buffer);
	a = allocate_mat(NRA, NCA);
	b = allocate_mat(NCA, NCB);
	c = allocate_mat(NRA, NCB);
	//printf("Getting Matrices A & B.....");
	
	
	if (numtasks < 2 )
	{
		  printf("Need at least two MPI tasks. Quitting...\n");
		  MPI_Abort(MPI_COMM_WORLD, rc);
		  exit(1);
  	}
	numworkers = numtasks-1;
	//printf("numworkers %d\n",numworkers);


/**************************** master task ************************************/
   if (taskid == MASTER)
   {
   	
	//printf("Getting Matrices A & B.....");
	chars = getline(&buffer, &bufsize, fp);	//Read newline
	bufsize = sizeof(long) * NCA;
   	for (row_no = 0 ; row_no < NRA; row_no++)
		{
			//printf("Getting Mat A.....");
			buffer = (char*)malloc(bufsize);
			chars = getline(&buffer, &bufsize, fp);
			mat_from_line(&a, buffer, row_no);
			free(buffer);
		}

		chars = getline(&buffer, &bufsize, fp);	//Read newline
		bufsize = sizeof(long) * NCB;	//We consider that each number is 8 bytes max e.g. "12345678"
      	for (row_no = 0 ; row_no < NCA; row_no++)
		{
		
			buffer = (char*)malloc(bufsize);
			chars = getline(&buffer, &bufsize, fp);
			mat_from_line(&b, buffer, row_no);
			free(buffer);
		}
	printf("mpi_mm has started with %d tasks.\n",numtasks);
        printf("Initializing arrays...\n");
      
      printf("\nMatrix A:\n");
      for (i=0; i<NRA; i++)
      {
         printf("\n"); 
         for (j=0; j<NCA; j++) 
            printf("%d   ", a[i][j]);
      }
      
      printf("\nMatrix B:\n");
      for (i=0; i<NCA; i++)
      {
         printf("\n"); 
         for (j=0; j<NCB; j++) 
            printf("%d  ", b[i][j]);
      }
      /* Send matrix data to the worker tasks */
      averow = NRA/numworkers;
      //printf("\naverow = %d",averow);
      extra = NRA%numworkers;
      //printf("\nextra = %d",extra);
      offset = 0;
      mtype = FROM_MASTER;
      for (dest=1; dest<=numworkers; dest++)
      {
         rowz = (dest <= extra) ? averow+1 : averow;   
         //printf("\nrows = %d",rows);	
         printf("\nSending %d rows to task %d offset=%d\n",rowz,dest,offset);
         MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&rowz, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&(a[offset][0]), rowz*NCA, MPI_DOUBLE, dest, mtype,MPI_COMM_WORLD);
         MPI_Send(&(b[0][0]), NCA*NCB, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
         offset = offset + rowz;   //update offset
         //printf("Task Sent");
      }
	
      /* Receive results from worker tasks */
      mtype = FROM_WORKER;
      for (i=1; i<=numworkers; i++)
      {
         source = i;
         MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&rowz, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&(c[offset][0]), rowz*NCB, MPI_DOUBLE, source, mtype,MPI_COMM_WORLD, &status);
         printf("Received results from task %d\n",source);
      }

      /* Print results */
      printf("******************************************************\n");
      printf("Result Matrix:\n");
      for (i=0; i<NRA; i++)
      {
         printf("\n"); 
         for (j=0; j<NCB; j++) 
            printf("%d", c[i][j]);
      }
      printf("\n******************************************************\n");
      printf ("Done.\n");
   }


/**************************** worker task ************************************/
   if (taskid > MASTER)
   {
      mtype = FROM_MASTER;
      MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&rowz, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&(a[offset][0]), rowz*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&(b[0][0]), NCA*NCB, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);

      for (k=0; k<NCB; k++)
         for (i=0; i<rowz; i++)
         {
            c[i][k] = 0.0;
            for (j=0; j<NCA; j++)
               c[i][k] = c[i][k] + a[i][j] * b[j][k];
         }
      mtype = FROM_WORKER;
      MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&rowz, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&(c[0][0]), rowz*NCB, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
   }
   MPI_Finalize();
   free(a);
   free(b);
   free(c);
}

