#if !defined(CAMERA_H)
#define CAMERA_H

const f32 CAMERA_SENS = 1.0f;

struct Camera {
    f32 clip_near;
    f32 clip_far;

    f32 virtual_w;
    f32 virtual_h;

    f32 fov;

    Vector3 position;
    Vector3 velocity;
    Vector3 rotation;
    Vector3 rotation_velocity;
    Vector3 direction;

    Matrix4x4 perspective;
    Matrix4x4 transform;
};

#endif
