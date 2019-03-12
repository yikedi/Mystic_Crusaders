#version 330
// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec4 fcolor;
uniform int light_up;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = fcolor * texture(sampler0, vec2(texcoord.x, texcoord.y));
	if (light_up == 1)
	{
		// 0.6 is just to make it not too strong
		color.xyz += 0.5 * vec3(1.0, 1.0, 0.0);
	}
}
