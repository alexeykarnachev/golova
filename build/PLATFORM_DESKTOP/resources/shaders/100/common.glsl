precision mediump float;

vec4 blur_texture(sampler2D tex, vec2 uv) {
    return texture2D(tex, uv);
}

