#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"

#define MAXURL 100

pageRank initPages(void);

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s dampingFactor diffPR maxIterations\n",
				argv[0]);
		return EXIT_FAILURE;
	}
	double damping = atof(argv[1]);
	double minDiff = atof(argv[2]);
	int maxIt = atoi(argv[3]);
	pageRank pg = initPages();
	wInCalc(pg);
	wOutCalc(pg);
	rankCalculator(pg, damping, minDiff, maxIt);
	orderUrls(pg);
	pgFree(pg);
}

// Inititalises the pages from the given file into a pageRank graph.
pageRank initPages(void)
{
	pageRank pg = pageRankNew();
	FILE *collection = fopen("collection.txt", "r");
	if (collection == NULL)
	{
		fprintf(stderr, "File does not exist!");
		exit(EXIT_FAILURE);
	}
	char file[MAXURL];
	char fileExt[MAXURL + 4];
	char outLink[MAXURL];
	char *extension = ".txt";
	while (fscanf(collection, "%s", file) != EOF)
	{
		pgAddLink(pg, file);
	}
	fseek(collection, 0, SEEK_SET);
	while (fscanf(collection, "%s", file) != EOF)
	{
		strcpy(fileExt, file);
		strcat(fileExt, extension);

		FILE *urlPage = fopen(fileExt, "r");
		if (urlPage == NULL)
		{
			fprintf(stderr, "File does not exist!");
			exit(EXIT_FAILURE);
		}
		while (strcmp(outLink, "Section-1") != 0)
		{
			fscanf(urlPage, "%s", outLink);
		}
		fscanf(urlPage, "%s", outLink);
		while (strcmp(outLink, "#end") != 0)
		{
			pgLink(pg, file, outLink);
			fscanf(urlPage, "%s", outLink);
		}
		fclose(urlPage);
	}
	fclose(collection);
	return pg;
}
