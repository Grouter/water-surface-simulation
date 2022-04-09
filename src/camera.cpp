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

internal Vector3 get_forward_vector(Camera *camera) {
    return { camera->transform.raw[2], camera->transform.raw[6], camera->transform.raw[10] };
}

internal Vector3 get_up_vector(Camera *camera) {
    return { camera->transform.raw[1], camera->transform.raw[5], camera->transform.raw[9] };
}

internal Vector3 get_side_vector(Camera *camera) {
    return { camera->transform.raw[0], camera->transform.raw[4], camera->transform.raw[8] };
}

internal void camera_update(Camera *camera) {
    if (camera->rotation.pitch > 89.0f)
        camera->rotation.pitch = 89.0f;
    else if (camera->rotation.pitch < -89.0f)
        camera->rotation.pitch = -89.0f;

    Vector3 direction;
    direction.x = cosf(TO_RADIANS(camera->rotation.yaw)) * cosf(TO_RADIANS(camera->rotation.pitch));
    direction.y = sinf(TO_RADIANS(camera->rotation.pitch));
    direction.z = sinf(TO_RADIANS(camera->rotation.yaw)) * cosf(TO_RADIANS(camera->rotation.pitch));

    Vector3 forward = normalized(direction);
    Vector3 side    = normalized(cross(V3_UP, forward));
    Vector3 up      = cross(forward, side);

    camera->direction = forward * -1.0f;

    Vector3 position = make_vector3(
        -camera->position.x * side.x    - camera->position.y * side.y    - camera->position.z * side.z,
        -camera->position.x * up.x      - camera->position.y * up.y      - camera->position.z * up.z,
        -camera->position.x * forward.x - camera->position.y * forward.y - camera->position.z * forward.z
    );

    camera->transform = {
        side.x, up.x, forward.x, 0.0f,
        side.y, up.y, forward.y, 0.0f,
        side.z, up.z, forward.z, 0.0f,
        position.x, position.y, position.z, 1.0f,
    };
}

internal void camera_handle_input(Camera *camera, f32 dt) {
    // Rotation
    if (input_state->mouse_locked) {
        // @Broken: this is not windows size independent!!!!!!!!!!!!!
        camera->rotation.yaw   += (f32)input_state->mouse_delta.x * CAMERA_SENS * 0.04f;
        camera->rotation.pitch += (f32)input_state->mouse_delta.y * CAMERA_SENS * 0.04f;

        if (camera->rotation.yaw >= 360.0f) {
            camera->rotation.yaw -= 360.0f;
        }
        else if (camera->rotation.yaw <= -360.0f) {
            camera->rotation.yaw += 360.0f;
        }

        if (camera->rotation.pitch >= 360.0f) {
            camera->rotation.pitch -= 360.0f;
        }
        else if (camera->rotation.pitch <= -360.0f) {
            camera->rotation.pitch += 360.0f;
        }
    }

    // Movement
    Vector3 forward = get_forward_vector(camera);
    Vector3 side    = get_side_vector(camera);
    Vector3 input   = {};

    if (input_state->wasd_pressed[1]) { // A
        input.x = -1;
    }
    if (input_state->wasd_pressed[3]) { // D
        input.x = 1;
    }
    if (input_state->wasd_pressed[0]) { // W
        input.y = -1;
    }
    if (input_state->wasd_pressed[2]) { // S
        input.y = 1;
    }

    input = normalized(input);

    Vector3 offset = input * 0.8f;

    offset *= dt;

    offset *= 100.0f;

    if (input_state->shift_pressed) offset *= 10.0f;

    camera->position += (side * offset.x);
    camera->position += (forward * offset.y);
}
