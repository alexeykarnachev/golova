#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int u_is_hot;

out vec4 finalColor;

void main() {
    vec4 tex_color;

    vec2 uv = fragTexCoord;
    bool is_border = min(uv.x, uv.y) <= 0.05 || max(uv.x, uv.y) >= 0.95;
    bool is_hot = u_is_hot == 1;
    if (is_hot && is_border) {
        tex_color = vec4(1.0, 1.0, 0.0, 1.0);
    } else {
        tex_color = texture(texture0, uv);
    }

    if (tex_color.a < 0.01) discard;
    finalColor = tex_color * colDiffuse;
}

