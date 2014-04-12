
#ifndef COUNTRY_H
#define COUNTRY_H

#include "../gui/richtext.h"

class Country
{
public:
	unsigned char player;
	float colorcode[4];
	RichText name;

	// Indexes into the resource types array.
	int currencyres;
};

#define COUNTRIES	8

extern Country g_country[COUNTRIES];

void DefineCountry(int ID, float red, float green, float blue, float alpha, RichText name, int currencyres);

#endif