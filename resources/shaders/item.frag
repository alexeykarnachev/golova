#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform ivec2 atlas_grid_size;
uniform int item_id;
uniform int is_picking;

out vec4 finalColor;

void main() {
    if (is_picking == 1) {
        finalColor = vec4(float(item_id) / 255.0, 0.0, 0.0, 1.0);
    } else {
        vec2 cell_size = 1.0 / vec2(atlas_grid_size);
        int row = item_id / atlas_grid_size.y;
        int col = item_id % atlas_grid_size.y;
        vec2 offset = vec2(col * cell_size.x, row * cell_size.y);
        vec2 uv = fragTexCoord * cell_size + offset;
        vec4 tex_color = texture(texture0, uv);
        if (tex_color.a < 0.01) discard;

        finalColor = tex_color * colDiffuse;
    }
}

