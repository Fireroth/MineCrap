#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aFaceID;

out vec2 TexCoord;
out float FaceID;
out vec3 WorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

float pi = 3.1415926535;

void main()
{
    vec3 animatedPos = aPos;
    // Formula taken from WSAL Evan --> https://github.com/EvanatorM/ScuffedMinecraft
    animatedPos.y += (sin(aPos.x * pi / 2.0 + time) + sin(aPos.z * pi / 2.0 + time * 1.5)) * 0.04;

    vec4 worldPosition = model * vec4(animatedPos, 1.0);
    gl_Position = projection * view * worldPosition;
    TexCoord = aTexCoord;
    FaceID = aFaceID;
    WorldPos = worldPosition.xyz;
}