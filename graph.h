// PageRank ADT

// !!! DO NOT MODIFY THIS FILE !!!

#ifndef PG_H
#define PG_H

#include "List.h"

typedef struct adjNode *AdjList;
typedef struct urlnode *urlNode;
typedef struct pagerank *pageRank;

////////////////////////////////////////////////////////////////////////

/**
 * Creates a new instance of a pageRank graph
 */
pageRank pageRankNew(void);

/**
 * Frees all memory allocated to the given pageRank instance
 */
void pgFree(pageRank pg);

/**
 * Creates a URL node for the given URL. Returns true if
 * successful, and false if a person with that name already exists.
 */
bool pgAddLink(pageRank pg, char *name);

/**
 * Links two pages. Assumes that they both have a page existing in the pageRank graph,
 * and  that they are not the page. Returns true if the operation
 * was successful, and false if they were already linked.
 */
bool pgLink(pageRank pg, char *url1, char *url2);

/**
 * Calculates the Op value in the Win formula for every node in the given pageRank graph.
 **/
void wInCalc(pageRank pg);

/**
 * Calculates the Op value in the Wout formula for every node in the given pageRank graph.
 **/
void wOutCalc(pageRank pg);

/**
 * The main function that iterates through weight calculations until the maxIteration threshold
 * is surpasses or when the minDiff exceeds the difference between the old and current weights.
 **/
void rankCalculator(pageRank pg, double damping, double minDiff, int maxIt);

/**
 * Calculates the weight for the given page index and returns the value.
 **/
double rawWeightingCalc(pageRank pg, int index);

/**
 * Iterates through all nodes and returns the sum of the weight differences between the current weight
 * and the weight from the iteration before.
 **/
double diffPR(pageRank pg);

/**
 * Orders the pages in the given pageRank graph by weight and then alphabetical order. Uses bubble sort twice
 * to order these pages and prints the result to the terminal.
 **/
void orderUrls(pageRank pg);
#endif
