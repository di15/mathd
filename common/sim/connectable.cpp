
#include <stdlib.h>
#include "connection.h"

int GetConnectionType(bool n, bool e, bool s, bool w)
{
	if(n && e && s && w)
		return NORTHEASTSOUTHWEST;
	else if(n && e && s && !w)
		return NORTHEASTSOUTH;
	else if(n && e && !s && w)
		return NORTHEASTWEST;
	else if(n && e && !s && !w)
		return NORTHEAST;
	else if(n && !e && s && w)
		return NORTHSOUTHWEST;
	else if(n && !e && s && !w)
		return NORTHSOUTH;
	else if(n && !e && !s && w)
		return NORTHWEST;
	else if(n && !e && !s && !w)
		return NORTH;
	else if(!n && e && s && w)
		return EASTSOUTHWEST;
	else if(!n && e && s && !w)
		return EASTSOUTH;
	else if(!n && e && !s && w)
		return EASTWEST;
	else if(!n && e && !s && !w)
		return EAST;
	else if(!n && !e && s && w)
		return SOUTHWEST;
	else if(!n && !e && s && !w)
		return SOUTH;
	else if(!n && !e && !s && w)
		return WEST;
	else
		return NOCONNECTION;

	/*
	int r = rand()%4;

	if(r == 0)
		return NORTH;
	else if(r == 1)
		return EAST;
	else if(r == 2)
		return SOUTH;
	else
		return WEST;*/
}