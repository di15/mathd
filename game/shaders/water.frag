
#version 130


uniform vec4 color;

uniform sampler2D texture0;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform sampler2D texture3;
uniform sampler2D specularmap;
uniform sampler2D normalmap;
uniform sampler2D shadowmap;

in vec4 lpos;
in vec3 light_vec;
in vec3 light_dir;

in vec2 texCoordOut0;
in vec2 phasetexCoordOut;

in vec3 normalOut;

in vec3 eyevec;
//varying float elevtransp;
uniform float maxelev;
//varying float elevy;

uniform sampler2D gradienttex;
uniform sampler2D detailtex;

in float logz;

out vec4 outfrag;

void main (void)
{
	//if(elevy > maxelev)
	//	discard;

	gl_FragDepth = logz;

	//vec3 bump = normalize( texture(normalmap, phasetexCoordOut/10).xyz * 2.0 - 1.0);

	vec3 lvec = normalize(light_vec);
	float diffuse = max(dot(lvec, normalOut), 0.0) + 0.50;

	//float distSqr = dot(light_vec, light_vec);
	//vec3 lvec = light_vec * inversesqrt(distSqr);
	//float diffuse = max( dot(lvec, bump), 0.0 ) * 0.75 + 0.50;

	//vec3 vvec = normalize(eyevec);
	//float specular = pow(clamp(dot(reflect(-lvec, bump), vvec), 0.0, 1.0), 0.7 );
	//vec3 vspecular = vec3(0,0,0);
	//vec3 vspecular = texture(specularmap, phasetexCoordOut/10).xyz * specular;

	vec4 gradtxl = texture(gradienttex, texCoordOut0 / 20);
	vec4 dettxl = texture(detailtex, phasetexCoordOut/10);

	vec4 stexel = vec4(gradtxl.xyz * dettxl.xyz, gradtxl.w * dettxl.w);
	//vec4 stexel = vec4(dettxl.xyz, dettxl.w);

	//float alph = color.w * texel0.w * elevtransp;
	float alph = color.w * stexel.w;

	outfrag = vec4(color.xyz * stexel.xyz * diffuse, alph);
	//gl_FragColor = vec4(1,0,0,1);
	//gl_FragColor = texel0;
	//gl_FragColor = vec4(light_vec, color.w * texel0.w);	
	//gl_FragColor = vec4(vspecular, color.w * texel0.w);
	//gl_FragColor = vec4(eyevec, color.w * texel0.w);
}

