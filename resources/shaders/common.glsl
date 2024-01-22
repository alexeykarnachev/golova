float offset[3] = float[](0.0, 1.38, 3.23);
float weight[3] = float[](0.23, 0.32, 0.07);

vec4 blur_texture(sampler2D tex, vec2 uv) {
    vec2 size = vec2(textureSize(tex, 0));

    vec4 color = texture(tex, uv) * weight[0];

    for (int i = 1; i < 3; i++) {
        color += texture(tex, uv + vec2(offset[i]) / size.x) * weight[i];
        color += texture(tex, uv - vec2(offset[i]) / size.x) * weight[i];
    }

    return color;
}

