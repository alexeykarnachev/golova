#version 330

in vec2 shadow_uv;
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int u_with_shadows;

out vec4 finalColor;

void main() {
    bool is_shadow = false;
    bool with_shadows = u_with_shadows == 1;

    if (with_shadows) {
        vec4 shadow = texture(texture0, shadow_uv);
        is_shadow = shadow.a > 0.1;
    }

    if (is_shadow) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        finalColor = vec4(0.8, 0.8, 0.8, 1.0);
    }
}

