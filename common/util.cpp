

#include "platform.h"

void StripPathExtension(const char* n, char* o)
{
	string s0(n);
	size_t sep = s0.find_last_of("\\/");
	string s1;

    if (sep != std::string::npos)
        s1 = s0.substr(sep + 1, s0.size() - sep - 1);
	else
		s1 = s0;

	size_t dot = s1.find_last_of(".");
	string s2;

	if (dot != std::string::npos)
		s2 = s1.substr(0, dot);
	else
		s2 = s1;

	strcpy(o, s2.c_str());
}

string MakePathRelative(const char* full)
{
	string full2(full);
	string exepath = ExePath();

	//g_log<<"exepath: "<<exepath<<endl;
	//g_log<<"fulpath: "<<full<<endl;

	string::size_type pos = full2.find(exepath.c_str(), 0);

	if(pos == string::npos)
		return full2;

	//g_log<<"posposp: "<<pos<<endl;

	string sub = string( full2 ).substr(strlen(exepath.c_str())+1, strlen(full)-strlen(exepath.c_str())-1);

	//g_log<<"subpath: "<<sub<<endl;

    return sub;
}

string ExePath() 
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    string::size_type pos = string( buffer ).find_last_of( "\\/" );
    return string( buffer ).substr( 0, pos);
}