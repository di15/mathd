

#include "company.h"

Company g_company[COMPANIES];


void DefineCompany(int ID, float red, float green, float blue, float alpha, RichText name, int homecountry)
{
	Company* c = &g_company[ID];
	c->colorcode[0] = red;
	c->colorcode[1] = green;
	c->colorcode[2] = blue;
	c->colorcode[3] = alpha;
	c->name = name;
	c->homecountry = homecountry;
}