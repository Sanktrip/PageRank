#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Map.h"
#include "graph.h"

#define DEFAULT_CAPACITY 1

typedef struct adjNode *AdjList;
struct adjNode
{
	int v;
	AdjList next;
};
typedef struct urlnode *urlNode;

struct urlnode
{
	int outDegree;	  // the number of outlinks from the node
	int inDegree;	  // the number of links going into the node
	double weight;	  // the current weight of the node
	double oldWeight; // the weight of the node from the previous iteration
	double wIn;		  // the Op value in the Win formula
	double wOut;	  // the Op value in the Wout formula
	AdjList list;	  // the list of nodes that are outbound links from this node
};

struct pagerank
{
	int numPages; // number of pages in the graph
	int capacity; // the total capacity of pages
	char **urls;  // the id of a person is simply the index
	Map urlToId;  // maps names to ids
	urlNode *url; // adjacency lists, kept in increasing order
};

struct orderUrl
{
	char *s;	   // name of the page
	double weight; // weight of the page
	int outDegree; // number of outlinks of the page
};

static void increaseCapacity(pageRank pg);
static char *myStrdup(char *s);
static int urlToId(pageRank pg, char *url);

static AdjList adjListInsert(AdjList l, int v);
static AdjList newAdjNode(int v);
static bool inAdjList(AdjList l, int v);
static void freeAdjList(AdjList l);
static void sortByName(pageRank pg, struct orderUrl *orderUrl);
static void sortByWeight(pageRank pg, struct orderUrl *orderUrl);
void printWeights(pageRank pg);

pageRank pageRankNew(void)
{
	pageRank pg = malloc(sizeof(*pg));
	if (pg == NULL)
	{
		fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
	}

	pg->numPages = 0;
	pg->capacity = DEFAULT_CAPACITY;

	pg->url = calloc(pg->capacity, sizeof(struct urlnode));
	if (pg->url == NULL)
	{
		fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
	}
	pg->urls = calloc(pg->capacity, sizeof(char *));
	if (pg->urls == NULL)
	{
		fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
	}
	pg->urlToId = MapNew();
	return pg;
}

void pgFree(pageRank pg)
{
	for (int i = 0; i < pg->numPages; i++)
	{
		freeAdjList(pg->url[i]->list);
	}
	for (int j = 0; j < pg->capacity; j++)
	{
		free(pg->url[j]);
	}
	free(pg->url);
	MapFree(pg->urlToId);

	for (int i = 0; i < pg->numPages; i++)
	{
		free(pg->urls[i]);
	}
	free(pg->urls);

	free(pg);
}

bool pgAddLink(pageRank pg, char *name)
{
	if (pg->numPages == pg->capacity)
	{
		increaseCapacity(pg);
	}

	if (!MapContains(pg->urlToId, name))
	{
		int id = pg->numPages++;
		pg->urls[id] = myStrdup(name);
		MapSet(pg->urlToId, name, id);
		pg->url[id] = malloc(sizeof(struct urlnode));
		pg->url[id]->list = NULL;
		pg->url[id]->outDegree = 0.0;
		pg->url[id]->inDegree = 0.0;
		pg->url[id]->wIn = 0.0;
		pg->url[id]->wOut = 0.0;
		return true;
	}
	else
	{
		return false;
	}
}
static void increaseCapacity(pageRank pg)
{
	int newCapacity = pg->capacity * 2;

	pg->urls = realloc(pg->urls, newCapacity * sizeof(char *));
	if (pg->urls == NULL)
	{
		fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
	}
	for (int i = pg->capacity; i < newCapacity; i++)
	{
		pg->urls[i] = NULL;
	}

	pg->url = realloc(pg->url, newCapacity * sizeof(AdjList));
	if (pg->url == NULL)
	{
		fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
	}
	for (int i = pg->capacity; i < newCapacity; i++)
	{
		pg->url[i] = NULL;
	}

	pg->capacity = newCapacity;
}

bool pgLink(pageRank pg, char *url1, char *url2)
{
	int id1 = urlToId(pg, url1);
	int id2 = urlToId(pg, url2);
	if (id1 == id2)
	{
		return false;
	}

	if (!inAdjList(pg->url[id1]->list, id2))
	{
		pg->url[id1]->list = adjListInsert(pg->url[id1]->list, id2);
		pg->url[id1]->outDegree++;
		pg->url[id2]->inDegree++;
		return true;
	}
	else
	{
		return false;
	}
}

bool pgIsLinked(pageRank pg, char *url1, char *url2)
{
	int id1 = urlToId(pg, url1);
	int id2 = urlToId(pg, url2);
	return inAdjList(pg->url[id1]->list, id2);
}

bool adjNodeIndex(AdjList L, int num)
{
	while (L != NULL)
	{
		if (L->v == num)
		{
			return true;
		}
		L = L->next;
	}
	return false;
}

List inAdjUrlNodes(pageRank pg, int node)
{
	List l = ListNew();
	for (int i = 0; i < pg->numPages; i++)
	{
		if (inAdjList(pg->url[i]->list, node))
		{
			ListAppend(l, pg->urls[i]);
		}
	}
	return l;
}

void wOutCalc(pageRank pg)
{
	for (int i = 0; i < pg->numPages; i++)
	{
		double refPageSum = 0.0;
		for (AdjList curr = pg->url[i]->list; curr != NULL; curr = curr->next)
		{
			int index = curr->v;
			if (pg->url[index]->outDegree == 0)
			{
				refPageSum += 0.5;
			}
			else
			{
				refPageSum += pg->url[index]->outDegree;
			}
		}
		pg->url[i]->wOut = refPageSum;
	}
}
void wInCalc(pageRank pg)
{
	for (int i = 0; i < pg->numPages; i++)
	{
		double refPageSum = 0.0;
		for (AdjList curr = pg->url[i]->list; curr != NULL; curr = curr->next)
		{
			int index = curr->v;
			refPageSum += pg->url[index]->inDegree;
		}
		pg->url[i]->wIn = refPageSum;
	}
}
void rankCalculator(pageRank pg, double damping, double minDiff, int maxIt)
{
	double currDiff = 9999999999.0;
	double constant = (1.0 - damping) / pg->numPages;
	for (int i = 0; i < pg->numPages; i++)
	{
		pg->url[i]->weight = 1.0 / pg->numPages;
		pg->url[i]->oldWeight = 1.0 / pg->numPages;
	}
	for (int currIt = 0; currIt < maxIt && minDiff <= currDiff; currIt++)
	{
		for (int i = 0; i < pg->numPages; i++)
		{
			double currWeight = rawWeightingCalc(pg, i);
			currWeight *= damping;
			currWeight += constant;
			pg->url[i]->weight = currWeight;
		}
		currDiff = diffPR(pg);
		for (int j = 0; j < pg->numPages; j++)
		{
			pg->url[j]->oldWeight = pg->url[j]->weight;
		}
	}
}

double rawWeightingCalc(pageRank pg, int index)
{
	List l = inAdjUrlNodes(pg, index);
	double sum = 0.0;
	ListIterator it = ListItNew(l);
	double wOut;
	while (ListItHasNext(it))
	{
		char *file = ListItNext(it);
		int inIndex = urlToId(pg, file);
		if (pg->url[index]->outDegree == 0)
		{
			wOut = 0.5 / pg->url[inIndex]->wOut;
		}
		else
		{
			wOut = pg->url[index]->outDegree / pg->url[inIndex]->wOut;
		}
		double wIn = pg->url[index]->inDegree / pg->url[inIndex]->wIn;
		sum += pg->url[inIndex]->oldWeight * wOut * wIn;
	}
	ListFree(l);
	ListItFree(it);
	return sum;
}

double diffPR(pageRank pg)
{
	double currDiff = 0.0;
	for (int i = 0; i < pg->numPages; i++)
	{
		currDiff += fabs(pg->url[i]->weight - pg->url[i]->oldWeight);
	}
	return currDiff;
}

void orderUrls(pageRank pg)
{
	struct orderUrl orderUrl[pg->numPages];
	for (int i = 0; i < pg->numPages; i++)
	{
		orderUrl[i].s = pg->urls[i];
		orderUrl[i].weight = pg->url[i]->weight;
		orderUrl[i].outDegree = pg->url[i]->outDegree;
	}
	sortByName(pg, orderUrl);
	sortByWeight(pg, orderUrl);
	for (int k = 0; k < pg->numPages; k++)
	{
		printf("%s %d %.7lf\n", orderUrl[k].s, orderUrl[k].outDegree,
			   orderUrl[k].weight);
	}
}
////////////////////////////////////////////////////////////////////////
// Helper Functions

static char *myStrdup(char *s)
{
	char *copy = malloc((strlen(s) + 1) * sizeof(char));
	if (copy == NULL)
	{
		fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
	}
	return strcpy(copy, s);
}

// Converts a name to an ID. Raises an error if the name doesn't exist.
static int urlToId(pageRank pg, char *name)
{
	if (!MapContains(pg->urlToId, name))
	{
		fprintf(stderr, "error: url '%s' does not exist!\n", name);
		exit(EXIT_FAILURE);
	}
	return MapGet(pg->urlToId, name);
}

// Inserts the given value into the adjacency list if it is not there already.
static AdjList adjListInsert(AdjList l, int v)
{
	if (l == NULL || v < l->v)
	{
		AdjList new = newAdjNode(v);
		new->next = l;
		return new;
	}
	else if (v == l->v)
	{
		return l;
	}
	else
	{
		l->next = adjListInsert(l->next, v);
		return l;
	}
}

// Creates a new adjacency node.
static AdjList newAdjNode(int v)
{
	AdjList n = malloc(sizeof(*n));
	if (n == NULL)
	{
		fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
	}
	n->v = v;
	n->next = NULL;
	return n;
}

// Frees the given adjacency list.
static void freeAdjList(AdjList l)
{
	AdjList n = l;
	while (n != NULL)
	{
		AdjList temp = n;
		n = n->next;
		free(temp);
	}
}

// Checks whether a node already exists in an adjacency list.
// Returns true if it does exist, else it returns false.
static bool inAdjList(AdjList l, int v)
{
	for (AdjList n = l; n != NULL && n->v <= v; n = n->next)
	{
		if (n->v == v)
		{
			return true;
		}
	}
	return false;
}

// Sorts the given pages by alphabetical order using a stable bubblesort.
static void sortByName(pageRank pg, struct orderUrl *orderUrl)
{
	for (int i = 0; i < pg->numPages; i++)
	{
		for (int j = 0; j < pg->numPages - 1; j++)
		{
			if (strcmp(orderUrl[j].s, orderUrl[j + 1].s) > 0)
			{
				double weightTemp = orderUrl[j].weight;
				char *temp = orderUrl[j].s;
				int outTemp = orderUrl[j].outDegree;
				orderUrl[j].weight = orderUrl[j + 1].weight;
				orderUrl[j].s = orderUrl[j + 1].s;
				orderUrl[j].outDegree = orderUrl[j + 1].outDegree;
				orderUrl[j + 1].weight = weightTemp;
				orderUrl[j + 1].outDegree = outTemp;
				orderUrl[j + 1].s = temp;
			}
		}
	}
}

// Sorts the given pages by decreasing weight order using a stable bubblesort.
static void sortByWeight(pageRank pg, struct orderUrl *orderUrl)
{
	for (int i = 0; i < pg->numPages; i++)
	{
		for (int j = 0; j < pg->numPages - 1; j++)
		{
			if (orderUrl[j].weight < orderUrl[j + 1].weight)
			{
				double weightTemp = orderUrl[j].weight;
				char *temp = orderUrl[j].s;
				int outTemp = orderUrl[j].outDegree;
				orderUrl[j].weight = orderUrl[j + 1].weight;
				orderUrl[j].s = orderUrl[j + 1].s;
				orderUrl[j].outDegree = orderUrl[j + 1].outDegree;
				orderUrl[j + 1].weight = weightTemp;
				orderUrl[j + 1].outDegree = outTemp;
				orderUrl[j + 1].s = temp;
			}
		}
	}
}

// Prints the weights of the pages in the given pageRank graph.
void printWeights(pageRank pg)
{
	for (int i = 0; i < pg->numPages; i++)
	{
		printf("%s: %.7lf\n", pg->urls[i], pg->url[i]->weight);
	}
}