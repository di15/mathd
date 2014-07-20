

#include "utility.h"

// food/housing/physical res utility evaluation
int PhUtil(int price, int cmdist)
{
	return 100000000 / price / cmdist;
}

// electricity utility evaluation
int GlUtil(int price)
{
	return 100000000 / price;
}

// inverse phys utility - solve for distance based on utility and price
int InvPhUtilD(int util, int price)
{
	
	//util = 100000000 / price / cmdist;
	//util / 100000000 = 1 / price / cmdist;
	//util * price / 100000000 = 1 / cmdist;
	//100000000 / (util * price) = cmdist;

	return 100000000 / (util * price);
}

// inverse phys utility - solve for price based on utility and distance
int InvPhUtilP(int util, int cmdist)
{
	return 100000000 / (util * cmdist);
}

int InvGlUtilP(int util)
{
	return 100000000 / util;
}