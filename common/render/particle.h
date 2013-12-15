

#ifndef _PARTICLE_H
#define _PARTICLE_H

#pragma warning(disable: 4018)

#include "3dmath.h"

class CParticle;
class CBillboard;

class CParticleType
{
public:
	int billbT;
	int delay;
	float decay;
	Vec3f minvelocity;
	Vec3f velvariation;
	Vec3f minacceleration;
	Vec3f accelvariation;
	float minsize;
	float sizevariation;
	void (*collision)(CParticle* part, CBillboard* billb, Vec3f trace, Vec3f normal);
};

enum PARTICLETYPE{EXHAUST, EXHAUSTBIG, FIREBALL, FIREBALL2, SMOKE, SMOKE2, DEBRIS, FLAME, PLUME, PARTICLE_TYPES};
extern CParticleType g_particleType[PARTICLE_TYPES];

class EmitterCounter
{
public:
	long last;

	EmitterCounter() { last = GetTickCount(); }
	bool EmitNext(int delay) 
	{ 
		if(GetTickCount()-last > delay) 
		{
			last = GetTickCount();
			return true;
		} 
		else 
			return false; 
	}
};

class CBillboard;

class CParticle
{
public:
	bool on;
	int type;
	float life;
	Vec3f vel;
	float dist;

	CParticle() { on = false; }
	CParticle(Vec3f p, Vec3f v) { on = true; vel = v; }

	void Update(CBillboard* billb);
};

#define PARTICLES 256
extern CParticle g_particle[PARTICLES];

class EmitterPlace
{
public:
	EmitterPlace() {}
	EmitterPlace(int t, Vec3f off) { type=t; offset=off; }
	Vec3f offset;
	int type;
};

void LoadParticles();
void Particles();
void EmitParticle(int type, Vec3f pos);
void UpdateParticles();

#endif