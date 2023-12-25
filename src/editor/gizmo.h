#include <raylib.h>

#define GIZMO_SIZE 10.0f
#define GIZMO_ROT_CIRCLE_THICKNESS 0.1f
#define GIZMO_ROT_CIRCLE_DRAW_THICKNESS 4.0f
#define GIZMO_ROT_HANDLE_DRAW_THICKNESS 2.0f

typedef enum GizmoState {
    GIZMO_COLD,

    GIZMO_HOT_ROT,
    GIZMO_ACTIVE_ROT,
} GizmoState;

typedef struct Gizmo {
    GizmoState state;
    Vector3 active_axis;
} Gizmo;

Matrix gizmo_update(Gizmo *gizmo, Camera3D camera, Vector3 *position);
void gizmo_draw(Gizmo *gizmo, Camera3D camera, Vector3 position);
