
#include <vector>

#include "3dmath.h"
#include "gui.h"
#include "resource.h"

using namespace std;

class Line
{
public:
	Vec3f pos[2];
};

class BGraph
{
public:
	bool resrelevant[RESOURCES];
	bool resrelevant2[RESOURCES];
	CWidget resnamecolor[RESOURCES];
	vector<Vec3f> rescurve[RESOURCES];
	vector<Vec3f> rescurve2[RESOURCES];
	vector<Line> bglines;
	vector<CWidget> periodicmarks;

	void reset();
	BGraph();
	void draw();
};

extern BGraph g_bgraph;

void DrawBGraph();
void FillBGraph();