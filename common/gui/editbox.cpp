

#include "widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "dropdowns.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"
#include "gui.h"

EditBox::EditBox(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* thisw), bool pw, int maxl, void (*change2)(int p), int parm) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_EDITBOX;
	m_name = n;
	m_font = f;
	m_value = t;
	m_caret = m_value.texlen();
	m_opened = false;
	m_passw = pw;
	m_maxlen = maxl;
	reframefunc = reframef;
	m_scroll[0] = 0;
	m_highl[0] = 0;
	m_highl[1] = 0;
	CreateTexture(m_frametex, "gui\\frame.jpg", true);
	m_param = parm;
	changefunc2 = change2;
	reframe();
}

RichText EditBox::drawvalue()
{
	/*
	string val = m_value;

	if(m_passw)
	{
		val = "";
		for(int i=0; i<m_value.length(); i++)
			val.append("*");
	}

	return val;*/

	if(!m_passw)
		return m_value;

	return m_value.pwver();
}

void EditBox::draw()
{
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	DrawImage(g_texture[m_frametex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

	if(m_over)
		//glColor4f(1, 1, 1, 1);
		glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
	else
		//glColor4f(0.8f, 0.8f, 0.8f, 1);
		glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 0.8f, 0.8f, 0.8f, 1);

	RichText val = drawvalue();

	//if(m_opened)
	//	g_log<<"op m_caret="<<m_caret<<endl;

	DrawShadowedTextF(m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], &val, NULL, m_opened ? m_caret : -1);

	//glColor4f(1, 1, 1, 1);
	//glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	HighlightF(m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], &val, m_highl[0], m_highl[1]);
}

bool EditBox::mousemove()
{
//#ifdef MOUSESC_DEBUG
	//g_log<<"editbox mousemove"<<endl;
	//g_log.flush();
//#endif

	if(m_ldown)
	{
		RichText val = drawvalue();
		int newcaret = MatchGlyphF(&val, m_font, g_mouse.x, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);
		
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

	if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
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

//#define MOUSESC_DEBUG

void EditBox::frameupd()
{
#ifdef MOUSESC_DEBUG
	g_log<<"editbox frameup"<<endl;
	g_log.flush();
#endif

	if(m_ldown)
	{
		bool movedcar = false;

#ifdef MOUSESC_DEBUG
		g_log<<"ldown frameup"<<endl;
		g_log.flush();
#endif

		if(g_mouse.x >= m_pos[2]-5)
		{
			m_scroll[0] -= max(1, g_font[m_font].gheight/4.0f);

			RichText val = drawvalue();
			int vallen = val.texlen();
			
			int endx = EndX(&val, vallen, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

			if(endx < m_pos[2])
				m_scroll[0] += m_pos[2] - endx;

			if(m_scroll[0] > 0.0f)
				m_scroll[0] = 0.0f;

			movedcar = true;
		}
		else if(g_mouse.x <= m_pos[0]+5)
		{
			m_scroll[0] += max(1, g_font[m_font].gheight/4.0f);

			if(m_scroll[0] > 0.0f)
				m_scroll[0] = 0.0f;

			movedcar = true;
		}

		if(movedcar)
		{
			RichText val = drawvalue();
			int newcaret = MatchGlyphF(&val, m_font, g_mouse.x, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

			if(newcaret > m_caret)
			{
				m_highl[0] = m_caret;
				m_highl[1] = newcaret;
			}
			else if(newcaret < m_caret)
			{
				m_highl[0] = newcaret;
				m_highl[1] = m_caret;
			}
		}
	}
}

bool EditBox::prelbuttondown()
{
	if(m_opened)
	{
		m_opened = false;
		m_highl[0] = m_highl[1] = 0;
	}

	return false;
}

bool EditBox::lbuttondown()
{
	if(m_over)
	{
		m_ldown = true;

		RichText val = drawvalue();

		//m_highl[1] = MatchGlyphF(m_value.c_str(), m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);
		//m_highl[0] = m_highl[1];
		//m_caret = m_highl[1];
		m_caret = MatchGlyphF(&val, m_font, g_mouse.x, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

		m_highl[0] = 0;
		m_highl[1] = 0;

		return true;
	}

	return false;
}

bool EditBox::lbuttonup(bool moved)
{
	//if(m_over && m_ldown)
	if(m_ldown)
	{
		m_opened = true;
		m_ldown = false;

		if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
		{
			m_caret = -1;
		}

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

bool EditBox::keydown(int k)
{
	if(!m_opened)
		return false;
	
	int len = m_value.texlen();

	if(m_caret > len)
		m_caret = len;

	if(k == VK_LEFT)
	{	
		if(m_highl[0] > 0 && m_highl[0] != m_highl[1])
		{
			m_caret = m_highl[0];
			m_highl[0] = m_highl[1] = 0;
		}
		else if(m_caret <= 0)
			return true;
		else
			m_caret --;
		
		RichText val = drawvalue();
		int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

		//g_log<<"left endx = "<<endx<<"/"<<m_pos[0]<<endl;
		//g_log.flush();

		if(endx <= m_pos[0])
			m_scroll[0] += m_pos[0] - endx + 1;
	}
	else if(k == VK_RIGHT)
	{
		int len = m_value.texlen();

		if(m_highl[0] > 0 && m_highl[0] != m_highl[1])
		{
			m_caret = m_highl[1];
			m_highl[0] = m_highl[1] = 0;
		}
		else if(m_caret >= len)
			return true;
		else
			m_caret ++;
		
		RichText val = drawvalue();
		int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

		if(endx >= m_pos[2])
			m_scroll[0] -= endx - m_pos[2] + 1;
	}
	else if(k == VK_DELETE)
	{
		len = m_value.texlen();

		//g_log<<"vk del"<<endl;
		//g_log.flush();
		
		if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && m_caret >= len || len <= 0)
			return true;

		delnext();

		if(!m_passw)
			m_value = ParseTags(m_value, &m_caret);
	}
	else if(k == 190 && !g_keys[VK_SHIFT])
	{
		//placechar('.');
	}
	
	if(changefunc2 != NULL)
		changefunc2(m_param);

	return true;
}

bool EditBox::keyup(int k)
{
	if(!m_opened)
		return false;

	return true;
}

void EditBox::placechar(int k)
{
	int len = m_value.texlen();

	if(len >= m_maxlen)
		return;

	//char addchar = k;

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText before = m_value.substr(0, m_highl[0]);
		RichText after = m_value.substr(m_highl[1], len-m_highl[1]);
		m_value = before + after;

		m_caret = m_highl[0];
		m_highl[0] = m_highl[1] = 0;

		before = m_value.substr(0, m_caret);
		after = m_value.substr(m_caret, len-m_caret);
		m_value = before + RichTextP(UString(k)) + after;
		m_caret ++;
	}
	else
	{
		RichText before = m_value.substr(0, m_caret);
		RichText after = m_value.substr(m_caret, len-m_caret);
		
		//g_log<<"m_value str: "<<m_value.rawstr()<<endl;
		//g_log<<"before str: "<<before.rawstr()<<endl;
		//g_log<<"after str: "<<after.rawstr()<<endl;
		//g_log<<"addstr: "<<RichText(RichTextP(UString(k))).rawstr()<<endl;
		//g_log.flush();

		m_value = before + RichText(RichTextP(UString(k))) + after;
		m_caret ++;
	}
	
	RichText val = drawvalue();
	int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;
}

void EditBox::placestr(const char* str)
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
	
	RichText val = drawvalue();
	int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;

	delete [] addstr;
}

void EditBox::changevalue(const char* str)
{
	int len = m_value.texlen();

	if(len >= m_maxlen)
		return;

	int setlen = strlen(str);
	if(setlen >= m_maxlen)
		setlen = m_maxlen;

	char* setstr = new char[setlen+1];
	
	for(int i=0; i<setlen; i++)
		setstr[i] = str[i];
	setstr[setlen] = '\0';

	m_value = setstr;
	m_highl[0] = m_highl[1] = 0;
	m_caret = 0;

	delete [] setstr;
}

bool EditBox::delnext()
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
		
	RichText val = drawvalue();
	int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[0])
		m_scroll[0] += m_pos[0] - endx + 1;
	else if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;

	return true;
}

bool EditBox::delprev()
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
		
		//g_log<<"before newval="<<before.rawstr()<<" texlen="<<before.texlen()<<endl;
		//g_log<<"after="<<after.rawstr()<<" texlen="<<after.texlen()<<endl;
		//g_log<<"ba newval="<<m_value.rawstr()<<" texlen="<<(before + after).texlen()<<endl;
		//g_log<<"newval="<<m_value.rawstr()<<" texlen="<<m_value.texlen()<<endl;

		m_caret--;
	}
		
	RichText val = drawvalue();
	int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[0])
		m_scroll[0] += m_pos[0] - endx + 1;
	else if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;

	return true;
}

//#define PASTE_DEBUG

void EditBox::copyval()
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

void EditBox::pasteval()
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

void EditBox::selectall()
{
	m_highl[0] = 0;
	m_highl[1] = m_value.texlen()+1;
	m_caret = -1;
			
	RichText val = drawvalue();
	int endx = EndX(&val, m_value.texlen(), m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[2])
		m_scroll[0] += m_pos[2] - endx - 1;

	if(m_scroll[0] >= 0)
		m_scroll[0] = 0;

	//return true;
}

bool EditBox::charin(int k)
{
	if(!m_opened)
		return false;
	
	int len = m_value.texlen();

	if(m_caret > len)
		m_caret = len;

	//g_log<<"vk "<<k<<endl;
	//g_log.flush();

	if(k == VK_BACK)
	{
		len = m_value.texlen();

		if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && len <= 0)
			return true;

		delprev();

		if(!m_passw)
			m_value = ParseTags(m_value, &m_caret);
	}/*
	else if(k == VK_DELETE)
	{
		len = m_value.texlen();

		g_log<<"vk del"<<endl;
		g_log.flush();
		
		if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && m_caret >= len || len <= 0)
			return true;

		delnext();

		if(!m_passw)
			m_value = ParseTags(m_value, &m_caret);
	}*/
	else if(k == VK_SHIFT)
		return true;
	else if(k == VK_CAPITAL)
		return true;
	else if(k == VK_RETURN)
		return true;
	else if(k == VK_SPACE)
	{
		placechar(' ');
	}
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

