#version 330 core
in vec2 TexCoord;
in float FaceID;
in vec3 WorldPos;
out vec4 FragColor;

uniform sampler2D atlas;
uniform vec3 fogColor;
uniform vec3 cameraPos;
uniform float fogStartDistance;
uniform float fogDensity;

void main()
{
    vec4 texColor = texture(atlas, TexCoord);

    float brightness = 1.0;

    int faceIndex = int(FaceID + 0.5);
    
    switch(faceIndex) {
        case 0: brightness = 0.90; break; // Front
        case 1: brightness = 0.90; break; // Back
        case 2: brightness = 0.80; break; // Left
        case 3: brightness = 0.80; break; // Right
        case 4: brightness = 1.00; break; // Top
        case 5: brightness = 0.75; break; // Bottom
    }

    vec4 baseColor = vec4(texColor.rgb * brightness, texColor.a);

    float distance = length(WorldPos - cameraPos);
    float adjustedDistance = max(0.0, distance - fogStartDistance);
    float fogFactor = exp(-fogDensity * adjustedDistance);

    vec3 finalColor = mix(fogColor, baseColor.rgb, fogFactor);
    FragColor = vec4(finalColor, baseColor.a);
}