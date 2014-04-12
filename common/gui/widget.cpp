

#include "widget.h"
#include "gui.h"
#include "font.h"
#include "../window.h"
#include "icon.h"

void Widget::select(int which)
{
	m_selected = which;

	if(m_selected >= m_options.size())
		m_selected = m_options.size()-1;

	if(m_selected < 0)
		m_selected = 0;

	if(changefunc != NULL)
		changefunc();
}

void Widget::clear()
{
	m_selected = -1;
	m_scroll[1] = 0;
	m_options.clear();
	m_text = "";
}

void Widget::changetext(const char* newt)
{
	m_text = newt;

	if(m_type == WIDGET_BUTTON)
	{
		float length = 0;
		for(int i=0; i<strlen(newt); i++)
			length += g_font[m_font].glyph[newt[i]].origsize[0];
		m_tpos[0] = (m_pos[0]+m_pos[2])/2.0f - length/2.0f;
		m_tpos[1] = (m_pos[1]+m_pos[3])/2.0f - g_font[m_font].gheight/2.0f;
	}
}

void Widget::changevalue(const char* newv)
{
	m_value = newv;
	if(m_caret > strlen(newv))
		m_caret = strlen(newv);
}

void Widget::placechar(unsigned int k)
{
	//int len = m_value.length();

	//if(m_type == WIDGET_EDITBOX && len >= m_maxlen)
	//	return;

	//char addchar = k;

	//string before = m_value.substr(0, m_caret);
	//string after = m_value.substr(m_caret, len-m_caret);
	//m_value = before + addchar + after;

	RichText newval;

	int currplace = 0;
	bool changed = false;
	for(auto i=m_value.m_part.begin(); i!=m_value.m_part.end(); i++)
	{
		if(currplace + i->texlen() >= m_caret && !changed)
		{
			changed = true;

			if(i->m_type == RICHTEXT_TEXT)
			{
				if(i->m_text.m_length <= 1)
					continue;

				RichTextP chpart;

				chpart.m_type = RICHTEXT_TEXT;

				int subplace = m_caret - currplace;

				if(subplace > 0)
				{
					chpart.m_text = chpart.m_text + i->m_text.substr(0, subplace);
				}

				chpart.m_text = chpart.m_text + UString(k);

				if(i->m_text.m_length - subplace > 0)
				{
					chpart.m_text = chpart.m_text + i->m_text.substr(subplace, i->m_text.m_length-subplace);
				}

				chpart.m_text = i->m_text.substr(0, i->m_text.m_length-1);

				newval = newval + RichText(chpart);
			}
			else if(i->m_type == RICHTEXT_ICON)
			{
				Icon* icon = &g_icon[i->m_icon];
				
				int subplace = m_caret - currplace;

				if(subplace <= 0)
				{
					newval = newval + RichText(RichTextP(UString(k)));
					newval = newval + RichText(*i);
				}
				else
				{
					newval = newval + RichText(*i);
					newval = newval + RichText(RichTextP(UString(k)));
				}
			}

		}
		else
		{
			newval = newval + RichText(*i);
			currplace += i->texlen();
		}
	}

	m_value = newval;

	m_caret ++;
}

void Widget::reframe()	//resized or moved
{
	if(reframefunc)
		reframefunc(this);

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->reframe();
}

bool Widget::delprev()
{
	int len = m_value.texlen();
		
	if(m_caret <= 0 || len <= 0)
		return true;

	//string before = m_value.substr(0, m_caret-1);
	//string after = m_value.substr(m_caret, len-m_caret);
	//m_value = before;
	//m_value.append(after);

	RichText newval;

	int currplace = 0;
	bool changed = false;
	for(auto i=m_value.m_part.begin(); i!=m_value.m_part.end(); i++)
	{
		if(currplace + i->texlen() >= m_caret && !changed)
		{
			changed = true;
			RichTextP chpart;

			if(i->m_type == RICHTEXT_TEXT)
			{
				if(i->m_text.m_length <= 1)
					continue;

				chpart.m_type = RICHTEXT_TEXT;
				chpart.m_text = i->m_text.substr(0, i->m_text.m_length-1);
			}
			else if(i->m_type == RICHTEXT_ICON)
			{
				Icon* icon = &g_icon[i->m_icon];

				m_caret += icon->m_tag.m_length - 1;
					
				if(icon->m_tag.m_length <= 1)
					continue;
					
				chpart.m_type = RICHTEXT_TEXT;
				chpart.m_text = icon->m_tag.substr(0, icon->m_tag.m_length-1);
			}

			newval = newval + RichText(chpart);
		}
		else
		{
			newval = newval + RichText(*i);
			currplace += i->texlen();
		}
	}

	m_value = newval;

	m_caret--;

	return true;
}

bool Widget::delnext()
{
	int len = m_value.texlen();
		
	if(m_caret >= len || len <= 0)
		return true;

	//string before = m_value.substr(0, m_caret);
	//string after = m_value.substr(m_caret+1, len-m_caret);
	//m_value = before;
	//m_value.append(after);

	RichText newval;

	int currplace = 0;
	bool changed = false;
	for(auto i=m_value.m_part.begin(); i!=m_value.m_part.end(); i++)
	{
		if(currplace + i->texlen() > m_caret && !changed)
		{
			changed = true;
			RichTextP chpart;

			if(i->m_type == RICHTEXT_TEXT)
			{
				if(i->m_text.m_length <= 1)
					continue;

				chpart.m_type = RICHTEXT_TEXT;
				chpart.m_text = i->m_text.substr(m_caret-currplace+1, i->m_text.m_length-1);
			}
			else if(i->m_type == RICHTEXT_ICON)
			{
				Icon* icon = &g_icon[i->m_icon];
					
				if(icon->m_tag.m_length <= 1)
					continue;
					
				chpart.m_type = RICHTEXT_TEXT;
				chpart.m_text = icon->m_tag.substr(m_caret-currplace+1, icon->m_tag.m_length-1);
			}

			newval = newval + RichText(chpart);
		}
		else
		{
			newval = newval + RichText(*i);
			currplace += i->texlen();
		}
	}

	m_value = newval;

	return true;
}

void CenterLabel(Widget* w)
{
	Font* f = &g_font[w->m_font];

	int texwidth = TextWidth(w->m_font, &w->m_label);

	w->m_tpos[0] = (w->m_pos[2]+w->m_pos[0])/2 - texwidth/2;
	w->m_tpos[1] = (w->m_pos[3]+w->m_pos[1])/2 - f->gheight/2;
}