internal void init_game() {
    game_state->camera = create_camera((f32)VIRTUAL_WINDOW_W, (f32)VIRTUAL_WINDOW_H);

    game_state->camera.rotation.y = 90.0f;
    game_state->camera.position.y = 10.0f;
    game_state->camera.position.z = 10.0f;
    game_state->camera.rotation.x = 30.0f;

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

        allocate_array_from_block(game_state->water_displacement_data, resolution * resolution, &game_state->program_memory);
        for (u32 i = 0; i < (resolution * resolution); i++) game_state->water_displacement_data.add(V3_ZERO);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution, resolution, 0, GL_RGB, GL_FLOAT, game_state->water_displacement_data.data);

        game_state->water_displacement.handle = water_displacement;
        game_state->water_displacement.pixel_width = resolution;
        game_state->water_displacement.pixel_height = resolution;
    }

    // Generate debug texture
    {
        u32 debug_texture;
        glGenTextures(1, &debug_texture);

        glBindTexture(GL_TEXTURE_2D, debug_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        u32 resolution = game_state->resolution;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, resolution, resolution, 0, GL_RG, GL_FLOAT, nullptr);

        game_state->debug_texture.handle = debug_texture;
        game_state->debug_texture.pixel_width = resolution;
        game_state->debug_texture.pixel_height = resolution;
    }

    game_state->normal_dist = std::normal_distribution<f32>(0.0f, 1.0f);
    game_state->water_plane = create_water_plane(20.0f, 20.0f, 200, &game_state->program_memory);

    // Allocate memory for data
    {
        u32 resolution = game_state->resolution;
        u32 resolution_squared = resolution * resolution;

        allocate_array_from_block(game_state->h0tk, resolution_squared, &game_state->program_memory);
        allocate_array_from_block(game_state->h0tmk, resolution_squared, &game_state->program_memory);
        allocate_array_from_block(game_state->h_tilde, resolution_squared, &game_state->program_memory);
        allocate_array_from_block(game_state->h_tilde_i, resolution_squared, &game_state->program_memory);
        allocate_array_from_block(game_state->dx, resolution_squared, &game_state->program_memory);
        allocate_array_from_block(game_state->dx_i, resolution_squared, &game_state->program_memory);
        allocate_array_from_block(game_state->dz, resolution_squared, &game_state->program_memory);
        allocate_array_from_block(game_state->dz_i, resolution_squared, &game_state->program_memory);
    }

    // FFTW
    {
        u32 resolution = game_state->resolution;

        game_state->p_h_tilde = fftwf_plan_dft_2d(
            resolution,
            resolution,
            (fftwf_complex*)(game_state->h_tilde_i.data),
            (fftwf_complex*)(game_state->h_tilde.data),
            FFTW_BACKWARD, FFTW_ESTIMATE
        );

        game_state->p_dx = fftwf_plan_dft_2d(
            resolution,
            resolution,
            (fftwf_complex*)(game_state->dx_i.data),
            (fftwf_complex*)(game_state->dx.data),
            FFTW_BACKWARD, FFTW_ESTIMATE
        );

        game_state->p_dz = fftwf_plan_dft_2d(
            resolution,
            resolution,
            (fftwf_complex*)(game_state->dz_i.data),
            (fftwf_complex*)(game_state->dz.data),
            FFTW_BACKWARD, FFTW_ESTIMATE
        );
    }

    water_precalculate();

    log_print(
        "Game memory: %d / %d (%f)\n",
        game_state->program_memory.used,
        game_state->program_memory.bytes,
        ((f32)game_state->program_memory.used / (f32)game_state->program_memory.bytes) * 100.0f
    );
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
    camera_handle_input(&game_state->camera, dt);
    camera_update(&game_state->camera);

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

    // {
    //     glBindTexture(GL_TEXTURE_2D, game_state->debug_texture.handle);
    //     glTexSubImage2D(
    //         GL_TEXTURE_2D,
    //         0,
    //         0, 0,
    //         game_state->debug_texture.pixel_width, game_state->debug_texture.pixel_height,
    //         GL_RG,
    //         GL_FLOAT,
    //         game_state->h0tk.data
    //     );
    //     glBindTexture(GL_TEXTURE_2D, 0);
    // }

    set_shader(ShaderResource::ShaderResource_DEFAULT);
    set_shader_mat4x4("projection", game_state->camera.perspective);
    set_shader_mat4x4("view", game_state->camera.transform);
    set_shader_texture("displacement_map", game_state->water_displacement.handle);
    // set_shader_texture_2("debug_map", game_state->debug_texture.handle);
    draw_mesh(&game_state->water_plane, V3_ZERO);

    // Variables
    {
        ImGui::Begin("Variables");

        ImGuiIO io = ImGui::GetIO();
        ImGui::Text("FPS: %f", io.Framerate);

        ImGui::Text("Resolution: %d", game_state->resolution);

        if (ImGui::InputFloat("Amplitude", &game_state->amplitude)) {
            if (game_state->amplitude < 0.1f) game_state->amplitude = 0.1f;
        }

        if (ImGui::InputFloat("Wind Speed", &game_state->wind_speed)) {
            if (game_state->wind_speed < 0.1f) game_state->wind_speed = 0.1f;
        }

        if (ImGui::InputFloat("Sample Size", &game_state->length)) {
            if (game_state->length < 1.0f) game_state->length = 1.0f;
        }

        ImGui::InputFloat("Lambda", &game_state->lambda);
        ImGui::InputFloat("Smoothing", &game_state->smoothing);

        if (ImGui::Button("Recalculate!")) {
            water_precalculate();
        }

        ImGui::End();
    }

    // @Todo this will cause errors after a while (precision errors)
    game_state->time += dt;
}

internal void game_handle_key_down(KeyInput key) {}
