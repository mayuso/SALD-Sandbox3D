#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 DirectionalLightSpacePos;

uniform mat4 model;
uniform mat4 normalMatrix; // transpose(inverse(model)), set per object
uniform mat4 projection;
uniform mat4 view;
uniform mat4 directionalLightTransform;

void main()
{
	vec4 worldPos = model * vec4(pos, 1.0);

	gl_Position = projection * view * worldPos;
	DirectionalLightSpacePos = directionalLightTransform * worldPos;

	TexCoord = tex;

	Normal = mat3(normalMatrix) * norm;

	FragPos = worldPos.xyz;
}
