#version 330 core
//in vec4 vertex; // <vec2 pos, vec2 tex>
in vec3 in_position;
in vec2 in_texcoords;

out vec2 TexCoords;

uniform mat3 projection;
//uniform mat3 transform;
//uniform mat4 projection;

void main()
{
	TexCoords = in_texcoords;
    vec3 pos = projection * vec3(in_position.xy, 1.0);
    gl_Position = vec4(pos.xy, in_position.z, 1.0);
}  