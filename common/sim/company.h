

#ifndef COMPANY_H
#define COMPANY_H

#include "../gui/richtext.h"

class Company
{
public:
	unsigned char player;
	float colorcode[4];
	RichText name;
	unsigned char homecountry;
};

#define COMPANIES		24

extern Company g_company[COMPANIES];

void DefineCompany(int ID, float red, float green, float blue, float alpha, RichText name, int homecountry);

#endif