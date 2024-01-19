in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform int u_with_blur;
out vec4 finalColor;

void main() {
    vec2 uv = fragTexCoord;

    vec3 tex_color;
    if (u_with_blur == 1) {
        tex_color = sample_texture(texture0, uv, 87, 16).rgb * 0.4;
    } else {
        tex_color = texture(texture0, uv).rgb;
    }

    finalColor = vec4(tex_color, 1.0);
}
