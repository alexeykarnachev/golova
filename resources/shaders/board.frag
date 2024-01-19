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
        shadow = sample_texture(texture0, shadow_uv, 87, 6).a;
    }

    vec3 color = vec3(0.8, 0.8, 0.8);
    color *= (1.0 - shadow);
    finalColor = vec4(color, 1.0);
}

