
#include "../platform.h"
#include "brush.h"
#include "plane.h"
#include "3dmath.h"
#include "line.h"
#include "polygon.h"
#include "../utils.h"
#include "3dmath.h"
#include "../sim/building.h"

Brush::Brush(const Brush& original)
{
	/*
	int m_nsides;
	BrushSide* m_sides;
	*/

	m_nsides = original.m_nsides;
	for(int i=0; i<m_nsides; i++)
		m_sides[i] = original.m_sides[i];

	m_texture = original.m_texture;
}

EdBrush& EdBrush::operator=(const EdBrush& original)
{
	/*
	int m_nsides;
	EdBrushSide* m_sides;
	*/

	//g_log<<"copy edbrush nsides="<<original.m_nsides<<endl;
	//g_log.flush();

	m_nsides = original.m_nsides;
	m_sides = new EdBrushSide[m_nsides];
	for(int i=0; i<m_nsides; i++)
		m_sides[i] = original.m_sides[i];
	m_nsharedv = original.m_nsharedv;
	m_sharedv = new Vec3f[m_nsharedv];
	for(int i=0; i<m_nsharedv; i++)
		m_sharedv[i] = original.m_sharedv[i];

	m_texture = original.m_texture;

	return *this;
}

EdBrush::EdBrush(const EdBrush& original)
{
	*this = original;
}

Brush::Brush()
{
	m_sides = NULL;
	m_nsides = 0;
	m_texture = 0;
}

Brush::~Brush()
{
	if(m_sides)
	{
		delete [] m_sides;
		m_sides = NULL;
	}
	m_nsides = 0;
}

EdBrush::EdBrush()
{
	m_sides = NULL;
	m_nsides = 0;
	m_sharedv = NULL;
	m_nsharedv = 0;
	m_texture = 0;
}

EdBrush::~EdBrush()
{
	if(m_sides)
	{
		delete [] m_sides;
		m_sides = NULL;
	}

	if(m_sharedv)
	{
		delete [] m_sharedv;
		m_sharedv = NULL;
	}

	m_nsharedv = 0;
	m_nsides = 0;
}

Vec3f Brush::traceray(Vec3f line[])
{
	Vec3f intersection = line[1];

	return intersection;
}

//#define SELECT_DEBUG

#ifdef SELECT_DEBUG
EdBrush* g_debugb = NULL;
#endif

Vec3f EdBrush::traceray(Vec3f line[])
{
	float startRatio = -1.0f;
	float endRatio = 1.0f;
	Vec3f intersection = line[1];

	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];

		float startD = Dot(line[0], s->m_plane.m_normal) + s->m_plane.m_d;
		float endD = Dot(line[1], s->m_plane.m_normal) + s->m_plane.m_d;

#ifdef SELECT_DEBUG
		if(g_debugb == this)
		{
			g_log<<"side "<<i<<endl;
			g_log<<"startD="<<startD<<", endD="<<endD<<endl;
			g_log.flush();
		}

		if(g_debugb == this)
		{
			g_log<<"traceray plane=("<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<"d="<<s->m_plane.m_d<<") startD="<<startD<<" endD="<<endD<<endl;
			g_log.flush();
		}
#endif

		if(startD > 0 && endD > 0)
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"startD > 0 && endD > 0"<<endl;
				g_log.flush();
			}
#endif

			return line[1];
		}

		if(startD <= 0 && endD <= 0)
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"startD <= 0 && endD <= 0"<<endl;
				g_log.flush();
			}
#endif

			continue;
		}

		if(startD > endD)
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"startD > endD"<<endl;
				g_log.flush();
			}
#endif

			// This gets a ratio from our starting point to the approximate collision spot
			float ratio1 = (startD - EPSILON) / (startD - endD);
			
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"ratio1 ="<<ratio1<<endl;
				g_log.flush();
			}
#endif

			if(ratio1 > startRatio)
			{
				startRatio = ratio1;
				
#ifdef SELECT_DEBUG
				if(g_debugb == this)
				{
					g_log<<"ratio1 > startRatio == "<<startRatio<<endl;
					g_log.flush();
				}
#endif
			}
		}
		else
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"else startD <= endD"<<endl;
				g_log.flush();
			}
#endif

			float ratio = (startD + EPSILON) / (startD - endD);
			
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"ratio ="<<ratio<<endl;
				g_log.flush();
			}
#endif

			if(ratio < endRatio)
			{
				endRatio = ratio;
				
#ifdef SELECT_DEBUG
				if(g_debugb == this)
				{
					g_log<<"ratio < endRatio == "<<endRatio<<endl;
					g_log.flush();
				}
#endif
			}
		}
	}

	if(startRatio < endRatio)
	{
#ifdef SELECT_DEBUG
		if(g_debugb == this)
		{
			g_log<<"startRatio ("<<startRatio<<") < endRatio ("<<endRatio<<")"<<endl;
			g_log.flush();
		}
#endif

		if(startRatio > -1)
		{
#ifdef SELECT_DEBUG
				if(g_debugb == this)
				{
					g_log<<"startRatio > -1"<<endl;
					g_log.flush();
				}
#endif

			if(startRatio < 0)
				startRatio = 0;
			
#ifdef SELECT_DEBUG
			if(g_debugb == NULL)
				g_debugb = this;
#endif

			return line[0] + (line[1]-line[0]) * startRatio;
		}
	}

	return line[1];
}

void EdBrush::prunev(bool* invalidv)
{
	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];

		for(int j=0; j<m_nsharedv; j++)
		{
			if(!PointBehindPlane(m_sharedv[j], s->m_plane))
				invalidv[j] = true;
		}
	}
}

void EdBrush::moveto(Vec3f newp)
{
	Vec3f currp;

	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];
		currp = currp + s->m_centroid;
	}

	currp = currp / (float)m_nsides;

	Vec3f delta = newp - currp;

	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];

		Vec3f pop = PointOnPlane(s->m_plane);
		pop = pop + delta;
		s->m_plane.m_d = PlaneDistance(s->m_plane.m_normal, pop);

		for(int j=0; j<2; j++)
		{
			pop = PointOnPlane(s->m_tceq[j]);
			pop = pop + delta;
			s->m_tceq[j].m_d = PlaneDistance(s->m_tceq[j].m_normal, pop);
		}
	}

	collapse();
	remaptex();
}

void EdBrush::move(Vec3f move)
{
	list<float> oldus;
	list<float> oldvs;

	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];

		//Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
		float oldu = s->m_centroid.x*s->m_tceq[0].m_normal.x + s->m_centroid.y*s->m_tceq[0].m_normal.y + s->m_centroid.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
		float oldv = s->m_centroid.x*s->m_tceq[1].m_normal.x + s->m_centroid.y*s->m_tceq[1].m_normal.y + s->m_centroid.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
		//Vec3f axis = s->m_plane.m_normal;
		//float radians = DEGTORAD(degrees);
		//s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
		//s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
				
		oldus.push_back(oldu);
		oldvs.push_back(oldv);

		Vec3f pop = PointOnPlane(s->m_plane);
		pop = pop + move;
		s->m_plane.m_d = PlaneDistance(s->m_plane.m_normal, pop);

	}
			
	collapse();
			
	auto oldu = oldus.begin();
	auto oldv = oldvs.begin();

	for(int i=0; i<m_nsides; i++, oldu++, oldv++)
	{
		EdBrushSide* s = &m_sides[i];

		//Vec3f newsharedv = b->m_sharedv[ s->m_vindices[0] ];
		float newu = s->m_centroid.x*s->m_tceq[0].m_normal.x + s->m_centroid.y*s->m_tceq[0].m_normal.y + s->m_centroid.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
		float newv = s->m_centroid.x*s->m_tceq[1].m_normal.x + s->m_centroid.y*s->m_tceq[1].m_normal.y + s->m_centroid.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
		float changeu = newu - *oldu;
		float changev = newv - *oldv;
		s->m_tceq[0].m_d -= changeu;
		s->m_tceq[1].m_d -= changev;
	}
			
	remaptex();
}

void EdBrush::add(EdBrushSide b)
{
	EdBrushSide* newsides = new EdBrushSide[m_nsides + 1];
	
	if(m_nsides > 0)
	{
		//memcpy(newsides, m_sides, sizeof(EdBrushSide)*m_nsides);
		for(int i=0; i<m_nsides; i++)
			newsides[i] = m_sides[i];
		delete [] m_sides;
	}
	
	newsides[m_nsides] = b;
	m_sides = newsides;
	m_nsides ++;
}

//#define REMOVESIDE_DEBUG

void EdBrush::removeside(int i)
{
	EdBrushSide* newsides = new EdBrushSide[m_nsides-1];

	memcpy(&newsides[0], &m_sides[0], sizeof(EdBrushSide)*i);
	memcpy(&newsides[i], &m_sides[i+1], sizeof(EdBrushSide)*(m_nsides-i-1));

	m_nsides --;
	m_sides = newsides;

#ifdef REMOVESIDE_DEBUG
	g_log<<"remove "<<i<<endl;
	g_log.flush();
#endif
}

//#define DEBUG_COLLAPSE

//collapse geometry into triangles
void EdBrush::collapse()
{
	int oldnsharedv = m_nsharedv;

#ifdef DEBUG_COLLAPSE
	g_log<<"==================collapse this===================="<<endl;
	g_log.flush();
	
	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];
		Vec3f n = s->m_plane.m_normal;
		float d = s->m_plane.m_d;
		g_log<<"\t side["<<i<<"] plane=("<<n.x<<","<<n.y<<","<<n.z<<"),"<<d<<endl;
	}
#endif

	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];
		if(s->m_tris)
		{
			delete [] s->m_tris;
			s->m_tris = NULL;
		}
		s->m_ntris = 0;

		if(s->m_vindices)
		{
			delete [] s->m_vindices;
			s->m_vindices = NULL;
		}
	}

	if(m_sharedv)
	{
		delete [] m_sharedv;
		m_sharedv = NULL;
	}
	m_nsharedv = 0;

	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* a = &m_sides[i];
		for(int j=i+1; j<m_nsides; j++)
		{
			EdBrushSide* b = &m_sides[j];
			if(Close(a->m_plane, b->m_plane))
			{
				
#ifdef DEBUG_COLLAPSE
				g_log<<"planes["<<i<<","<<j<<"] ("<<a->m_plane.m_normal.x<<","<<a->m_plane.m_normal.y<<","<<a->m_plane.m_normal.z<<"),"<<a->m_plane.m_d<<" and ("<<b->m_plane.m_normal.x<<","<<b->m_plane.m_normal.y<<","<<b->m_plane.m_normal.z<<"),"<<b->m_plane.m_d<<endl;
				g_log.flush();
#endif
				removeside(j);
				j--;
			}
		}
	}

	
#ifdef DEBUG_COLLAPSE
	g_log<<"tag6. m_nsides="<<m_nsides<<endl;
	g_log.flush();
#endif
	
	list<Line>* sideedges = new list<Line>[m_nsides];	// a line along a plane intersecting two other planes. both vertices form the edge of a polygon.

	for(int i=0; i<m_nsides; i++)
	{
		//for(int j=i+1; j<m_nsides; j++)
		for(int j=0; j<m_nsides; j++)
		{
			if(j == i)
				continue;

			for(int k=0; k<m_nsides; k++)
			{
				if(k == i || k == j)
					continue;
				
				//for(int l=0; l<m_nsides; l++)
				for(int l=k+1; l<m_nsides; l++)
				{
					if(l == i || l == j || l == k)
						continue;

					EdBrushSide* a = &m_sides[i];
					EdBrushSide* b = &m_sides[j];
					EdBrushSide* c = &m_sides[k];
					EdBrushSide* d = &m_sides[l];

					//if(a->m_plane == b->m_plane)
					//if(Close(a->m_plane, b->m_plane))
					//{
					//	g_log<<"close "<<i<<","<<j<<endl;
					//	g_log.flush();
					//	continue;
					//}

					//if(Close(a->m_plane, c->m_plane))
					//{
					//	g_log<<"close "<<i<<","<<k<<endl;
					//	g_log.flush();
					//	continue;
					//}
				
#ifdef DEBUG_COLLAPSE
					g_log<<"--------new side"<<i<<" edge--------"<<endl;
					g_log.flush();
#endif

					// http://devmaster.net/forums/topic/8676-2-plane-intersection/page__view__findpost__p__47568
					Vec3f linedir = Cross(a->m_plane.m_normal, b->m_plane.m_normal);
					Vec3f pointonplanea = PointOnPlane(a->m_plane);	// arbitrary point on plane A
					Vec3f v = Cross(linedir, a->m_plane.m_normal); // direction toward plane B, parallel to plane A
					Vec3f l0;
					
#ifdef DEBUG_COLLAPSE
					g_log<<"collapse ("<
				
					g_log<<"linedir="<<linedir.x<<","<<linedir.y<<","<<linedir.z<<endl;
					g_log<<"pointonplanea="<<pointonplanea.x<<","<<pointonplanea.y<<","<<pointonplanea.z<<endl;
					g_log.flush();
#endif

					if(!Intersection(pointonplanea, v, b->m_plane, l0))
						continue;
					
#ifdef DEBUG_COLLAPSE
					g_log<<"l0="<<l0.x<<","<<l0.y<<","<<l0.z<<endl;
					g_log.flush();
					
					g_log<<"\tcollapse ("<<i<<","<<j<<","<<k<<","<<l<<") 2"<<endl;
					g_log.flush();
#endif
					Vec3f lineorigin;

					if(!Intersection(l0, linedir, c->m_plane, lineorigin))
						continue;
					
#ifdef DEBUG_COLLAPSE
					g_log<<"\t\tcollapse ("<<i<<","<<j<<","<<k<<","<<l<<") 3"<<endl;
					g_log.flush();
#endif
					Vec3f lineend;

					if(!Intersection(l0, linedir, d->m_plane, lineend))
						continue;
					
#ifdef DEBUG_COLLAPSE
					g_log<<"\t\t\tcollapse ("<<i<<","<<j<<","<<k<<","<<l<<") 4"<<endl;
					g_log.flush();
#endif

					if(Close(lineorigin, lineend))
					//if(lineorigin == lineend)
					{
#ifdef DEBUG_COLLAPSE
						g_log<<"close side["<<i<<"] newedge ("<<lineorigin.x<<","<<lineorigin.y<<","<<lineorigin.z<<")->("<<lineend.x<<","<<lineend.y<<","<<lineend.z<<")"<<endl;
#endif
						continue;
					}

					Line edge(lineorigin, lineend);
					sideedges[i].push_back(edge);
					
#ifdef DEBUG_COLLAPSE
					g_log<<"\t\t\t\tfinal=("<<lineorigin.x<<","<<lineorigin.y<<","<<lineorigin.z<<") final=("<<lineend.x<<","<<lineend.y<<","<<lineend.z<<")"<<endl;
					g_log.flush();
#endif
				}
			}
		}
		
#ifdef DEBUG_COLLAPSE
		g_log<<"side"<<i<<" remaining edges"<<endl;
		for(auto k=sideedges[i].begin(); k!=sideedges[i].end(); k++)
		{
			Line l = *k;
			g_log<<"("<<l.m_vertex[0].x<<","<<l.m_vertex[0].y<<","<<l.m_vertex[0].z<<")->("<<l.m_vertex[1].x<<","<<l.m_vertex[1].y<<","<<l.m_vertex[1].z<<")"<<endl;
		}
#endif
	}

	
#ifdef DEBUG_COLLAPSE
	g_log<<"tag7. m_nsides="<<m_nsides<<endl;
	g_log.flush();
#endif
			
	//remove side edges that have any vertex outside (and not on) of at least one other plane
	for(int i=0; i<m_nsides; i++)
	{
		for(int j=0; j<m_nsides; j++)
		{
			if(i == j)
				continue;

			EdBrushSide* s = &m_sides[j];

			auto k=sideedges[i].begin();
			while(k != sideedges[i].end())
			{
				if(!PointBehindPlane(k->m_vertex[0], s->m_plane) || !PointBehindPlane(k->m_vertex[1], s->m_plane))
				{
#ifdef DEBUG_COLLAPSE
					g_log<<"-----------remove side["<<i<<"] edge ("<<k->m_vertex[0].x<<","<<k->m_vertex[0].y<<","<<k->m_vertex[0].z<<")->("<<k->m_vertex[1].x<<","<<k->m_vertex[1].y<<","<<k->m_vertex[1].z<<") out of plane["<<j<<"]=("<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<"d="<<s->m_plane.m_d<<")--------------"<<endl;
					g_log.flush();
#endif
					k = sideedges[i].erase(k);
					continue;
				}

				k++;
			}
		}

#ifdef DEBUG_COLLAPSE
		g_log<<"side"<<i<<" 2remaining edges"<<endl;
		for(auto k=sideedges[i].begin(); k!=sideedges[i].end(); k++)
		{
			Line l = *k;
			g_log<<"("<<l.m_vertex[0].x<<","<<l.m_vertex[0].y<<","<<l.m_vertex[0].z<<")->("<<l.m_vertex[1].x<<","<<l.m_vertex[1].y<<","<<l.m_vertex[1].z<<")"<<endl;
		}
#endif
	}
	
#ifdef DEBUG_COLLAPSE
	g_log<<"tag8. m_nsides="<<m_nsides<<endl;
	g_log.flush();
#endif
	
	//construct outlines of polygons from side edges
	//Polyg* sidepolys = new Polyg[m_nsides];
	for(int i=0; i<m_nsides; i++)
	{
#ifdef DEBUG_COLLAPSE
		for(auto j=sideedges[i].begin(); j!=sideedges[i].end(); j++)
		{
			Line* edge = &*j;
			if(Close(edge->m_vertex[0], edge->m_vertex[1]))
			{
				//j = sideedges[i].erase(j);
				g_log<<"close side["<<i<<"] edge ("<<edge->m_vertex[0].x<<","<<edge->m_vertex[0].y<<","<<edge->m_vertex[0].z<<")->("<<edge->m_vertex[1].x<<","<<edge->m_vertex[1].y<<","<<edge->m_vertex[1].z<<")"<<endl;
			}
		}
#endif

		if(sideedges[i].size() < 3)
		{
#ifdef DEBUG_COLLAPSE
			g_log<<"sideedges["<<i<<"] < 3"<<endl;
			g_log.flush();
#endif
			continue;
		}

		auto j=sideedges[i].begin();
		//auto lastconnection = j;
		set<Line*> connected;
		//set<Vec3f> connectedv;
		list<Vec3f> connectedv;
		//set<Vec3f> connected;

		Vec3f first = j->m_vertex[0];
		//connectedv.insert(first);
		connectedv.push_back(first);
		//sidepolys[i].m_vertex.push_back(first);
		EdBrushSide* s = &m_sides[i];
		s->m_outline.m_edv.clear();
		s->m_outline.m_edv.push_back(first);

		Vec3f curr = j->m_vertex[1];
		for(j++; j!=sideedges[i].end(); j++)
		{
#ifdef DEBUG_COLLAPSE
			g_log<<"sideedges["<<i<<"] first="<<first.x<<","<<first.y<<","<<first.z<<endl;
			g_log.flush();
#endif

			//if(j == lastconnection)
			//	continue;

			if(connected.count(&*j) > 0)
				continue;

			int whichclose = -1;
			
#ifdef DEBUG_COLLAPSE
			g_log<<"\tclose? "<<j->m_vertex[0].x<<","<<j->m_vertex[0].y<<","<<j->m_vertex[0].z<<" == "<<curr.x<<","<<curr.y<<","<<curr.z<<endl;
			g_log.flush();
#endif

			if(Close(j->m_vertex[0], curr))
			{
#ifdef DEBUG_COLLAPSE
				g_log<<"\t\tyes"<<endl;
				g_log.flush();
#endif
				whichclose = 0;
				goto foundnext;
			}
			
#ifdef DEBUG_COLLAPSE
			g_log<<"\tclose? "<<j->m_vertex[1].x<<","<<j->m_vertex[1].y<<","<<j->m_vertex[1].z<<" == "<<curr.x<<","<<curr.y<<","<<curr.z<<endl;
			g_log.flush();
#endif

			if(Close(j->m_vertex[1], curr))
			{
#ifdef DEBUG_COLLAPSE
				g_log<<"\t\tyes"<<endl;
				g_log.flush();
#endif
				whichclose = 1;
				goto foundnext;
			}

			foundnext:

			if(whichclose >= 0)
			{
				//lastconnection = j;
				connected.insert(&*j);

				Vec3f next = j->m_vertex[1-whichclose];

				bool found = false;
				for(auto k=connectedv.begin(); k!=connectedv.end(); k++)
				{
					if(Close(*k, next))
					{
						found = true;
						break;
					}
				}

				//if(connectedv.count( next ) > 0)
				if(found)
				{
#ifdef DEBUG_COLLAPSE
					g_log<<"\t\t found"<<endl;
#endif
					if(Close(next, first))
					{
#ifdef DEBUG_COLLAPSE
						g_log<<"\t\t\t next close to first"<<endl;
#endif
						if(s->m_outline.m_edv.size() < 2)
						{
#ifdef DEBUG_COLLAPSE
							g_log<<"found close(next,first) outline verts < 3"<<endl;
#endif
							continue;
						}
					}
					else
						continue;
				}
#ifdef DEBUG_COLLAPSE
				else
					g_log<<"\t\t unique"<<endl;
#endif

				//connectedv.insert(next);
				connectedv.push_back(next);

				//sidepolys[i].m_vertex.push_back(j->m_vertex[whichclose]);
				s->m_outline.m_edv.push_back(curr);

				if(Close(curr, next))
					break;	//avoid infinite loop in degenerate sides

				curr = next;
				
#ifdef DEBUG_COLLAPSE
				g_log<<"\t\tcurr = "<<curr.x<<","<<curr.y<<","<<curr.z<<endl;
				g_log.flush();
#endif
				
				//if(Close(j->m_vertex[0], first) || Close(j->m_vertex[1], first))
				if(Close(curr, first))
				{
#ifdef DEBUG_COLLAPSE
					g_log<<"\t\t\tpolygon loop complete"<<endl;

					for(auto k=s->m_outline.m_edv.begin(); k!=s->m_outline.m_edv.end(); k++)
					{
						g_log<<"("<<(*k).x<<","<<(*k).y<<","<<(*k).z<<")->";
					}

					g_log<<endl;

					g_log.flush();
#endif
					//sidepolys[i].m_vertex.push_back(curr);
					break;	//loop completed
				}

				j = sideedges[i].begin();
			}
		}
	}

	
#ifdef DEBUG_COLLAPSE
	g_log<<"tag9. m_nsides="<<m_nsides<<endl;
	g_log.flush();
#endif
	
	//make sure all polys are clockwise
	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];

		Vec3f tri[3];
		//auto j = sidepolys[i].m_vertex.begin();
		auto j = s->m_outline.m_edv.begin();
		tri[0] = *j;	j++;
		tri[1] = *j;	j++;
		tri[2] = *j;

		Vec3f norm = Normal(tri);
		Vec3f revnorm = Normal2(tri);

		if(Magnitude(s->m_plane.m_normal - revnorm) < Magnitude(s->m_plane.m_normal - norm))
		{
#ifdef DEBUG_COLLAPSE
			g_log<<"reverse polygon loop order (revnorm("<<revnorm.x<<","<<revnorm.y<<","<<revnorm.z<<") is closer to planenorm("<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<")d="<<s->m_plane.m_d<<")"<<endl;
#endif
			//sidepolys[i].m_vertex.reverse();
			s->m_outline.m_edv.reverse();
		}
	}
	
#ifdef DEBUG_COLLAPSE
	g_log<<"tag10. m_nsides="<<m_nsides<<endl;
	g_log.flush();
#endif

	//construct triangles from polygons
	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];
		//s->m_ntris = (int)sidepolys[i].m_vertex.size() - 2;
		s->m_ntris = (int)s->m_outline.m_edv.size() - 2;
		
		//g_log<<"sidepolys["<<i<<"].m_vertex.size() = "<<(int)sidepolys[i].m_vertex.size()<<endl;
		//g_log<<"sidepolys["<<i<<"].m_vertex.size() = "<<(int)s->m_outline.m_edv.size()<<endl;

		if(s->m_ntris <= 0)
		{
#ifdef DEBUG_COLLAPSE
			g_log<<"tris["<<i<<"] = "<<0<<endl;
#endif
			continue;
		}
		
#ifdef DEBUG_COLLAPSE
			g_log<<"tris["<<i<<"] = "<<s->m_ntris<<endl;
#endif
			g_log.flush();

		s->m_tris = new Triangle2[s->m_ntris];
		
		//auto j = sidepolys[i].m_vertex.begin();
		auto j = s->m_outline.m_edv.begin();
		Vec3f first = *j;
		j++;
		Vec3f prev = *j;
		j++;
		//for(int tri=0; j!=sidepolys[i].m_vertex.end(); j++, tri++)
		for(int tri=0; j!=s->m_outline.m_edv.end(); j++, tri++)
		{
			Vec3f curr = *j;
			s->m_tris[tri].m_vertex[0] = first;
			s->m_tris[tri].m_vertex[1] = prev;
			s->m_tris[tri].m_vertex[2] = curr;
			prev = curr;
		}

		s->makeva();
	}

	
#ifdef DEBUG_COLLAPSE
	g_log<<"tag11. m_nsides="<<m_nsides<<endl;
	g_log.flush();
#endif
	
	//delete [] sidepolys;
	bool* removes = new bool[m_nsides];	//degenerate sides to remove
	
	//remove bounding planes outside of the brush.
	//if brush side has no side edges, remove it.
	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];

		//if(sideedges[i].size() <= 0)
		if(sideedges[i].size() < 3 || 
			s->m_outline.m_edv.size() < 3 ||	//3 is the minimum number of edges to form a polygon
			s->m_ntris <= 0)
		{
#ifdef DEBUG_COLLAPSE
			g_log<<"remove side. "<<i<<" sideedges[i].size()="<<sideedges[i].size()<<" s->m_outline.m_edv.size()="<<s->m_outline.m_edv.size()<<" s->m_ntris="<<s->m_ntris<<endl;
			g_log.flush();
#endif
			//removeside(i);
			//i--;
			removes[i] = true;
		}
		else
			removes[i] = false;
	}

	for(int i=m_nsides-1; i>=0; i--)
		if(removes[i])
			removeside(i);

	delete [] removes;

	delete [] sideedges;
	
#ifdef DEBUG_COLLAPSE
	g_log<<"1a"<<endl;
	g_log.flush();
#endif

	//get shared vertices and centroids
	vector<Vec3f> sharedv;
	for(int i=0; i<m_nsides; i++)
	{
		EdBrushSide* s = &m_sides[i];

		if(s->m_ntris <= 0)
			continue;
		
		//s->m_vindices = new int[s->m_outline.m_edv.size()];
		s->m_vindices = new int[s->m_ntris+2];
		
#ifdef DEBUG_COLLAPSE
	g_log<<"1b side"<<i<<" vindices="<<s->m_outline.m_edv.size()<<endl;
	g_log.flush();
#endif

		Vec3f centroid(0,0,0);
		float count = 0;

		int jj=0;
		for(auto j=s->m_outline.m_edv.begin(); j!=s->m_outline.m_edv.end(); j++, jj++)
		{
			centroid = centroid * (count/(count+1)) + (*j) * (1.0f/(count+1));
#ifdef DEBUG_COLLAPSE
			g_log<<"centroid "<<count<<" "<<centroid.x<<","<<centroid.y<<","<<centroid.z<<" *j "<<(*j).x<<","<<(*j).y<<","<<(*j).z<<endl;
			g_log.flush();
#endif

			count += 1;

			bool found = false;
			for(unsigned int k=0; k<sharedv.size(); k++)
			{
				if(Close(sharedv[k], *j))
				{
					found = true;
					s->m_vindices[jj] = k;
				}
			}

			if(!found)
			{
				sharedv.push_back(*j);
				s->m_vindices[jj] = sharedv.size()-1;
			}
		}

		s->m_centroid = centroid;
		
#ifdef DEBUG_COLLAPSE
		g_log<<"1c side"<<i<<endl;
		g_log.flush();
#endif
	}
	
#ifdef DEBUG_COLLAPSE
	g_log<<"1d"<<endl;
	g_log.flush();
#endif

	m_nsharedv = sharedv.size();
	m_sharedv = new Vec3f[m_nsharedv];
	for(int i=0; i<m_nsharedv; i++)
		m_sharedv[i] = sharedv[i];
	
#ifdef DEBUG_COLLAPSE
	g_log<<"shared vertices num = "<<m_nsharedv<<endl;
	g_log.flush();
#endif

	if(g_sel1b == this && oldnsharedv != m_nsharedv)
		g_dragV = -1;
}

void EdBrush::remaptex()
{
	for(int i=0; i<m_nsides; i++)
		m_sides[i].remaptex();
}

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float radius, const float height)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, height, 0), Vec3f(0, 1, 0));	//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, 0, 0), Vec3f(0, -1, 0));		//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(-radius, 0, 0), Vec3f(-1, 0, 0));	//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(radius, 0, 0), Vec3f(1, 0, 0));	//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, -radius), Vec3f(0, 0, -1));	//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, radius), Vec3f(0, 0, 1));	//back
}

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float hwx, const float hwz, const float height)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, height, 0), Vec3f(0, 1, 0));	//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, 0, 0), Vec3f(0, -1, 0));		//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(-hwx, 0, 0), Vec3f(-1, 0, 0));		//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(hwx, 0, 0), Vec3f(1, 0, 0));		//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, -hwz), Vec3f(0, 0, -1));		//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, hwz), Vec3f(0, 0, 1));		//back
}

// line intersects convex hull?
bool LineInterHull(const Vec3f* line, Plane* planes, const int numplanes)
{
	for(int i=0; i<numplanes; i++)
    {
		Vec3f inter;
        if(LineInterPlane(line, planes[i].m_normal, -planes[i].m_d, &inter))
        {
			bool allin = true;
			for(int j=0; j<numplanes; j++)
			{
				if(i == j)
					continue;

				if(!PointBehindPlane(inter, planes[j]))
				{
					allin = false;
					break;
				}
			}
			if(allin)
			{
				return true;
			}
        }
    }

    return false;
}