#version 330

in vec4 fragColor;
in vec3 fragPosition;

uniform vec3 cameraPosition;
uniform vec3 gizmoPosition;

out vec4 finalColor;

void main() {
    vec3 r = normalize(fragPosition - gizmoPosition);
    vec3 c = normalize(fragPosition - cameraPosition);
    if (dot(r, c) > 0.1) discard;
    finalColor = fragColor;
}

