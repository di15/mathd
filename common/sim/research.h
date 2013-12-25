

#include <stdlib.h>
#include <stdio.h>
#include <string>

using namespace std;

enum TECHNOLOGY{GYROSTABILIZERS, LIGHTFIELDCAMERAS, NEURALNETWARE, OBJECTRECOGNITION, INTELMOTIONREC, PROPRIOCEPTION, BATTLECOMPUTERS, ANTHROTANKS, TECHNOLOGIES};

class Technology
{
public:
	char name[32];
	string desc;
	unsigned int graphic;
	int gwidth;
	int gheight;
};