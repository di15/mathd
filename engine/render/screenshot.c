#include "../platform.h"
#include "../window.h"
#include "../utils.h"
#include "../texture.h"
#include "../sim/player.h"

void SaveScreenshot()
{
	Player* py = &g_player[g_localP];

	LoadedTex screenshot;
	screenshot.channels = 3;
	screenshot.sizeX = g_width;
	screenshot.sizeY = g_height;
	screenshot.data = (unsigned char*)malloc( sizeof(unsigned char) * g_width * g_height * 3 );

	if(!screenshot.data)
	{
		OutOfMem(__FILE__, __LINE__);
		return;
	}

	memset(screenshot.data, 0, g_width * g_height * 3);

	glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE, screenshot.data);

	FlipImage(&screenshot);

	char relative[256];
	std::string datetime = FileDateTime();
	//sprintf(relative, "screenshots/%s.jpg", datetime.c_str());
	sprintf(relative, "screenshots/%s.png", datetime.c_str());
	char fullpath[MAX_PATH+1];
	FullPath(relative, fullpath);

	g_log<<"Writing screenshot "<<fullpath<<std::endl;
	g_log.flush();

	//SaveJPEG(fullpath, &screenshot, 0.9f);
	SavePNG(fullpath, &screenshot);

	//free(screenshot.data);
}
