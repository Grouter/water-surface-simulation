internal Camera create_camera(f32 window_w, f32 window_h) {
    Camera camera = {};

    camera.clip_near = 0.01f;
    camera.clip_far  = 1000.0f;

    camera.virtual_w = window_w;
    camera.virtual_h = window_h;

    camera.fov = 70.0f;

    // camera.perspective = ortho(-1.0f, 1.0f, window_w, window_h);
    camera.perspective = perspective(camera.fov, camera.clip_near, camera.clip_far, window_w, window_h);
    camera.transform = identity();

    return camera;
}

inline Vector3 get_forward_vector(Camera &camera) {
    return { camera.transform.raw[2], camera.transform.raw[6], camera.transform.raw[10] };
}

inline Vector3 get_up_vector(Camera &camera) {
    return { camera.transform.raw[1], camera.transform.raw[5], camera.transform.raw[9] };
}

inline Vector3 get_side_vector(Camera &camera) {
    return { camera.transform.raw[0], camera.transform.raw[4], camera.transform.raw[8] };
}
