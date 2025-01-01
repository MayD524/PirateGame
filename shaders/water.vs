// water.vs

#version 330 core

// Input vertex data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

// Output data to the fragment shader
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

// Wave parameters
uniform float waveSpeed;
uniform float waveScale;
uniform float waveHeight;
uniform float wavelength;
uniform float amplitude;
uniform vec2 direction;

void main()
{
    // Calculate wave displacement
    float wave = sin(dot(aPosition.xz, direction) / wavelength + time * waveSpeed) * amplitude;
    vec3 displacedPos = aPosition + vec3(0.0, wave, 0.0);

    // Pass texture coordinates to fragment shader
    TexCoord = aTexCoord;

    // Pass transformed position to fragment shader
    FragPos = vec3(model * vec4(displacedPos, 1.0));

    // Calculate normals using partial derivatives
    float delta = 0.1;
    float waveL = sin(dot((aPosition.xz + vec2(-delta, 0.0)), direction) / wavelength + time * waveSpeed) * amplitude;
    float waveR = sin(dot((aPosition.xz + vec2(delta, 0.0)), direction) / wavelength + time * waveSpeed) * amplitude;
    float waveD = sin(dot((aPosition.xz + vec2(0.0, -delta)), direction) / wavelength + time * waveSpeed) * amplitude;
    float waveU = sin(dot((aPosition.xz + vec2(0.0, delta)), direction) / wavelength + time * waveSpeed) * amplitude;

    vec3 normal = normalize(vec3(waveL - waveR, 2.0 * delta, waveD - waveU));
    Normal = normal;

    // Final vertex position
    gl_Position = projection * view * model * vec4(displacedPos, 1.0);
}
