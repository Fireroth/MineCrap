#version 330 core
in vec2 TexCoord;
in float FaceID;
out vec4 FragColor;

uniform sampler2D atlas;

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

    FragColor = vec4(texColor.rgb * brightness, texColor.a);
}