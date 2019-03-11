#version 330
// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform int light_up;
uniform int tree_species;


uniform float time;


// Output color
layout(location = 0) out  vec4 color;


void main()
{

	float height = 1.0 - texcoord.y;
	float offset = pow(height, 2.5);
	float speed = 0.6;
	float bendFactor = 0.1;
	//multiply by sin since it gives us nice bending
	offset *= (sin(time * speed) * bendFactor);
	
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x +offset, texcoord.y));

}
