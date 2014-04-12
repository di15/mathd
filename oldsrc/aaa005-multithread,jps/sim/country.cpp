

#include "country.h"
#include "../platform.h"
#include "../ustring.h"
#include "../gui/richtext.h"

Country g_country[COUNTRIES];

void DefineCountry(int ID, float red, float green, float blue, float alpha, RichText name, int currencyres)
{
	Country* c = &g_country[ID];
	c->colorcode[0] = red;
	c->colorcode[1] = green;
	c->colorcode[2] = blue;
	c->colorcode[3] = alpha;
	c->name = name;
	c->currencyres = currencyres;
}