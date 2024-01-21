precision mediump float;

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 u_light_vp;

// Output vertex attributes (to fragment shader)
varying vec2 shadow_uv;
varying vec2 fragTexCoord;
varying vec4 fragColor;

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

