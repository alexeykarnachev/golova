in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main() {
    vec2 uv = fragTexCoord;

    vec4 tex_color = texture(texture0, fragTexCoord);
    if (tex_color.a < 0.01) discard;

    finalColor = tex_color * colDiffuse;
}

