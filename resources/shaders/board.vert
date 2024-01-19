#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 u_light_vp;

// Output vertex attributes (to fragment shader)
out vec2 shadow_uv;
out vec2 fragTexCoord;
out vec4 fragColor;

// NOTE: Add here your custom variables

void main() {
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // Get shadowmap gl position
    vec4 shadow_gl_position = matModel * u_light_vp * vec4(vertexPosition, 1.0);
    vec2 shadow_screen_position = shadow_gl_position.xy / shadow_gl_position.w;
    shadow_uv = (shadow_screen_position + 1.0) / 2.0;

    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
