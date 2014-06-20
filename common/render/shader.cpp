






#include "../utils.h"
#include "shader.h"
#include "../platform.h"
#include "../window.h"
#include "../sim/player.h"

Shader g_shader[SHADERS];
int g_curS = 0;

GLint Shader::GetUniform(const char* strVariable)
{
	if(!m_hProgramObject)
		return -1;

	return glGetUniformLocation(m_hProgramObject, strVariable);
}

GLint Shader::GetAttrib(const char* strVariable)
{
	if(!m_hProgramObject)
		return -1;

	return glGetAttribLocation(m_hProgramObject, strVariable);
}

void Shader::MapUniform(int slot, const char* variable)
{
	m_slot[slot] = GetUniform(variable);
	//g_log<<"\tmap uniform "<<variable<<" = "<<(int)m_slot[slot]<<endl;
}

void Shader::MapAttrib(int slot, const char* variable)
{
	m_slot[slot] = GetAttrib(variable);
	//g_log<<"\tmap attrib "<<variable<<" = "<<(int)m_slot[slot]<<endl;
}

void GetGLVersion(int* major, int* minor)
{
	// for all versions
	char* ver = (char*)glGetString(GL_VERSION); // ver = "3.2.0"
	
	char vermaj[4];

	for(int i=0; i<4; i++)
	{
		if(ver[i] != '.')
			vermaj[i] = ver[i];
		else
			vermaj[i] = '\0';
	}

	//*major = ver[0] - '0';
	*major = StrToInt(vermaj);
	if( *major >= 3)
	{
		// for GL 3.x
		glGetIntegerv(GL_MAJOR_VERSION, major); // major = 3
		glGetIntegerv(GL_MINOR_VERSION, minor); // minor = 2
	}
	else
	{
		*minor = ver[2] - '0';
	}

	// GLSL
	ver = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION); // ver = "1.50 NVIDIA via Cg compiler"
}

void InitGLSL()
{
	//strstr("abab", "ba");

	GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
        ErrorMessage("Error initializing GLEW!", (const char*)glewGetErrorString( glewError ));
        return;
    }
	
	g_log<<"Renderer: "<<(char*)glGetString(GL_RENDERER)<<endl;
	g_log<<"GL_VERSION = "<<(char*)glGetString(GL_VERSION)<<endl;

#if 0
    //Make sure OpenGL 2.1 is supported
    if( !GLEW_VERSION_2_1 )
    {
        ErrorMessage("Error", "OpenGL 2.1 not supported!\n" );
        return;
    }
#else
    if( !GLEW_VERSION_3_2 )
    {
        ErrorMessage("Error", "OpenGL 3.2 not supported!\n" );
        return;
    }
#endif

#if 1
	char *szGLExtensions = (char*)glGetString(GL_EXTENSIONS);

	g_log<<szGLExtensions<<endl;
	g_log.flush();

	if(!strstr(szGLExtensions, "GL_ARB_shader_objects"))
	{
		ErrorMessage("Error", "GL_ARB_shader_objects extension not supported!");
		return;
	}

	if(!strstr(szGLExtensions, "GL_ARB_shading_language_100"))
	{
		ErrorMessage("Error", "GL_ARB_shading_language_100 extension not supported!");
		return;
	}
#endif

	int major, minor;
	GetGLVersion(&major, &minor);

	if(major < 3 || ( major == 3 && minor < 2 ))
	{
		ErrorMessage("Error", "OpenGL 3.2 is not supported!");
	}
	
	LoadShader(SHADER_ORTHO, "shaders/ortho.vert", "shaders/ortho.frag");
	LoadShader(SHADER_COLOR2D, "shaders/color2d.vert", "shaders/color2d.frag");
	LoadShader(SHADER_COLOR3D, "shaders/color3d.vert", "shaders/color3d.frag");
	LoadShader(SHADER_BILLBOARD, "shaders/billboard.vert", "shaders/billboard.frag");
	LoadShader(SHADER_DEPTH, "shaders/depth.vert", "shaders/depth.frag");
	LoadShader(SHADER_DEPTHTRANSP, "shaders/depth.vert", "shaders/depthtransp.frag");
	LoadShader(SHADER_OWNED, "shaders/owned.vert", "shaders/owned.frag");
	//LoadShader(SHADER_MODEL, "shaders/building.vert", "shaders/building.frag");
	//LoadShader(SHADER_MAPTILES, "shaders/maptilesmegatex.vert", "shaders/maptilesmegatex.frag");
	LoadShader(SHADER_MAPTILES, "shaders/maptiles.vert", "shaders/maptiles.frag");
	LoadShader(SHADER_WATER, "shaders/water.vert", "shaders/water.frag");
	LoadShader(SHADER_FOLIAGE, "shaders/foliage.vert", "shaders/foliage.frag");
	LoadShader(SHADER_MAPTILESMM, "shaders/maptilesmm2.vert", "shaders/maptilesmm2.frag");
	LoadShader(SHADER_WATERMM, "shaders/watermm.vert", "shaders/watermm.frag");
	LoadShader(SHADER_MAPTILESPREREND, "shaders/maptilesprerend.vert", "shaders/maptilesprerend.frag");
	LoadShader(SHADER_RIM, "shaders/rim.vert", "shaders/rim.frag");
	LoadShader(SHADER_SKYBOX, "shaders/skybox.vert", "shaders/skybox.frag");
	LoadShader(SHADER_UNIT, "shaders/unit.vert", "shaders/unit.frag");
}

string LoadTextFile(char* strFile)
{
	ifstream fin(strFile);

	if(!fin)
	{
		g_log<<"Failed to load file "<<strFile<<endl;
		return "";
	}

	string strLine = "";
	string strText = "";

	while(getline(fin, strLine))
		strText = strText + "\n" + strLine;

	fin.close();

	return strText;
}

void LoadShader(int shader, char* strVertex, char* strFragment)
{
	Shader* s = &g_shader[shader];
	string strVShader, strFShader;

	if(s->m_hVertexShader || s->m_hFragmentShader || s->m_hProgramObject)
		s->release();

	s->m_hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	s->m_hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	strVShader = LoadTextFile(strVertex);
	strFShader = LoadTextFile(strFragment);

	const char *szVShader = strVShader.c_str();
	const char *szFShader = strFShader.c_str();

	glShaderSource(s->m_hVertexShader, 1, &szVShader, NULL);
	glShaderSource(s->m_hFragmentShader, 1, &szFShader, NULL);
	
	glCompileShader(s->m_hVertexShader);
	GLint logLength;
	glGetShaderiv(s->m_hVertexShader, GL_INFO_LOG_LENGTH, &logLength);
    if(logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);

		if(!log)
		{
			OutOfMem(__FILE__, __LINE__);
			return;
		}

        glGetShaderInfoLog(s->m_hVertexShader, logLength, &logLength, log);
        g_log<<"Shader "<<strVertex<<" compile log: "<<endl<<log<<endl;
        free(log);
    }

	glCompileShader(s->m_hFragmentShader);
	glGetShaderiv(s->m_hFragmentShader, GL_INFO_LOG_LENGTH, &logLength);
    if(logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
		
		if(!log)
		{
			OutOfMem(__FILE__, __LINE__);
			return;
		}

        glGetShaderInfoLog(s->m_hFragmentShader, logLength, &logLength, log);
        g_log<<"Shader "<<strFragment<<" compile log: "<<endl<<log<<endl;
        free(log);
    }

	s->m_hProgramObject = glCreateProgramObjectARB();
	glAttachShader(s->m_hProgramObject, s->m_hVertexShader);
	glAttachShader(s->m_hProgramObject, s->m_hFragmentShader);
	glLinkProgram(s->m_hProgramObject);

	//glUseProgramObjectARB(s->m_hProgramObject);

	//g_log<<"shader "<<strVertex<<","<<strFragment<<endl;
	
	g_log<<"Program "<<strVertex<<" / "<<strFragment<<" :";

	glGetProgramiv(s->m_hProgramObject, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		GLchar *log = (GLchar *)malloc(logLength);
		glGetProgramInfoLog(s->m_hProgramObject, logLength, &logLength, log);
		g_log<<"Program link log:"<<endl<<log<<endl;
		free(log);
	}

	GLint status;
	glGetProgramiv(s->m_hProgramObject, GL_LINK_STATUS, &status);
	if (status == 0)
	{
		g_log<<"link status 0"<<endl;
	}
	else
	{
		g_log<<"link status ok"<<endl;
	}

	g_log<<endl<<endl;
	
	
    s->MapAttrib(SSLOT_POSITION, "position");
    s->MapAttrib(SSLOT_NORMAL, "normalIn");
    s->MapAttrib(SSLOT_TEXCOORD0, "texCoordIn0");
    s->MapAttrib(SSLOT_TEXCOORD1, "texCoordIn1");
	s->MapAttrib(SSLOT_VERTCOLORS, "vertcolors");
    //s->MapAttrib(SSLOT_TANGENT, "tangent");
	s->MapUniform(SSLOT_SHADOWMAP, "shadowmap");
	s->MapUniform(SSLOT_LIGHTMATRIX, "lightMatrix");
	s->MapUniform(SSLOT_LIGHTPOS, "lightPos");
	//s->MapUniform(SSLOT_LIGHTDIR, "lightDir");
	s->MapUniform(SSLOT_TEXTURE0, "texture0");
	s->MapUniform(SSLOT_TEXTURE1, "texture1");
	s->MapUniform(SSLOT_TEXTURE2, "texture2");
	s->MapUniform(SSLOT_TEXTURE3, "texture3");
	s->MapUniform(SSLOT_NORMALMAP, "normalmap");
	s->MapUniform(SSLOT_SPECULARMAP, "specularmap");
    s->MapUniform(SSLOT_PROJECTION, "projection");
    s->MapUniform(SSLOT_MODELMAT, "model");
    s->MapUniform(SSLOT_VIEWMAT, "view");
    s->MapUniform(SSLOT_MVPMAT, "mvpmat");
	//s->MapUniform(SSLOT_NORMALMAT, "normalMat");
	//s->MapUniform(SSLOT_INVMODLVIEWMAT, "invModelView");
    s->MapUniform(SSLOT_COLOR, "color");
    s->MapUniform(SSLOT_OWNCOLOR, "owncolor");
    s->MapUniform(SSLOT_WIDTH, "width");
    s->MapUniform(SSLOT_HEIGHT, "height");
    s->MapUniform(SSLOT_MIND, "mind");
    s->MapUniform(SSLOT_MAXD, "maxd");
    s->MapUniform(SSLOT_CAMERAPOS, "cameraPos");
    s->MapUniform(SSLOT_SCALE, "scale");
	s->MapUniform(SSLOT_MAXELEV, "maxelev");
	s->MapUniform(SSLOT_SANDONLYMAXY, "sandonlymaxy");
	s->MapUniform(SSLOT_SANDGRASSMAXY, "sandgrassmaxy");
	s->MapUniform(SSLOT_GRASSONLYMAXY, "grassonlymaxy");
	s->MapUniform(SSLOT_GRASSROCKMAXY, "dirtrockmaxy");
	s->MapUniform(SSLOT_SANDTEX, "sandtex");
	s->MapUniform(SSLOT_GRASSTEX, "grasstex");
	s->MapUniform(SSLOT_SNOWTEX, "snowtex");
	s->MapUniform(SSLOT_ROCKTEX, "rocktex");
	s->MapUniform(SSLOT_ROCKNORMTEX, "rocknormtex");
	s->MapUniform(SSLOT_CRACKEDROCKTEX, "crackedrocktex");
	s->MapUniform(SSLOT_CRACKEDROCKNORMTEX, "crackedrocknormtex");
	s->MapUniform(SSLOT_SUNDIRECTION, "sundirection");
	s->MapUniform(SSLOT_GRADIENTTEX, "gradienttex");
	s->MapUniform(SSLOT_DETAILTEX, "detailtex");
	s->MapUniform(SSLOT_OWNERMAP, "ownermap");
	s->MapUniform(SSLOT_MAPMINZ, "mapminz");
	s->MapUniform(SSLOT_MAPMAXZ, "mapmaxz");
	s->MapUniform(SSLOT_MODELMATS, "modelmats");
	s->MapUniform(SSLOT_ONSWITCHES, "onswitches");
	s->MapUniform(SSLOT_MAPMINX, "mapminx");
	s->MapUniform(SSLOT_MAPMAXX, "mapmaxx");
	s->MapUniform(SSLOT_MAPMINY, "mapminy");
	s->MapUniform(SSLOT_MAPMAXY, "mapmaxy");
	s->MapUniform(SSLOT_WAVEPHASE, "wavephase");
}

void UseS(int shader)		
{	
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	g_curS = shader;
	
	Shader* s = &g_shader[g_curS];

	//glUseProgramObjectARB(g_shader[shader].m_hProgramObject); 
	glUseProgram(s->m_hProgramObject); 
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_POSITION] != -1)	glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_TEXCOORD0] != -1) glEnableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_NORMAL] != -1)	glEnableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	
	Player* py = &g_player[g_currP];

	glUniform1f(s->m_slot[SSLOT_MIND], MIN_DISTANCE);
	glUniform1f(s->m_slot[SSLOT_MAXD], MAX_DISTANCE / py->zoom);
}

void EndS()
{
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif

	if(g_curS < 0)
		return;

	Shader* s = &g_shader[g_curS];
	
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_POSITION] != -1)	glDisableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_TEXCOORD0] != -1) glDisableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_NORMAL] != -1)	glDisableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif

	glUseProgram(0); 

	g_curS = -1;
}

void Shader::release()
{
	if(m_hVertexShader)
	{
		glDetachObjectARB(m_hProgramObject, m_hVertexShader);
		glDeleteObjectARB(m_hVertexShader);
		m_hVertexShader = NULL;
	}

	if(m_hFragmentShader)
	{
		glDetachObjectARB(m_hProgramObject, m_hFragmentShader);
		glDeleteObjectARB(m_hFragmentShader);
		m_hFragmentShader = NULL;
	}

	if(m_hProgramObject)
	{
		glDeleteObjectARB(m_hProgramObject);
		m_hProgramObject = NULL;
	}
}

void TurnOffShader()
{
	glUseProgramObjectARB(0);
}

void ReleaseShaders()
{
	for(int i=0; i<SHADERS; i++)
		g_shader[i].release();
}

