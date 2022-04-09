#if !defined(CAMERA_H)
#define CAMERA_H

struct Camera {
    f32 clip_near;
    f32 clip_far;

    f32 virtual_w;
    f32 virtual_h;

    f32 fov;

    Matrix4x4 perspective;
    Matrix4x4 transform;
};

#endif
