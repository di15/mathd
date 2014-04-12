

#ifndef PATHBLOCKED_H
#define PATHBLOCKED_H

bool PathBlocked(int wthread, int utype, int umode, int cmstartx, int cmstartz, int target, int targetx, int targetz, 
			  Unit* thisu, Unit* ignoreu, Building* ignoreb, 
			  int cmgoalx, int cmgoalz, int cmgoalminx, int cmgoalminz, int cmgoalmaxx, int cmgoalmaxz,
			  int maxsearch);

#endif