

#ifndef GRAPH_H
#define GRAPH_H

#include "../platform.h"

class Graph
{
public:
	std::list<float> points;
	unsigned int startframe;
	unsigned int cycles;

	Graph()
	{
		startframe = 0;
		cycles = 0;
	}
};

#define GR_AVGSAT		0	//average personal satiety
#define GR_AVGFUN		1	//average personal funds
#define GR_TOTSAT		2	//total personal satiety
#define GR_TOTFUN		3	//total personal funds
#define GRAPHS			4

extern const char* GRAPHNAME[GRAPHS];

extern Graph g_graph[GRAPHS];

void Tally();
void FreeGraphs();
void DrawGraph(Graph* g, float left, float top, float right, float bottom);

#endif