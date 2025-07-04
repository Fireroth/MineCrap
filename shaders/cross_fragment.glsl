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

    if (texColor.a == 0.0)
        discard;

    vec4 baseColor = vec4(texColor.rgb, texColor.a);

    float distance = length(WorldPos - cameraPos);
    float adjustedDistance = max(0.0, distance - fogStartDistance);
    float fogFactor = exp(-fogDensity * adjustedDistance);

    vec3 finalColor = mix(fogColor, baseColor.rgb, fogFactor);
    FragColor = vec4(finalColor, baseColor.a);
}