in vec2 shadow_uv;
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform bool u_with_shadows;

out vec4 finalColor;

void main() {
    float shadow = 0.0;
    if (u_with_shadows) {
        shadow = blur_texture(texture0, shadow_uv).a;
    }

    vec2 uv = fragTexCoord;
    vec3 color;

    float r = 0.1;
    float d;
    if (uv.x <= r && uv.y <= r) {
        d = distance(vec2(r, r), uv) / r;
    } else if (uv.x >= 1.0 - r && uv.y <= r) {
        d = distance(vec2(1.0 - r, r), uv) / r;
    } else if (uv.x <= r && uv.y >= 1.0 - r) {
        d = distance(vec2(r, 1.0 - r), uv) / r;
    } else if (uv.x >= 1.0 - r && uv.y >= 1.0 - r) {
        d = distance(vec2(1.0 - r, 1.0 - r), uv) / r;
    } else if (uv.x <= r) {
        d = 1.0 - (uv.x / r);
    } else if (uv.x >= 1.0 - r) {
        d = 1.0 - (1.0 - uv.x) / r;
    } else if (uv.y <= r) {
        d = 1.0 - (uv.y / r);
    } else if (uv.y >= 1.0 - r) {
        d = 1.0 - (1.0 - uv.y) / r;
    } else {
        d = 0.0;
    }

    color = vec3(1.0, 1.0, 1.0) * (1.0 - d);
    color *= (1.0 - shadow * 0.2);
    finalColor = vec4(color, 0.4);
}

