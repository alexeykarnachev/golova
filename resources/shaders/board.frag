#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main() {
    vec2 uv = fragTexCoord;
    finalColor = vec4(0.45, 0.45, 0.5, 1.0);
}

