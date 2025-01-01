// water.fs

#version 330 core

// Input data from vertex shader
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

// Output color
out vec4 FragColor;

// Uniforms
uniform vec3 viewPos;
uniform vec4 waterColor;
uniform vec4 specularColor;
uniform float shininess;

// Light properties
uniform vec3 lightPos;
uniform vec3 lightColor;

// Function to calculate reflection vector
vec3 calculateReflection(vec3 viewDir, vec3 normal)
{
    return reflect(-viewDir, normal);
}

void main()
{
    // Normalize interpolated normal
    vec3 N = normalize(Normal);

    // Calculate view direction
    vec3 V = normalize(viewPos - FragPos);

    // Calculate reflection vector
    vec3 R = calculateReflection(V, N);

    // Procedural reflection color (simple sky-like color based on reflection vector)
    vec3 reflection = vec3(0.2, 0.5, 0.7) * max(dot(R, vec3(0.0, 1.0, 0.0)), 0.0);

    // Procedural specular highlight
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular calculations
    vec3 reflectDir = reflect(-lightDir, N);
    float spec = pow(max(dot(V, reflectDir), 0.0), shininess);
    vec3 specular = specularColor.rgb * spec;

    // Combine lighting components
    vec3 lighting = diffuse + specular;

    // Combine with reflection
    vec3 color = mix(waterColor.rgb, reflection, 0.5) * lighting;

    // Final color with alpha
    FragColor = vec4(color, waterColor.a);
}
