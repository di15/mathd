
#ifndef COUNTRY_H
#define COUNTRY_H



class Country
{
public:
	float colorcode[4];

	// Indexes into the resource types array.
	int currencyres;
};

#define COUNTRIES	4

extern Country g_country[COUNTRIES];

#endif