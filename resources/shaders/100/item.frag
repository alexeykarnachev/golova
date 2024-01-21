precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec4 u_border_color;

void main() {
    vec4 tex_color;

    vec2 uv = fragTexCoord;
    bool is_border = min(uv.x, uv.y) <= 0.05 || max(uv.x, uv.y) >= 0.95;
    if (is_border && u_border_color.a > 0.0) {
        tex_color = u_border_color;
    } else {
        tex_color = texture2D(texture0, uv);
    }

    if (tex_color.a < 0.01) discard;
    gl_FragColor = tex_color * colDiffuse; 
}

