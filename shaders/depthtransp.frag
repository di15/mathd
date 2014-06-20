
#version 150



uniform sampler2D texture0;

in vec2 texCoordOut0;

out vec4 outfrag;

void main(void)
{
	//gl_FragColor = color * texture2D(texture0, texCoordOut0);
	//gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	//gl_FragColor = vec4(fragCoordZ, fragCoordZ, fragCoordZ, 1.0);
	//gl_FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);


	vec4 texel0 = texture2D(texture0, texCoordOut0);

	if(texel0.w < 0.5)
		discard;

	//gl_FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, texel0.w);
	outfrag = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
/*
	gl_FragColor.x = gl_FragCoord.z * 1.05;
	gl_FragColor.y = gl_FragCoord.z * 1.05;
	gl_FragColor.z = gl_FragCoord.z * 1.05;
	gl_FragColor.w = 1;
*/
}
