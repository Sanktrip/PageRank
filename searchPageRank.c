#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXURL 100
#define MAXLINE 1000
struct url
{
	char *s;
	int hits;
	double weight;
};

int findNumPages(void);
void initPages(struct url *allUrls, int numPages);
void checkUrlMatch(char *token, struct url *allUrls, int numPages);
void findMatches(int numPages, struct url *allUrls, char *argv[], int argc);
void sortPages(struct url *allUrls, int numPages);
void printResults(struct url *allUrls, int numPages);
void freeAllUrls(struct url *allUrls, int numPages);

int main(int argc, char *argv[])
{
	int numPages = findNumPages();
	struct url allUrls[numPages];
	initPages(allUrls, numPages);
	findMatches(numPages, allUrls, argv, argc);
	sortPages(allUrls, numPages);
	printResults(allUrls, numPages);
	freeAllUrls(allUrls, numPages);
	return 0;
}

/**
 * Free's the memory containg the name of the url's in struct url.
 **/
void freeAllUrls(struct url *allUrls, int numPages)
{
	for (int i = 0; i < numPages; i++)
	{
		free(allUrls[i].s);
	}
}

/**
 * Prints the pages and their weights and number of edges to the terminal.
 **/
void printResults(struct url *allUrls, int numPages)
{
	int totalPages = (numPages > 30) ? 30 : numPages;
	for (int j = 0; j < totalPages; j++)
	{
		if (allUrls[j].hits > 0)
		{
			printf("%s\n", allUrls[j].s);
		}
	}
}

/**
 * Sorts the pages in struct Url by number of hits.
 **/
void sortPages(struct url *allUrls, int numPages)
{
	for (int i = 0; i < numPages; i++)
	{
		for (int j = 0; j < numPages - 1; j++)
		{
			if (allUrls[j].hits < allUrls[j + 1].hits)
			{
				int temp = allUrls[j].hits;
				double tempWeight = allUrls[j].weight;
				char *tempName = allUrls[j].s;
				allUrls[j].hits = allUrls[j + 1].hits;
				allUrls[j].weight = allUrls[j + 1].weight;
				allUrls[j].s = allUrls[j + 1].s;
				allUrls[j + 1].hits = temp;
				allUrls[j + 1].weight = tempWeight;
				allUrls[j + 1].s = tempName;
			}
		}
	}
}

/**
 * Reads the pageRankList.txt file and returns the number of pages
 * in the file.
 **/
int findNumPages(void)
{
	char pageName[MAXURL];
	FILE *pages = fopen("pageRankList.txt", "r");
	if (pages == NULL)
	{
		fprintf(stderr, "File does not exist!");
		return EXIT_FAILURE;
	}
	int numPages = 0;
	int outD;
	double weight;
	while (fscanf(pages, "%s %d %lf", pageName, &outD, &weight) == 3)
	{
		numPages++;
	}
	fclose(pages);
	return numPages;
}

/**
 * Reads the pageRankList.txt file and initialised the pages and their
 * properties into a struct url.
 **/
void initPages(struct url *allUrls, int numPages)
{
	char pageName[MAXURL];
	FILE *pages = fopen("pageRankList.txt", "r");
	if (pages == NULL)
	{
		fprintf(stderr, "File does not exist!");
		exit(EXIT_FAILURE);
	}
	int outD;
	fseek(pages, 0, SEEK_SET);
	for (int i = 0; i < numPages; i++)
	{
		fscanf(pages, "%s %d %lf", pageName, &outD, &allUrls[i].weight);
		allUrls[i].hits = 0;
		allUrls[i].s = strdup(pageName);
	}
	fclose(pages);
}

/**
 * Checks which Urls contain the searched string,
 * if a URL does contain the string increases the number of hits
 * by one.
 **/
void checkUrlMatch(char *token, struct url *allUrls, int numPages)
{
	while (token != NULL)
	{
		for (int j = 0; j < numPages; j++)
		{
			if (strcmp(token, allUrls[j].s) == 0)
			{
				allUrls[j].hits++;
			}
		}
		token = strtok(NULL, " \n");
	}
}

/**
 * Finds tthe corresponding line that contains the the searched
 * strings.
 **/
void findMatches(int numPages, struct url *allUrls, char *argv[], int argc)
{
	FILE *inverted = fopen("invertedIndex.txt", "r");
	if (inverted == NULL)
	{
		fprintf(stderr, "File does not exist!");
		exit(EXIT_FAILURE);
	}
	char line[MAXLINE];
	char *token;
	for (int i = 1; i < argc; i++)
	{
		fseek(inverted, 0, SEEK_SET);
		while (fgets(line, MAXLINE, inverted) != NULL)
		{
			token = strtok(line, " ");
			if (strcmp(token, argv[i]) == 0)
			{
				token = strtok(NULL, " \n");
				checkUrlMatch(token, allUrls, numPages);
			}
		}
	}
	fclose(inverted);
}
