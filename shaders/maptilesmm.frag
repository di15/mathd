
#version 150

uniform vec4 color;

uniform sampler2D texture0;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform sampler2D texture3;
//uniform sampler2D specularmap;
//uniform sampler2D normalmap;
//uniform sampler2D shadowmap;

in vec4 lpos;
in vec3 light_vec;
in vec3 light_dir;

in vec2 texCoordOut0;

// 255 x 256 / 5.3 = 12316.98113207547169811320754717

out vec4 outfrag;

void main (void)
{
	vec4 grasstxl = texture(texture0, texCoordOut0);

	//gl_FragColor = vec4(stexel.xyz * minlight, 1);
	//gl_FragColor = vec4(color.xyz * stexel.xyz * minlight, alph);
	//gl_FragColor = vec4(color.xyz * stexel.xyz * shadow, alph);
	//gl_FragColor = vec4(color.xyz * stexel.xyz * shadow * diffuse, alph);
	//gl_FragColor = vec4(color.xyz * sanddettxl.xyz * shadow * diffuse, alph);
	//gl_FragColor = vec4(grassdettxl.xyz, 1.0);
	//gl_FragColor = vec4(grassgradtxl.rgb, 1.0);
	outfrag = grasstxl;
	//gl_FragColor = vec4(1,0,0,1);
	//gl_FragColor = texel0;
	//gl_FragColor = vec4(light_vec, color.w * texel0.w);	
	//gl_FragColor = vec4(vspecular, color.w * texel0.w);
	//gl_FragColor = vec4(eyevec, color.w * texel0.w);
	//gl_FragColor = vec4(totalalpha, totalalpha, totalalpha, 1);

	//if(gl_FragColor.x + gl_FragColor.y + gl_FragColor.z < 0.5)
	//	gl_FragColor = vec4(color.xyz * sandtxl.xyz * minlight, alph);
}