#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

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


int main(int argc, char **argv)
{
	int NRA = string_to_num(argv[2]);
	int NCA = string_to_num(argv[3]);
	int NCB = string_to_num(argv[5]);
	int i, j;
	FILE *fp = fopen(argv[1], "w");

	fprintf(fp, "%d %d %d %d\n", NRA, NCA, NCA, NCB);
	fprintf(fp, "\n");
	for(i = 0; i < NRA; i++)
	{
		for(j=0; j < NCA; j++)
		{
			if (j == NCA-1)
			{
				fprintf(fp, "%d", (i+j));
			}
			else
				fprintf(fp, "%d ", (i+j));
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");
	for(i = 0; i < NCA; i++)
	{
		for(j=0; j < NCB; j++)
		{
			if (j == NCB-1)
			{
				fprintf(fp, "%d", (i*j));
			}
			else
				fprintf(fp, "%d ", (i*j));

		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	return 0;
}
