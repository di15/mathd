

#include "widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "dropdowns.h"
#include "hscroller.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"
#include "icon.h"

TextArea::TextArea(Widget* parent, const char* n, const RichText t, int f, Margin left, Margin top, Margin right, Margin bottom, float r, float g, float b, float a, void (*change)()) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_TEXTAREA;
	m_name = n;
	m_value = t;
	m_font = f;
	m_pos[0] = left;
	m_pos[1] = top;
    m_pos[2] = right;
    m_pos[3] = bottom;
	m_ldown = false;
	m_rgba[0] = r;
	m_rgba[1] = g;
	m_rgba[2] = b;
	m_rgba[3] = a;
	m_highl[0] = 0;
	m_highl[1] = 0;
	changefunc = change;
	m_scroll[1] = 0;
	m_highl[0] = 0;
	m_highl[1] = 0;
	m_caret = m_value.texlen();
	CreateTexture(m_frametex, "gui\\frame.jpg", true);
	CreateTexture(m_filledtex, "gui\\filled.jpg", true);
	CreateTexture(m_uptex, "gui\\up.jpg", true);
	CreateTexture(m_downtex, "gui\\down.jpg", true);
	reframe();
	m_lines = CountLines(&m_value, f, m_pos[0].m_cached, m_pos[1].m_cached, m_pos[2].m_cached-m_pos[0].m_cached-square(), m_pos[3].m_cached-m_pos[1].m_cached);
}

void TextArea::draw()
{
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	DrawImage(m_frametex, m_pos[0].m_cached, m_pos[1].m_cached, m_pos[2].m_cached, m_pos[3].m_cached);
	
	DrawImage(g_texture[m_frametex].texname, m_pos[2].m_cached-square(), m_pos[1].m_cached, m_pos[2].m_cached, m_pos[3].m_cached);
	DrawImage(g_texture[m_uptex].texname, m_pos[2].m_cached-square(), m_pos[1].m_cached, m_pos[2].m_cached, m_pos[1].m_cached+square());
	DrawImage(g_texture[m_downtex].texname, m_pos[2].m_cached-square(), m_pos[3].m_cached-square(), m_pos[2].m_cached, m_pos[3].m_cached);
	DrawImage(g_texture[m_filledtex].texname, m_pos[2].m_cached-square(), m_pos[1].m_cached+square()+scrollspace()*topratio(), m_pos[2].m_cached, m_pos[1].m_cached+square()+scrollspace()*bottomratio());

    float width = m_pos[2].m_cached - m_pos[0].m_cached - square();
    float height = m_pos[3].m_cached - m_pos[1].m_cached;
    
    //DrawBoxShadText(m_font, m_pos[0].m_cached, m_pos[1].m_cached, width, height, m_value.c_str(), m_rgba, m_scroll[1], m_opened ? m_caret : -1);

	DrawShadowedTextF(m_font, m_pos[0].m_cached+m_scroll[0], m_pos[1].m_cached, m_pos[0].m_cached, m_pos[1].m_cached, m_pos[2].m_cached, m_pos[3].m_cached, &m_value, NULL, m_opened ? m_caret : -1);

	HighlightF(m_font, m_pos[0].m_cached+m_scroll[0], m_pos[1].m_cached, m_pos[0].m_cached, m_pos[1].m_cached, m_pos[2].m_cached, m_pos[3].m_cached, &m_value, m_highl[0], m_highl[1]);
}

int TextArea::rowsshown()
{
	int rows = (m_pos[3].m_cached-m_pos[1].m_cached)/g_font[m_font].gheight;

	return rows;
}

int TextArea::square()
{
	return g_font[m_font].gheight;
}

float TextArea::scrollspace()
{
	return (m_pos[3].m_cached-m_pos[1].m_cached-square()*2);
}

bool TextArea::mousemove()
{
	if(m_ldown)
	{
		int newcaret = MatchGlyphF(&m_value, m_font, g_mouse.x, m_pos[0].m_cached+m_scroll[0], m_pos[1].m_cached, m_pos[0].m_cached, m_pos[1].m_cached, m_pos[2].m_cached, m_pos[3].m_cached);
		
		if(newcaret > m_caret)
		{
			m_highl[0] = m_caret;
			m_highl[1] = newcaret;
			//g_log<<"hihgl "<<m_highl[0]<<"->"<<m_highl[1]<<endl;
			//g_log.flush();
		}
		else
		{
			m_highl[0] = newcaret;
			m_highl[1] = m_caret;
			//g_log<<"hihgl "<<m_highl[0]<<"->"<<m_highl[1]<<endl;
			//g_log.flush();
		}

		return true;
	}

	if(g_mouse.x >= m_pos[0].m_cached && g_mouse.x <= m_pos[2].m_cached && g_mouse.y >= m_pos[1].m_cached && g_mouse.y <= m_pos[3].m_cached)
	{
		m_over = true;

		return true;
	}
	else
	{
		m_over = false;

		return false;
	}
}

bool TextArea::lbuttondown()
{
	if(m_over)
	{
		m_ldown = true;
		return true;
	}

	return false;
}

bool TextArea::lbuttonup(bool moved)
{
	if(m_over && m_ldown)
	{
		m_opened = true;
		m_ldown = false;
		return true;
	}

	m_ldown = false;

	if(m_opened)
	{
		m_opened = false;
		return true;
	}

	return false;
}

bool TextArea::keydown(int k)
{
	if(!m_opened)
		return false;
	
	//int len = m_value.length();
	int len = m_value.texlen();

	if(m_caret > len)
		m_caret = len;

	if(k == VK_LEFT)
	{
		if(m_caret <= 0)
			return true;

		m_caret --;
	}
	else if(k == VK_RIGHT)
	{
		int len = m_value.texlen();
		
		if(m_caret >= len)
			return true;

		m_caret ++;
	}
	else if(k == 190 && !g_keys[VK_SHIFT])
		placechar('.');
	
	if(changefunc != NULL)
		changefunc();

	if(changefunc2 != NULL)
		changefunc2(m_param);

	return true;
}

void TextArea::changevalue(const char* newv)
{
	m_value = newv;
	if(m_caret > strlen(newv))
		m_caret = strlen(newv);
	m_lines = CountLines(&m_value, MAINFONT8, m_pos[0].m_cached, m_pos[1].m_cached, m_pos[2].m_cached-m_pos[0].m_cached-square(), m_pos[3].m_cached-m_pos[1].m_cached);
}

bool TextArea::keyup(int k)
{
	if(!m_opened)
		return false;

	return true;
}

void TextArea::placestr(const char* str)
{
	int len = m_value.texlen();

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		len -= m_highl[1] - m_highl[0];
	}

	int addlen = strlen(str);
	if(addlen + len >= m_maxlen)
		addlen = m_maxlen - len;

	char* addstr = new char[addlen+1];
	
	if(addlen > 0)
	{
		for(int i=0; i<addlen; i++)
			addstr[i] = str[i];
		addstr[addlen] = '\0';
	}
	else
	{
		delete [] addstr;
		return;
	}

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText before = m_value.substr(0, m_highl[0]);
		RichText after = m_value.substr(m_highl[1]-1, m_value.texlen()-m_highl[1]);
		m_value = before + addstr + after;

		m_caret = m_highl[0] + addlen;
		m_highl[0] = m_highl[1] = 0;
	}
	else
	{
		if(len >= m_maxlen)
		{
			delete [] addstr;
			return;
		}

		RichText before = m_value.substr(0, m_caret);
		RichText after = m_value.substr(m_caret, m_value.texlen()-m_caret);
		m_value = before + addstr + after;
		m_caret += addlen;
	}
	
	//RichText val = drawvalue();
	int endx = EndX(&m_value, m_caret, m_font, m_pos[0].m_cached+m_scroll[0], m_pos[1].m_cached);

	if(endx >= m_pos[2].m_cached)
		m_scroll[0] -= endx - m_pos[2].m_cached + 1;

	delete [] addstr;
}

bool TextArea::delnext()
{
	int len = m_value.texlen();
		
	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText before = m_value.substr(0, m_highl[0]);
		RichText after = m_value.substr(m_highl[1], len-m_highl[1]);
		m_value = before + after;

		m_caret = m_highl[0];
		m_highl[0] = m_highl[1] = 0;
	}
	else if(m_caret >= len || len <= 0)
		return true;
	else
	{
		RichText before = m_value.substr(0, m_caret);
		RichText after = m_value.substr(m_caret+1, len-m_caret);
		m_value = before + after;
	}
		
	//RichText val = drawvalue();
	int endx = EndX(&m_value, m_caret, m_font, m_pos[0].m_cached+m_scroll[0], m_pos[1].m_cached);

	if(endx <= m_pos[0].m_cached)
		m_scroll[0] += m_pos[0].m_cached - endx + 1;
	else if(endx >= m_pos[2].m_cached)
		m_scroll[0] -= endx - m_pos[2].m_cached + 1;

	return true;
}

bool TextArea::delprev()
{
	int len = m_value.texlen();

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText before = m_value.substr(0, m_highl[0]);
		RichText after = m_value.substr(m_highl[1], len-m_highl[1]);
		m_value = before + after;

		m_caret = m_highl[0];
		m_highl[0] = m_highl[1] = 0;
	}
	else if(m_caret <= 0 || len <= 0)
		return true;
	else
	{
		RichText before = m_value.substr(0, m_caret-1);
		RichText after = m_value.substr(m_caret, len-m_caret);
		m_value = before + after;

		m_caret--;
	}
		
	//RichText val = drawvalue();
	int endx = EndX(&m_value, m_caret, m_font, m_pos[0].m_cached+m_scroll[0], m_pos[1].m_cached);

	if(endx <= m_pos[0].m_cached)
		m_scroll[0] += m_pos[0].m_cached - endx + 1;
	else if(endx >= m_pos[2].m_cached)
		m_scroll[0] -= endx - m_pos[2].m_cached + 1;

	return true;
}

//#define PASTE_DEBUG

void TextArea::copyval()
{
#ifdef PASTE_DEBUG
	g_log<<"copy vkc"<<endl;
	g_log.flush();
#endif

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText highl = m_value.substr(m_highl[0], m_highl[1]-m_highl[0]);
		string rawhighl = highl.rawstr();
		const size_t len = strlen(rawhighl.c_str())+1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), rawhighl.c_str(), len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}
	else
	{
		const char* output = "";
		const size_t len = strlen(output)+1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), output, len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}

	//return true;
}

void TextArea::pasteval()
{
#ifdef PASTE_DEBUG
	g_log<<"paste"<<endl;
#endif
	OpenClipboard(NULL);
			
#ifdef PASTE_DEBUG
	g_log<<"paste1"<<endl;
#endif
	HANDLE clip0 = GetClipboardData(CF_TEXT);
			
#ifdef PASTE_DEBUG
	g_log<<"paste2"<<endl;
#endif
	//HANDLE h = GlobalLock(clip0);
	//placestr((char*)clip0);
	char* str = (char*)GlobalLock(clip0);
#ifdef PASTE_DEBUG
	g_log<<"paste3"<<endl;
	g_log<<str<<endl;
#endif

	placestr(str);

#ifdef PASTE_DEBUG
	g_log<<"place str ";
	g_log<<str<<endl;
	g_log.flush();
	g_log.flush();
#endif

	if(!m_passw)
		m_value = ParseTags(m_value, &m_caret);

	GlobalUnlock(clip0);
	CloseClipboard();

	//return true;
}

void TextArea::selectall()
{
	m_highl[0] = 0;
	m_highl[1] = m_value.texlen()+1;
	m_caret = -1;
			
	//RichText val = drawvalue();
	int endx = EndX(&m_value, m_value.texlen(), m_font, m_pos[0].m_cached+m_scroll[0], m_pos[1].m_cached);

	if(endx <= m_pos[2].m_cached)
		m_scroll[0] += m_pos[2].m_cached - endx - 1;

	if(m_scroll[0] >= 0)
		m_scroll[0] = 0;

	//return true;
}

bool TextArea::charin(int k)
{
	if(!m_opened)
		return false;
	
	int len = m_value.texlen();

	if(m_caret > len)
		m_caret = len;

	if(k == VK_BACK)
	{
		len = m_value.texlen();

		if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && m_caret >= len || len <= 0)
			return true;

		delprev();

		if(!m_passw)
			m_value = ParseTags(m_value, &m_caret);
	}
	else if(k == VK_DELETE)
	{
		len = m_value.texlen();
		
		if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && m_caret >= len || len <= 0)
			return true;

		delnext();

		if(!m_passw)
			m_value = ParseTags(m_value, &m_caret);
	}
	else if(k == VK_SHIFT)
		return true;
	else if(k == VK_CAPITAL)
		return true;
	else if(k == VK_SPACE)
	{
		placechar(' ');
	}
	else if(k == VK_RETURN)
	{
		placechar('\n');
	}
	//else if(k >= 'A' && k <= 'Z' || k >= 'a' && k <= 'z')
	//else if(k == 190 || k == 188)
	//else if((k >= '!' && k <= '@') || (k >= '[' && k <= '`') || (k >= '{' && k <= '~') || (k >= '0' || k <= '9'))
	else 
	{
		
#ifdef PASTE_DEBUG
		g_log<<"charin "<<(char)k<<" ("<<k<<")"<<endl;
		g_log.flush();
#endif

		//if(k == 'C' && g_keys[VK_CONTROL])
		if(k == 3)	//copy
		{
			copyval();
		}
		//else if(k == 'V' && g_keys[VK_CONTROL])
		else if(k == 22)	//paste
		{
			pasteval();
		}
		//else if(k == 'A' && g_keys[VK_CONTROL])
		else if(k == 1)	//select all
		{
			selectall();
		}
		else
			placechar(k);
	}
	
	if(changefunc != NULL)
		changefunc();

	if(changefunc2 != NULL)
		changefunc2(m_param);

	return true;
}

