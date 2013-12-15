

#include "richtext.h"
#include "icon.h"
#include "../utils.h"


//#define USTR_DEBUG

RichTextP::RichTextP()
{
#ifdef USTR_DEBUG
	//g_log<<"RichTextP::RichTextP()"<<endl;
	//g_log.flush();
#endif
}

RichTextP::RichTextP(const RichTextP& original)
{
	
#ifdef USTR_DEBUG
	//g_log<<"RichTextP::RichTextP(const RichTextP& original)"<<endl;
	//g_log.flush();
#endif

	*this = original;
}

RichTextP::RichTextP(const char* cstr)
{
	m_type = RICHTEXT_TEXT;
	m_text = UString(cstr);
}

RichTextP::RichTextP(UString ustr)
{
	m_type = RICHTEXT_TEXT;
	m_text = ustr;
}


RichTextP::RichTextP(int type, int subtype)
{
	if(type == RICHTEXT_ICON)
	{
		m_type = RICHTEXT_ICON;
		m_icon = subtype;
		return;
	}

	m_type = RICHTEXT_TEXT;
}

RichTextP& RichTextP::operator=(const RichTextP &original)
{
#ifdef USTR_DEBUG
	//g_log<<"RichTextP& RichTextP::operator=(const RichTextP &original)"<<endl;
	//g_log.flush();
#endif

	m_type = original.m_type;
	m_text = original.m_text;
	m_icon = original.m_icon;

	return *this;
}

int RichTextP::texlen() const	// icons count as 1 glyph
{
	if(m_type == RICHTEXT_TEXT)
		return m_text.m_length;
	else if(m_type == RICHTEXT_ICON)
		//return g_icon[m_icon].m_tag.m_length;
		return 1;

	return 0;
}

int RichTextP::rawlen() const	// icon tag length is counted
{
	if(m_type == RICHTEXT_TEXT)
		return m_text.m_length;
	else if(m_type == RICHTEXT_ICON)
		return g_icon[m_icon].m_tag.m_length;

	return 0;
}

string RichTextP::texval() const
{
	if(m_type == RICHTEXT_TEXT)
	{
		m_text.rawstr();
	}
	else if(m_type == RICHTEXT_ICON)
	{
		Icon* icon = &g_icon[m_icon];
		return icon->m_tag.rawstr();
	}

	return m_text.rawstr();
}

RichTextP RichTextP::substr(int start, int length) const
{
	if(m_type == RICHTEXT_ICON)
		return *this;
	else if(m_type == RICHTEXT_TEXT)
	{
		RichTextP subp(m_text.substr(start, length));
		return subp;
	}

	return *this;
}

RichText::RichText(const RichTextP& part)
{
	m_part.push_back(part);
}

RichText::RichText(const RichText& original)
{
	*this = original;
}

RichText::RichText(const char* cstr)
{
	m_part.push_back( RichTextP(cstr) );
}

RichText& RichText::operator=(const RichText &original)
{
#ifdef USTR_DEBUG
	//g_log<<"richtext::= ";
	//g_log.flush();
	//g_log<<original.rawstr()<<endl;
	//g_log.flush();
#endif

	m_part.clear();

	for(auto i=original.m_part.begin(); i!=original.m_part.end(); i++)
		m_part.push_back( *i );

	return *this;
}

RichText RichText::operator+(const RichText &other)
{
	RichTextP twopart;
	
	RichTextP* last1 = NULL;
	RichTextP* first2 = NULL;

	if(m_part.size() > 0 && other.m_part.size() > 0)
	{
		for(auto i=m_part.rbegin(); i!=m_part.rend(); i++)
		{
			if(i->texlen() > 0)
			{
				last1 = &*i;
				break;
			}
		}

		for(auto i=other.m_part.begin(); i!=other.m_part.end(); i++)
		{
			if(i->texlen() > 0)
			{
				first2 = (RichTextP*)&*i;
				break;
			}
		}

		if(last1->m_type == RICHTEXT_TEXT && first2->m_type == RICHTEXT_TEXT)
		{
			twopart.m_type = RICHTEXT_TEXT;
			twopart.m_text = last1->m_text + first2->m_text;
		}
		else
		{
			last1 = NULL;
			first2 = NULL;
		}
	}

	RichText combined;

	for(auto i=m_part.begin(); i!=m_part.end(); i++)
	{
		if(&*i == last1)
			break;

		if(i->texlen() <= 0)
			continue;

		combined.m_part.push_back(*i);
	}

	if(twopart.texlen() > 0)
		combined.m_part.push_back(twopart);

	for(auto i=other.m_part.begin(); i!=other.m_part.end(); i++)
	{
		if(&*i == first2)
			break;

		if(i->texlen() <= 0)
			continue;

		combined.m_part.push_back(*i);
	}

	return combined;
}

RichText RichText::substr(int start, int length) const
{
	RichText retval;

	int totallen = texlen();
	int currplace = 0;
	auto currp = m_part.begin();

	while(currplace < totallen && currplace < start+length && currp != m_part.end())
	{
		int currlen = currp->texlen();

		if(currplace < start+length && currplace+currlen >= start)
		{
			if(currp->texlen() <= 0)
				continue;

			RichTextP addp;
		
			int startplace = currplace - start;
			int endplace = currplace - (start+length);

			if(startplace < 0)
				startplace = 0;

			if(endplace > currp->texlen())
				endplace = currp->texlen();

			int addlen = endplace - startplace;

			addp = currp->substr(startplace, addlen);

			retval = retval + addp;
		}

		currplace += currlen;
		currp++;
	}

	return retval;
}

string RichText::rawstr() const
{
	string raws;

	for(auto i=m_part.begin(); i!=m_part.end(); i++)
		raws += i->texval();

	return raws;
}

int RichText::texlen() const	// icons count as 1 glyph
{
	int runl = 0;

	for(auto i=m_part.begin(); i!=m_part.end(); i++)
		runl += i->texlen();

	return runl;
}

int RichText::rawlen() const	// icon tag length is counted
{
	int runl = 0;

	for(auto i=m_part.begin(); i!=m_part.end(); i++)
		runl += i->rawlen();

	return runl;
}

RichText::RichText()
{
}

RichText RichText::pwver() const	//asterisk-mask password string
{
	int len = 0;

	for(auto i=m_part.begin(); i!=m_part.end(); i++)
		len += i->texlen();

	char* pwcstr = new char[len+1];

	for(int i=0; i<len; i++)
		pwcstr[i] = '*';

	pwcstr[len] = '\0';
	
	RichTextP pwstrp(pwcstr);
	delete pwcstr;

	RichText pwstr;
	pwstr.m_part.push_back(pwstrp);

	return pwstr;
}

#ifdef USTR_DEBUG
int parsedepth = 0;
#endif

RichText ParseTags(RichText original, int* caret)
{
	RichText parsed;
	int currplace = 0;

	bool changed = false;
	
#ifdef USTR_DEBUG
	parsedepth ++;

	g_log<<"ParseTags #"<<parsedepth<<endl;
	g_log.flush();

	//if(parsedepth > 10)
	//	return original;
#endif

	for(auto i=original.m_part.begin(); i!=original.m_part.end(); i++)
	{
		if(i->m_type == RICHTEXT_TEXT)
		{
			bool foundtag = false;

			for(int j=0; j<ICONS; j++)
			{
				Icon* icon = &g_icon[j];
				int firstof = i->m_text.firstof(icon->m_tag);

				if(firstof < 0)
					continue;

#ifdef USTR_DEBUG
				g_log<<"ParseTags found tag \""<<icon->m_tag.rawstr()<<"\" in \""<<i->m_text.rawstr()<<"\""<<endl;
				g_log.flush();
#endif

				if(firstof > 0)
				{
					RichTextP before = i->substr(0, firstof);

#ifdef USTR_DEBUG
					g_log<<"ParseTags before str at "<<firstof<<" \""<<before.m_text.rawstr()<<"\""<<endl;
					g_log.flush();
#endif

					parsed = parsed + RichText(before);
					
#ifdef USTR_DEBUG
					g_log<<"\tparsed now = \""<<parsed.rawstr()<<"\""<<endl;
					g_log.flush();
#endif
				}

				RichTextP iconp(RICHTEXT_ICON, j);
				parsed = parsed + RichText(iconp);

				int taglen = icon->m_tag.m_length;
				int partlen =  i->m_text.m_length;

#ifdef USTR_DEBUG
				g_log<<"\tparsed now = \""<<parsed.rawstr()<<"\""<<endl;
				g_log.flush();
#endif

				if(firstof+taglen < partlen)
				{
					RichTextP after = i->substr(firstof+taglen, partlen-(firstof+taglen));

#ifdef USTR_DEBUG
					g_log<<"ParseTags after str at "<<(firstof+taglen)<<" \""<<after.m_text.rawstr()<<"\""<<endl;
					g_log.flush();
#endif

					parsed = parsed + RichText(after);

#ifdef USTR_DEBUG
					g_log<<"\tparsed now = \""<<parsed.rawstr()<<"\""<<endl;
					g_log.flush();
#endif
				}

				if(caret != NULL)
				{
					if(currplace+firstof < *caret)
					{
						*caret -= taglen-1;
						currplace += partlen-taglen+1;
					}
				}

				foundtag = true;
				changed = true;

				break;
			}

			if(!foundtag)
			{
				parsed = parsed + *i;
			}
		}
		else
		{
			parsed = parsed + *i;
		}

		if(!changed && caret != NULL)
			currplace += i->texlen();
	}

	if(!changed)
	{
#ifdef USTR_DEBUG
		g_log<<"ParseTags final = "<<original.rawstr()<<endl;
		g_log.flush();
#endif
		return original;
	}

	return ParseTags(parsed, caret);
}