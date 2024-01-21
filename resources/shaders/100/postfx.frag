precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0;
uniform int u_with_blur;

void main() {
    vec2 uv = fragTexCoord;

    vec4 tex_color;
    if (u_with_blur == 1) {
        tex_color = texture2D(texture0, uv) * 0.4;
    } else {
        tex_color = texture2D(texture0, uv);
    }

    tex_color.a = 1.0;
    gl_FragColor = tex_color;
}
