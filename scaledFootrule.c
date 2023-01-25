
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXURL 1000
#define MAXURLLEN 100
#define LARGE 9999999.0

static void initList(char *list[], char *files[], int num, int size);
int getIndex(char *name, char *file);
static void recursivePerm(char *list[], char *ordered[], int numFiles,
						  char *files[], double *lowestRankPt, int start,
						  int end);
double footCalculator(int numFiles, char *files[], char *list[], int size);
int numPagesFile(char *file);
static void freeArrs(char *list[], int num);
int initUrls(char *file[], int numFiles);
static void printResults(char *ordered[], double lowestRank, int num);

int main(int argc, char *argv[])
{
	int num = initUrls(argv, argc);
	char **list = malloc(sizeof(char *) * num);
	char **ordered = malloc(sizeof(char *) * num);
	if (list == NULL || ordered == NULL)
	{
		fprintf(stderr, "Ran out of memory!");
		return EXIT_FAILURE;
	}
	for (int i = 0; i < num; i++)
	{
		ordered[i] = malloc(sizeof(char *));
		list[i] = malloc(sizeof(char *));
	}
	initList(list, argv, argc, num);
	double lowestRank = LARGE;
	double *lowestRankPt = &lowestRank;
	recursivePerm(list, ordered, argc, argv, lowestRankPt, 0, num);
	printResults(ordered, lowestRank, num);
	freeArrs(list, num);
	freeArrs(ordered, num);
}

// prints the lowest rank aggregation and order of pages to the terminal.
static void printResults(char *ordered[], double lowestRank, int num)
{
	printf("%.7lf\n", lowestRank);
	for (int k = 0; k < num; k++)
	{
		printf("%s\n", ordered[k]);
	}
}

// Free's the given arrays
static void freeArrs(char *list[], int num)
{
	for (int i = 0; i < num; i++)
	{
		free(list[i]);
	}
	free(list);
}

// Recurses through all possible permutations and tracks the lowest scaled foot rule distance
// and the corresponding order of pages.
static void recursivePerm(char *list[], char *ordered[], int numFiles,
						  char *files[], double *lowestRankPt, int start,
						  int end)
{
	if (end == start)
	{
		double currVal = footCalculator(numFiles, files, list, end);
		if (*lowestRankPt > currVal)
		{
			for (int i = 0; i < end; i++)
			{
				strcpy(ordered[i], list[i]);
			}
			*lowestRankPt = currVal;
		}
	}
	int j;
	for (j = start; j < end; j++)
	{
		char *temp = list[j];
		list[j] = list[start];
		list[start] = temp;
		recursivePerm(list, ordered, numFiles, files, lowestRankPt, start + 1,
					  end);
		temp = list[j];
		list[j] = list[start];
		list[start] = temp;
	}
}

// Calculates the scaled footrule distance for the given permutation.
double footCalculator(int numFiles, char *files[], char *list[], int size)
{
	double totSum = 0.0;
	for (int i = 0; i < size; i++)
	{
		for (int j = 1; j < numFiles; j++)
		{
			int index = getIndex(list[i], files[j]);
			int numPages = numPagesFile(files[j]);
			if (index == 0)
			{
				continue;
			}
			double temp = index;
			temp /= numPages;
			double temp2 = i + 1;
			temp2 /= size;
			temp -= temp2;
			totSum += fabs(temp);
		}
	}
	return totSum;
}

// Returns the number of pages in a file.
int numPagesFile(char *file)
{
	FILE *curr = fopen(file, "r");
	if (curr == NULL)
	{

		fprintf(stderr, "File does not exist!");
		exit(EXIT_FAILURE);
	}
	int num = 0.0;
	char buff[MAXURLLEN];
	while (fscanf(curr, "%s", buff) != EOF)
	{
		num++;
	}
	fclose(curr);
	return num;
}

// Return the position of a given page in the given file.
// Returns 0 if the page does not exist in the file.
int getIndex(char *name, char *file)
{
	FILE *curr = fopen(file, "r");
	if (curr == NULL)
	{
		fprintf(stderr, "File does not exist!");
		return EXIT_FAILURE;
	}
	int index = 0;
	char buff[MAXURLLEN];
	bool present = false;
	while (fscanf(curr, "%s", buff) != EOF)
	{
		if (strcmp(buff, name) == 0)
		{
			present = true;
			break;
		}
		index++;
	}
	fclose(curr);
	if (present)
	{
		return index + 1;
	}
	return 0;
}

// Initialises the list with all the pages
static void initList(char *list[], char *files[], int num, int size)
{
	char urlName[MAXURLLEN];
	bool exists = false;
	int count = 0;
	for (int i = 1; i < num; i++)
	{
		FILE *curr = fopen(files[i], "r");
		if (curr == NULL)
		{
			fprintf(stderr, "File does not exist!");
			exit(EXIT_FAILURE);
		}
		while (fscanf(curr, "%s", urlName) != EOF)
		{
			for (int i = 0; i < count; i++)
			{
				if (strcmp(list[i], urlName) == 0)
				{
					exists = true;
				}
			}
			if (exists == false)
			{
				strcpy(list[count], urlName);
				count++;
			}
			exists = false;
		}

		fclose(curr);
	}
}

// Returns the number of unique pages
int initUrls(char *file[], int numFiles)
{
	int count = 0;
	char buff[MAXURLLEN];
	char **set = malloc(sizeof(char *) * MAXURL);
	if (set == NULL)
	{
		fprintf(stderr, "Ran out of memory!");
		return EXIT_FAILURE;
	}
	for (int i = 0; i < MAXURL; i++)
	{
		set[i] = malloc(sizeof(char *));
	}
	bool exists = false;
	for (int i = 1; i < numFiles; i++)
	{
		FILE *curr = fopen(file[i], "r");
		if (curr == NULL)
		{
			fprintf(stderr, "File does not exist!");
			return EXIT_FAILURE;
		}
		while (fscanf(curr, "%s", buff) != EOF)
		{
			for (int i = 0; i < count; i++)
			{
				if (strcmp(set[i], buff) == 0)
				{
					exists = true;
					break;
				}
			}
			if (!exists)
			{
				strcpy(set[count], buff);
				count++;
			}
			exists = false;
		}
		fclose(curr);
	}
	freeArrs(set, MAXURL);
	return count;
}