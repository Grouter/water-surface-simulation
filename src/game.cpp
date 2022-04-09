internal void init_game() {
    game_state->camera = create_camera((f32)VIRTUAL_WINDOW_W, (f32)VIRTUAL_WINDOW_H);

    translate(game_state->camera.transform, 0.0f, -5.0f, -10.0f);

    // Generate displacement texture
    {
        u32 water_displacement;
        glGenTextures(1, &water_displacement);

        glBindTexture(GL_TEXTURE_2D, water_displacement);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        u32 resolution = game_state->resolution;

        allocate_array_from_block(game_state->water_displacement_data, resolution * resolution, &game_state->transient_memory);
        for (u32 i = 0; i < (resolution * resolution); i++) game_state->water_displacement_data.add(V3_ZERO);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution, resolution, 0, GL_RGB, GL_FLOAT, game_state->water_displacement_data.data);

        game_state->water_displacement.handle = water_displacement;
        game_state->water_displacement.pixel_width = resolution;
        game_state->water_displacement.pixel_height = resolution;
    }

    // Generate normals texture
    {
        u32 water_normals;
        glGenTextures(1, &water_normals);

        glBindTexture(GL_TEXTURE_2D, water_normals);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        u32 resolution = game_state->resolution;

        allocate_array_from_block(game_state->water_normals_data, resolution * resolution, &game_state->transient_memory);
        for (u32 i = 0; i < (resolution * resolution); i++) game_state->water_normals_data.add(V3_ZERO);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution, resolution, 0, GL_RGB, GL_FLOAT, game_state->water_normals_data.data);

        game_state->water_normals.handle = water_normals;
        game_state->water_normals.pixel_width = resolution;
        game_state->water_normals.pixel_height = resolution;
    }

    // f32 window_width = game_state->viewport.width * game_state->pixels_to_units;
    // f32 window_height = game_state->viewport.height * game_state->pixels_to_units;

    game_state->normal_dist = std::normal_distribution<f32>(0.0f, 1.0f);
    game_state->water_plane = create_water_plane(20.0f, 20.0f, 100, &game_state->transient_memory);

    water_precalculate();
}

internal void on_window_created(i32 pixel_width, i32 pixel_height) {
    game_state->window_width_pixels  = pixel_width;
    game_state->window_height_pixels = pixel_height;
}

internal void on_window_resize(u32 pixel_width, u32 pixel_height) {
    game_state->window_width_pixels  = pixel_width;
    game_state->window_height_pixels = pixel_height;

    // Recalculate viewport to keep the aspect ratio
    {
        u32 w_ratio = pixel_width / TARGET_ASPECT_W;
        u32 h_ratio = pixel_height / TARGET_ASPECT_H;

        u32 target_ratio = min(w_ratio, h_ratio);
        if (target_ratio % 2 == 1) target_ratio -= 1;

        pixel_width = target_ratio * TARGET_ASPECT_W;
        pixel_height = target_ratio * TARGET_ASPECT_H;

        game_state->viewport.left   = (game_state->window_width_pixels / 2) - (pixel_width / 2);
        game_state->viewport.bottom = (game_state->window_height_pixels / 2) - (pixel_height / 2);
        game_state->viewport.width  = pixel_width;
        game_state->viewport.height = pixel_height;

        glViewport(
            game_state->viewport.left,
            game_state->viewport.bottom,
            game_state->viewport.width,
            game_state->viewport.height
        );
    }

    game_state->unit_to_pixels = (f32)game_state->viewport.width / (f32)VIRTUAL_WINDOW_W;
    game_state->pixels_to_units = 1.0f / game_state->unit_to_pixels;
}

internal void update_and_render_game(f32 dt) {
    // Camera move
    {
        Vector3 move_direction = {};

        if (input_state->wasd_pressed[0]) { // W
            if (input_state->shift_pressed) move_direction.z += 1;
            else move_direction.y -= 1;
        }

        if (input_state->wasd_pressed[2]) { // S
            if (input_state->shift_pressed) move_direction.z -= 1;
            else move_direction.y += 1;
        }

        if (input_state->wasd_pressed[1]) { // A
            move_direction.x += 1;
        }

        if (input_state->wasd_pressed[3]) { // D
            move_direction.x -= 1;
        }

        move_direction = normalized(move_direction);

        f32 move_speed = 5.0f * dt;

        Vector3 move_delta = {};
        move_delta.x = move_direction.x * move_speed;
        move_delta.y = move_direction.y * move_speed;
        move_delta.z = move_direction.z * move_speed;

        translate(game_state->camera.transform, move_delta.x, move_delta.y, move_delta.z);
    }

    // Draw axes
    {
        // f32 axis_length = 500.0f;
        // f32 axis_width = 0.01f;

        // draw_cube(V3_ZERO, make_vector3(axis_length, axis_width, axis_width), Color_RED);
        // draw_cube(V3_ZERO, make_vector3(axis_width, axis_length, axis_width), Color_GREEN);
        // draw_cube(V3_ZERO, make_vector3(axis_width, axis_width, axis_length), Color_BLUE);
    }

    water_update(&game_state->water_plane, &game_state->water_displacement_data, game_state->time);

    {
        glBindTexture(GL_TEXTURE_2D, game_state->water_displacement.handle);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0, 0,
            game_state->water_displacement.pixel_width, game_state->water_displacement.pixel_height,
            GL_RGB,
            GL_FLOAT,
            game_state->water_displacement_data.data
        );
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    {
        glBindTexture(GL_TEXTURE_2D, game_state->water_normals.handle);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0, 0,
            game_state->water_normals.pixel_width, game_state->water_normals.pixel_height,
            GL_RGB,
            GL_FLOAT,
            game_state->water_normals_data.data
        );
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    set_shader(ShaderResource::ShaderResource_DEFAULT);
    set_shader_mat4x4("projection", game_state->camera.perspective);
    set_shader_mat4x4("view", game_state->camera.transform);
    set_shader_texture("displacement_map", game_state->water_displacement.handle);
    // set_shader_texture("normal_map", game_state->water_normals.handle);
    draw_mesh(&game_state->water_plane, V3_ZERO);

    game_state->time += dt;
}

internal void game_handle_key_down(KeyInput key) {}
