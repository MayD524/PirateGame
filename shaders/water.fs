// shaders/water.fs
#version 330

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoord;

out vec4 finalColor;

// Light parameters
uniform vec3 lightDir = vec3(0.0, -1.0, 0.0);
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);

// Water color
uniform vec3 waterColor = vec3(0.0, 0.3, 0.5);

void main()
{
    // Simple diffuse lighting
    float diff = max(dot(normalize(fragNormal), normalize(lightDir)), 0.0);
    vec3 diffuse = diff * lightColor;

    // Combine color
    vec3 color = waterColor * diffuse;

    finalColor = vec4(color, 1.0);
}
