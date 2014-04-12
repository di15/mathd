

#include "../platform.h"
#include "../window.h"
#include "../utils.h"
#include "../texture.h"

void SaveScreenshot()
{
	LoadedTex screenshot;
	screenshot.channels = 3;
	screenshot.sizeX = g_width;
	screenshot.sizeY = g_height;
	screenshot.data = (unsigned char*)malloc( sizeof(unsigned char) * g_width * g_height * 3 );

	memset(screenshot.data, 0, g_width * g_height * 3);

	glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE, screenshot.data);

	FlipImage(&screenshot);

	char relative[256];
	string datetime = FileDateTime();
	sprintf(relative, "screenshots/%s.jpg", datetime.c_str());
	char fullpath[MAX_PATH+1];
	FullPath(relative, fullpath);

	g_log<<"Writing screenshot "<<fullpath<<endl;
	g_log.flush();

	SaveJPEG(fullpath, &screenshot, 0.50);

	free(screenshot.data);
}