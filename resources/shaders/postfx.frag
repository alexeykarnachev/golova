in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform int u_with_blur;
out vec4 finalColor;

void main() {
    vec2 uv = fragTexCoord;

    vec4 tex_color;
    if (u_with_blur == 1) {
        tex_color = blur_texture(texture0, uv) * 0.4;
    } else {
        tex_color = texture(texture0, uv);
    }

    tex_color.a = 1.0;
    finalColor = tex_color;
}
