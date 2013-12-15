

#ifndef TEXTAREA_H
#define TEXTAREA_H

#include "widget.h"

class TextArea : public Widget
{
public:
	int m_highl[2];	// highlighted (selected) text

	TextArea(Widget* parent, const char* n, const RichText t, int f, Margin left, Margin top, Margin right, Margin bottom, float r=1, float g=1, float b=1, float a=1, void (*change)()=NULL);

	void draw();
	bool lbuttonup(bool moved);
	bool lbuttondown();
	bool mousemove();
	int rowsshown();
	int square();

	float topratio()
	{
		return m_scroll[1] / (float)m_lines;
	}

	float bottomratio()
	{
		return (m_scroll[1]+rowsshown()) / (float)m_lines;
	}

	float scrollspace();
	bool keydown(int k);
	bool keyup(int k);
	bool charin(int k);
	void changevalue(const char* newv);
	bool delnext();
	bool delprev();
	void copyval();
	void pasteval();
	void selectall();
	void placestr(const char* str);
};

#endif