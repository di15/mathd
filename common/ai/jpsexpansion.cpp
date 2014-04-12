
#include "pathnode.h"
#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sys/workthread.h"
#include "../sim/unit.h"
#include "../sim/unittype.h"
#include "../sim/building.h"
#include "../sim/buildingtype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../sim/road.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/sim.h"
#include "../phys/trace.h"
#include "binheap.h"
#include "jpspath.h"
#include "reconstructpath.h"
#include "pathdebug.h"
#include "jpsexpansion.h"
#include "pathjob.h"
#include "../utils.h"
#include "../debug.h"

// https://github.com/Yonaba/Jumper/blob/master/jumper/search/jps.lua

// Returns neighbours. The returned value is an array of __walkable__ nodes neighbouring a given `node`.
list<PathNode*> GetNeighbours(PathJob* pj, PathNode* node)
{
	list<PathNode*> neighbours;

	Vec2i pos = PathNodePos(pj->wt, node);

	for( int i=0; i<ARRSZ(straightoffsets); i++ )
	{
		PathNode* n = PathNodeAt(pj->wt, 
			pos.x + straightoffsets[i].x,
			pos.y + straightoffsets[i].y
			);

		Vec2i p = PathNodePos(pj->wt, n);

		if( n && Walkable(pj, p.x, p.y) )
		{
			neighbours.push_back(n);
		}
	}

	if( !1 ) { return neighbours; }	//if not allow diagonal

	for( int i=0; i<ARRSZ(diagonaloffsets); i++ )
	{
		PathNode* n = PathNodeAt(pj->wt, 
			pos.x + diagonaloffsets[i].x,
			pos.y + diagonaloffsets[i].y
			);

		Vec2i p = PathNodePos(pj->wt, n);

		bool tunnel = false;

		if( n && Walkable(pj, p.x, p.y) )
		{
			if( tunnel ) 
			{
				neighbours.push_back(n);
			}
			else
			{
				bool skip = false;
				PathNode* n1 = PathNodeAt(pj->wt, p.x+diagonaloffsets[i].x, p.y);
				PathNode* n2 = PathNodeAt(pj->wt, p.x, p.y+diagonaloffsets[i].y);
				Vec2i p1 = PathNodePos(pj->wt, n1);
				Vec2i p2 = PathNodePos(pj->wt, n2);

				if( (n1 && n2) && !Walkable(pj, p1.x, p1.y) && !Walkable(pj, p2.x, p2.y) ) 
				{
					skip = true;
				}
				if( !skip ) { neighbours.push_back(n); }
			}
		}
	}

	return neighbours;
}


/*
Looks for the neighbours of a given node.
Returns its natural neighbours plus forced neighbours when the given
node has no parent (generally occurs with the starting node).
Otherwise, based on the direction of move from the parent, returns
neighbours while pruning directions which will lead to symmetric paths.

In case diagonal moves are forbidden, when the given node has no
parent, we return straight neighbours (up, down, left and right).
Otherwise, we add left and right node (perpendicular to the direction
of move) in the neighbours list.
*/
list<PathNode*> FindNeighbours(PathJob* pj, PathNode* node)
{
	if( node->previous && node->previous != node ) 
	{
		list<PathNode*> neighbours;

		Vec2i pos = PathNodePos(pj->wt, node);

		// Node have a parent, we will prune some neighbours
		// Gets the direction of move
		Vec2i delta = pos - PathNodePos(pj->wt, node->previous);
		Vec2i prevpos = PathNodePos(pj->wt, node->previous);
		delta.x = (pos.x-prevpos.x)/max(abs(pos.x-prevpos.x),1);
		delta.y = (pos.y-prevpos.y)/max(abs(pos.y-prevpos.y),1);

		// Diagonal move case
		if( delta.x!=0 && delta.y!=0 ) 
		{
			bool walkY = false;
			bool walkX = false;

			// Natural neighbours
			if( Walkable(pj, pos.x, pos.y+delta.y) ) 
			{
				neighbours.push_back(PathNodeAt(pj->wt, pos.x, pos.y+delta.y));
				walkY = true;
			}
			if( Walkable(pj, pos.x+delta.x, pos.y) ) 
			{
				neighbours.push_back(PathNodeAt(pj->wt, pos.x+delta.x, pos.y));
				walkX = true;
			}
			
#if 0
			if( walkX || walkY ) neighbours.push_back(PathNodeAt(pj->wt, pos.x+delta.x, pos.y+delta.y));
#else
			// Denis edit - trip corners
			if( walkX && walkY ) neighbours.push_back(PathNodeAt(pj->wt, pos.x+delta.x, pos.y+delta.y));
#endif

			// Forced neighbours
#if 0
			if( !Walkable(pj, pos.x-delta.x, pos.y) && walkY ) neighbours.push_back(PathNodeAt(pj->wt, pos.x-delta.x, pos.y+delta.y));
			if( !Walkable(pj, pos.x, pos.y-delta.y) && walkX ) neighbours.push_back(PathNodeAt(pj->wt, pos.x+delta.x, pos.y-delta.y));
#elif 0
			// Denis edit - trip corners
			if( !Walkable(pj, pos.x-delta.x, pos.y) && walkY && walkX ) neighbours.push_back(PathNodeAt(pj->wt, pos.x-delta.x, pos.y+delta.y));
			if( !Walkable(pj, pos.x, pos.y-delta.y) && walkX && walkY ) neighbours.push_back(PathNodeAt(pj->wt, pos.x+delta.x, pos.y-delta.y));
#else
			// Denis edit - trip corners

#endif
		}
		else
		{
			// Move along Y-axis case
			if( delta.x==0 ) 
			{
				bool walkY = false;

				if( Walkable(pj, pos.x, pos.y+delta.y) ) 
				{
					neighbours.push_back( PathNodeAt(pj->wt, pos.x, pos.y+delta.y) );

#if 0
					// Forced neighbours are left and right ahead along Y
					if( !Walkable(pj, pos.x+1, pos.y) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x+1, pos.y+delta.y) );
					if( !Walkable(pj, pos.x-1, pos.y) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x-1, pos.y+delta.y) );
#elif 0
					//Denis edit - trip corners
					if( Walkable(pj, pos.x, pos.y+delta.y) )
					{
						if( !Walkable(pj, pos.x+1, pos.y-delta.y) && Walkable(pj, pos.x+1, pos.y) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x+1, pos.y+delta.y) );
						if( !Walkable(pj, pos.x-1, pos.y-delta.y) && Walkable(pj, pos.x-1, pos.y) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x-1, pos.y+delta.y) );
					}
#else
				}
#endif
				//Denis edit - trip corners
				if( !Walkable(pj, pos.x+1, pos.y-delta.y) )
				{
						neighbours.push_back( PathNodeAt(pj->wt, pos.x+1, pos.y) );

						if( Walkable(pj, pos.x, pos.y+delta.y) && Walkable(pj, pos.x+1, pos.y) )
							neighbours.push_back( PathNodeAt(pj->wt, pos.x+1, pos.y+delta.y) );
				}
				if( !Walkable(pj, pos.x-1, pos.y-delta.y) )
				{
						neighbours.push_back( PathNodeAt(pj->wt, pos.x-1, pos.y) );
						
						if( Walkable(pj, pos.x, pos.y+delta.y) && Walkable(pj, pos.x-1, pos.y) )
							neighbours.push_back( PathNodeAt(pj->wt, pos.x-1, pos.y+delta.y) );
				}

				// In case diagonal moves are forbidden : Needs to be optimized
				if( !1 ) //if not allow diagonal
				{
					if( Walkable(pj, pos.x+1, pos.y) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x+1, pos.y) );
					if( Walkable(pj, pos.x-1, pos.y) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x-1, pos.y) );
				}
			}
			else
			{
				// Move along X-axis case
				if( Walkable(pj, pos.x+delta.x, pos.y) ) 
				{
					neighbours.push_back( PathNodeAt(pj->wt, pos.x+delta.x, pos.y) );

					// Forced neighbours are up and down ahead along X
#if 0
					if( !Walkable(pj, pos.x, pos.y+1) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x+delta.x, pos.y+1) );
					if( !Walkable(pj, pos.x, pos.y-1) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x+delta.x, pos.y-1) );
#elseif 0
					//Denis edit - trip corners
					if( Walkable(pj, pos.x+delta.x, pos.y) )
					{
						if( !Walkable(pj, pos.x-delta.x, pos.y+1) && Walkable(pj, pos.x, pos.y+1) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x+delta.x, pos.y+1) );
						if( !Walkable(pj, pos.x-delta.x, pos.y-1) && Walkable(pj, pos.x, pos.y-1) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x+delta.x, pos.y-1) );
					}
#else
				}
#endif
				
				//Denis edit - trip corners
				if( !Walkable(pj, pos.x-delta.x, pos.y+1) )
				{
					neighbours.push_back( PathNodeAt(pj->wt, pos.x, pos.y+1) );

					if( Walkable(pj, pos.x+delta.x, pos.y) && Walkable(pj, pos.x, pos.y+1) )
						neighbours.push_back( PathNodeAt(pj->wt, pos.x+delta.x, pos.y+1) );
				}
				if( !Walkable(pj, pos.x-delta.x, pos.y-1) )
				{
					neighbours.push_back( PathNodeAt(pj->wt, pos.x, pos.y-1) );

					if( Walkable(pj, pos.x+delta.x, pos.y) && Walkable(pj, pos.x, pos.y-1) )
						neighbours.push_back( PathNodeAt(pj->wt, pos.x+delta.x, pos.y-1) );
				}

				// : In case diagonal moves are forbidden
				if( !1 ) //if no diagonal
				{
					if( Walkable(pj, pos.x, pos.y+1) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x, pos.y+1) );
					if( Walkable(pj, pos.x, pos.y-1) ) neighbours.push_back( PathNodeAt(pj->wt, pos.x, pos.y-1) );
				}
			}
		}

		return neighbours;
	}

	// Node do not have parent, we return all neighbouring nodes
	return GetNeighbours(pj, node);
}


/*
Searches for a jump point (or a turning point) in a specific direction.
This is a generic translation of the algorithm 2 in the paper:
http://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf
The current expanded node is a jump point if( near a forced node

In case diagonal moves are forbidden, when lateral nodes (perpendicular to
the direction of moves are walkable, we force them to be turning points in other
to perform a straight move.
*/

PathNode* Jump(PathJob* pj, PathNode* node, PathNode* parent)
{
	if(!node) return NULL;

	Vec2i pos = PathNodePos(pj->wt, node);
	Vec2i delta = pos - PathNodePos(pj->wt, parent);

	// If the node to be examined is unwalkable, return nil
	if( !Walkable(pj, pos.x, pos.y) ) { return NULL; }

	// If the node to be examined is the endNode, return this node
	if( AtGoal(pj, node) ) { return node; }

	// Diagonal search case
	if( delta.x!=0 && delta.y!=0 ) 
	{
#if 0
		// Current node is a jump point if one of his leftside/rightside neighbours ahead is forced
		if( (Walkable(pj, pos.x-delta.x, pos.y+delta.y) && !Walkable(pj, pos.x-delta.x, pos.y)) ||
			(Walkable(pj, pos.x+delta.x, pos.y-delta.y) && !Walkable(pj, pos.x, pos.y-delta.y)) ) 
		{
			return node;
		}
#endif
	}
	else 
	{
		// Search along X-axis case
		if( delta.x!=0 ) 
		{
			if( 1 )		//allow diagonal?
			{
#if 0
				// Current node is a jump point if one of his upside/downside neighbours is forced
				if( (Walkable(pj, pos.x+delta.x, pos.y+1) && !Walkable(pj, pos.x, pos.y+1)) ||
					(Walkable(pj, pos.x+delta.x, pos.y-1) && !Walkable(pj, pos.x, pos.y-1)) ) 
#else
				//Denis edit - corner fix
				if( (Walkable(pj, pos.x, pos.y+1) && !Walkable(pj, pos.x-delta.x, pos.y+1)) ||
					(Walkable(pj, pos.x, pos.y-1) && !Walkable(pj, pos.x-delta.x, pos.y-1)) ) 
#endif
				{
					return node;
				}
			}
			else 
			{
				// : in case diagonal moves are forbidden
				if( Walkable(pj, pos.x+1, pos.y) || Walkable(pj, pos.x-1, pos.y) ) { return node; }
			}
		}
		else 
		{
			// Search along Y-axis case
			// Current node is a jump point if one of his leftside/rightside neighbours is forced
			if( 1 ) //allow diagonal?
			{
#if 0
				if( (Walkable(pj, pos.x+1, pos.y+delta.y) && !Walkable(pj, pos.x+1, pos.y)) ||
					(Walkable(pj, pos.x-1, pos.y+delta.y) && !Walkable(pj, pos.x-1, pos.y)) ) 
#else
				if( (Walkable(pj, pos.x+1, pos.y) && !Walkable(pj, pos.x+1, pos.y-delta.y)) ||
					(Walkable(pj, pos.x-1, pos.y) && !Walkable(pj, pos.x-1, pos.y-delta.y)) ) 
#endif
				{
					return node;
				}
			}
			else 
			{
				// : in case diagonal moves are forbidden
				if( Walkable(pj, pos.x, pos.y+1) || Walkable(pj, pos.x, pos.y-1) ) { return node; }
			}
		}
	}

	// Recursive horizontal/vertical search
	if( delta.x!=0 && delta.y!=0 ) 
	{
		if( Jump(pj, PathNodeAt(pj->wt, pos.x+delta.x, pos.y), node) ) { return node; }
		if( Jump(pj, PathNodeAt(pj->wt, pos.x, pos.y+delta.y), node) ) { return node; }
	}

	// Recursive diagonal search
	if( 1 ) //allow diagonal?
	{
#if 0
		if( Walkable(pj, pos.x+delta.x, pos.y) || Walkable(pj, pos.x, pos.y+delta.y) ) 
		{
			return Jump(pj, PathNodeAt(pj->wt, pos.x+delta.x, pos.y+delta.y), node);
		}
#else
		// Denis edit - tripping on corners
		if( Walkable(pj, pos.x+delta.x, pos.y) && Walkable(pj, pos.x, pos.y+delta.y) ) 
		{
			return Jump(pj, PathNodeAt(pj->wt, pos.x+delta.x, pos.y+delta.y), node);
		}
#endif
	}

	return NULL;
}

/*
Searches for successors of a given node in the direction of each of its neighbours.
This is a generic translation of the algorithm 1 in the paper:
http://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf

Also, we notice that processing neighbours in a reverse order producing a natural
looking path, as the pathfinder tends to keep heading in the same direction.
In case a jump point was found, and this node happened to be diagonal to the
node currently expanded in a straight mode search, we skip this jump point.
*/
void IdentifySuccessors_JPS(PathJob* pj, PathNode* node)
{
	PathNode* endnode = NULL;
	Vec2i pos = PathNodePos(pj->wt, node);

	// Gets the valid neighbours of the given node
	// Looks for a jump point in the direction of each neighbour
	list<PathNode*> neighbours = FindNeighbours(pj, node);

	for( auto niter = neighbours.rbegin(); niter != neighbours.rend(); niter++ ) 
	{
		bool skip = false;
		PathNode* neighbour = *niter;

		Vec2i npos = PathNodePos(pj->wt, neighbour);

		PathNode* jumpnode = Jump(pj, neighbour, node);

		Vec2i jumppos = PathNodePos(pj->wt, jumpnode);

		// : in case a diagonal jump point was found in straight mode, skip it.
		if( jumpnode && !1 ) // and not allow diagonal
		{
			if( jumppos.x != pos.x && jumppos.y != pos.y ) { skip = true; }
		}

		// Performs regular A-star on a set of jump points
		if( jumpnode && !skip )
		{
			int jumpi = jumpnode - pj->wt->pathnode;

			// Update the jump node and move it in the closed list if it wasn't there
			if( !jumpnode->closed )
			{			
				int extraD = Magnitude(jumppos - pos);
				int newD = node->totalD + extraD;

				if( !jumpnode->opened || newD < jumpnode->totalD )
				{
					pj->wt->toclear.push_back(jumpnode); // Records this node to reset its properties later.
					jumpnode->totalD = newD;
					int H = Manhattan( jumppos - Vec2i(pj->ngoalx, pj->ngoalz) );
					jumpnode->F = jumpnode->totalD + H;
					jumpnode->previous = node;

					if( !jumpnode->opened ) 
					{
						pj->wt->openlist.insert(jumpnode);
						jumpnode->opened = true;
					}
					else
					{
						pj->wt->openlist.heapify(jumpnode);
					}
				}					
			}
		}
	}
}