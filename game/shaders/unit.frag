
#version 130


uniform vec4 color;
uniform vec4 owncolor;

uniform sampler2D texture0;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform sampler2D texture3;
uniform sampler2D shadowmap;
uniform sampler2D ownermap;

in vec4 lpos;
in vec3 light_vec;
in vec3 light_dir;

in vec2 texCoordOut0;

in vec3 normalOut;
in vec3 eyevec;


out vec4 outfrag;

void main (void)
{
	vec4 texel0 = texture(texture0, texCoordOut0);

	float alph = texel0.w;

	if(alph < 0.5)
		discard;

	alph = color.w;

	float distSqr = dot(light_vec, light_vec);
	vec3 lvec = light_vec * inversesqrt(distSqr);
	float diffuse = max( dot(lvec, normalOut), 0.0 ) * 0.75 + 0.50;
	//float diffuse = max( dot(lvec, bump), 0.0 ) * 0.75 + 0.50;

	//vec3 vvec = normalize(eyevec);
	//float specular = pow(clamp(dot(reflect(-lvec, bump), vvec), 0.0, 1.0), 0.7 );
	//float specular = pow(clamp(dot(reflect(-lvec, normalOut), vvec), 0.0, 1.0), 0.7 );
	//vec3 vspecular = vec3(0,0,0);
	//vec3 vspecular = vec3(0.2,0.2,0.2) * specular;

	vec4 owntexel = texture(ownermap, texCoordOut0);

	float alph1 = owntexel.w;
	vec4 stexel = vec4(texel0.xyz * (1.0 - alph1) + owncolor.xyz * alph1, 1.0);

	float minlight = diffuse;

	outfrag = vec4(color.xyz * stexel.xyz * minlight, alph);
	//gl_FragColor = vec4(color.xyz * stexel.xyz * shadow * diffuse, alph);
	//gl_FragColor = vec4(1,0,0,1);
	//gl_FragColor = texel0;
	//gl_FragColor = vec4(light_vec, color.w * texel0.w);	
	//gl_FragColor = vec4(vspecular, color.w * texel0.w);
	//gl_FragColor = vec4(eyevec, color.w * texel0.w);
}

