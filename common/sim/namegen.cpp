
#include "namegen.h"
#include "unit.h"

// http://www.dreamincode.net/forums/topic/127035-random-name-generator/
// http://www.dreamincode.net/forums/topic/27024-data-modeling-for-games-in-c-part-ii/

//Here I will create an array of prefixes to help generate names.
// I am banking on multiplication to ensure a large number of names
// by using 7 prefixes and 20 stems, and 16 suffixes I should be able to
// create about 7 * 20 * 16 = 2240 names out of 312 bytes of data (In my earlier
// example from the forum I used this code to generate male and female names,
// but here I combined them).
char NamePrefix[][5] = 
{
    "", //who said we need to add a prefix?
    "bel", //lets say that means "the good"
    "nar", //"The not so good as Bel"
    "xan", //"The evil"
    "bell", //"the good"
    "natr", //"the neutral/natral"
    "ev", //Man am I original
};
    char NameSuffix[][5] = 
{
    "", "us", "ix", "ox", "ith",
    "ath", "um", "ator", "or", "axia",
    "imus", "ais", "itur", "orex", "o",
    "y"
};
 
const char NameStems[][10] = 
{
    "adur", "aes", "anim", "apoll", "imac",
    "educ", "equis", "extr", "guius", "hann",
    "equi", "amora", "hum", "iace", "ille",
    "inept", "iuv", "obe", "ocul", "orbis"
};
 
//The return type is void because we use a pointer to the array holding
// the characters of the name.
void NameGen(char* PlayerName)
{
    PlayerName[0]=0; //initialize the string to "" (zero length string).
    //add the prefix...
    strcat(PlayerName, NamePrefix[(rand() % 7)]);
    //add the stem...
    strcat(PlayerName, NameStems[(rand() % 20)]);
    //add the suffix...
    strcat(PlayerName, NameSuffix[(rand() % 16)]);
    //Make the first letter capital...
    PlayerName[0]=toupper(PlayerName[0]);
    return;
}


void GenName(string& firstname, string& lastname, int parent)
{
	/*
	if(parent < 0)
	{
		char newlast[64];
		NameGen(newlast);
		lastname = newlast;
	}
	else
	{
		CUnit* malepar;
		CUnit* par;

		//lastname = g_unit[parent].lastname;

		par = &g_unit[parent];

		if(par->male)
			lastname = par->lastname;
		else
		{
			if(par->spouse >= 0)
			{
				malepar = &g_unit[par->spouse];
				lastname = malepar->lastname;
			}
			else
				lastname = par->lastname;
		}
	}*/
	
	char newfirst[64];
	NameGen(newfirst);
	firstname = newfirst;
}