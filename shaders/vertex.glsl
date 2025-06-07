#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aFaceID;

out vec2 TexCoord;
out float FaceID;
out float fogFactor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float fogStartDistance;
uniform float fogDensity;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    FaceID = aFaceID;
    
    float distance = length(gl_Position.xyz);
    float adjustedDistance = max(0.0, distance - fogStartDistance);
    fogFactor = exp(-fogDensity * adjustedDistance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
}