
#version 150

in vec4 position;

in vec2 texCoordIn0;
out vec2 texCoordOut0;

uniform float mapminz;
uniform float mapmaxz;
uniform float mapminx;
uniform float mapmaxx;
uniform float mapminy;
uniform float mapmaxy;

uniform float width;
uniform float height;

void main(void)
{
	float maprangex = mapmaxx - mapminx;
	float maprangez = mapmaxz - mapminz;
	float maprangey = mapmaxy - mapminy;

	gl_Position = vec4((position.x-mapminx) * 2.0 / maprangex - 1.0,
		(position.z-mapminz) * -2.0 / maprangez + 1.0,
		1.0 - (position.y-mapminy) / maprangey, 
		1.0);

	//texCoordOut0 = texCoordIn0;
	texCoordOut0.x = (position.x - mapminx) / (mapmaxx - mapminx);
	texCoordOut0.y = 1.0 - (position.z - mapminz) / (mapmaxz - mapminz);
}
