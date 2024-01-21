precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

void main() {
    vec2 uv = fragTexCoord;

    vec4 tex_color = texture2D(texture0, fragTexCoord);
    if (tex_color.a < 0.01) discard;

    gl_FragColor = tex_color * colDiffuse;
}

