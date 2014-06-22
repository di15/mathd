#version 130


uniform vec4 color;

//varying vec3 normalOut;
in vec2 texCoordOut0;
uniform sampler2D texture0;

out vec4 outfrag;
in float logz;

void main(void)
{
	vec4 texel = texture(texture0, texCoordOut0);

	//if(texel.w < 0.5)
	// discard;

	//gl_FragColor = color * vec4(texel.xyz, 1);

	//gl_FragDepth = logz;

	outfrag = color * texel;
	//outfrag = vec4(1,0,0,1);
	//outfrag.x = 1;
	//outfrag.w = 1;
}
