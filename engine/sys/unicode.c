#include "unicode.h"
#include "../utils.h"
#include "utf8.h"

int ValidateUTF8(char unsigned *buff, int len)
{
	int x;
	for (x = 0; x < len; x++)
	{
		if ((unsigned char)(buff[x]) > 0xfd)
		{
			char msg[128];
			sprintf(msg, "Byte %i is invalid\n", x);
			InfoMess(msg, msg);
			return 0;
		}
	}
	return 1;
}

int UTF8Len(unsigned char ch)
{
	int l;
	unsigned char c = ch;
	c >>= 3;
	// 6 => 0x7e
	// 5 => 0x3e
	if (c == 0x1e)
	{
		l = 4;
	}
	else
	{
		c >>= 1;
		if (c == 0xe)
		{
			l = 3;
		}
		else
		{
			c >>= 1;
			if (c == 0x6)
			{
				l = 2;
			}
			else
			{
				l = 1;
			}
		}
	}
	return l;
}

int UTF32Len(const unsigned int* ustr)
{
	int i=0;

	for(; ustr[i]; i++)
		;

	return i;
}

unsigned char *ToUTF8(const unsigned int *unicode)
{
	int utf32len = UTF32Len(unicode);
	int utf8len = utf32len * 4;
	unsigned char* utf8 = new unsigned char [ utf8len + 1 ];
	utf8len = wchar_to_utf8(unicode, utf32len, (char*)utf8, utf8len, 0);
	utf8[utf8len] = 0;
	return utf8;
}

unsigned int *ToUTF32(const unsigned char *utf8)
{
	int utf8len = strlen((char*)utf8);
	unsigned int* utf32 = new unsigned int [ utf8len + 1 ];
	int utf32len = utf8_to_wchar((char*)utf8, utf8len, utf32, utf8len, 0);
	utf32[utf32len] = 0;
	return utf32;
}
