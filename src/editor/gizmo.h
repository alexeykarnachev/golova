#include <raylib.h>

#define GIZMO_SIZE 10.0f
#define GIZMO_ROT_HANDLE_THICKNESS 0.1f
#define GIZMO_ROT_HANDLE_DRAW_THICKNESS 4.0f

typedef enum GizmoState {
    GIZMO_IDLE,

    GIZMO_HOVER_ROT,
    GIZMO_ROT,
} GizmoState;

typedef enum GizmoAxis {
    GIZMO_AXIS_X,
    GIZMO_AXIS_Y,
    GIZMO_AXIS_Z
} GizmoAxis;

typedef struct Gizmo {
    GizmoState state;
    GizmoAxis active_axis;

    float start_angle;
    Vector3 start_rot_handle;
} Gizmo;

void gizmo_update(Gizmo *gizmo, Camera3D camera, Vector3 *position, Vector3 *rotation);
void gizmo_draw(Gizmo *gizmo, Camera3D camera, Vector3 position);
