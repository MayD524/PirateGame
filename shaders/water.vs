// shaders/water.vs
#version 330

// Input vertex attributes
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;

// Output to fragment shader
out vec3 fragNormal;
out vec3 fragPosition;
out vec2 fragTexCoord;

// Uniforms
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragPosition = vertexPosition;
    fragNormal = vertexNormal;
    fragTexCoord = vertexTexCoord;
    gl_Position = projection * view * vec4(vertexPosition, 1.0);
}
