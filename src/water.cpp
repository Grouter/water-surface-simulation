internal f32 phillips(Vector2 k) {
    f32 wind_speed = game_state->wind_speed;
    f32 smoothing = game_state->smoothing;
    f32 amplitude = game_state->amplitude;
    Vector2 wind_direction = game_state->wind_direction;

    f32 l = wind_speed * wind_speed / GRAVITY;

    f32 k_length = length(k);
    if (k_length < 0.0001f) k_length = 0.0001f;
    f32 k_length_2 = k_length * k_length;

    f32 k_dot_wind_dir = dot(normalized(k), normalized(wind_direction));

    f32 pow_dt = k_dot_wind_dir * k_dot_wind_dir * k_dot_wind_dir * k_dot_wind_dir;

    return
        amplitude *
        expf(-1.0f / (k_length_2 * l * l)) / (k_length_2 * k_length_2) *
        pow_dt *
        expf(-(k_length_2) * smoothing * smoothing);
}

internal Vector2 h0_tilde(Vector2 k) {
    Vector2 gauss_pick;
    gauss_pick.x = game_state->normal_dist(game_state->random_engine);
    gauss_pick.y = game_state->normal_dist(game_state->random_engine);

    f32 p = phillips(k);

    f32 coef = (1.0f / sqrtf(2.0f)) * sqrtf(p);

    Vector2 result = gauss_pick * coef;
    return result;
}

internal void water_precalculate() {
    if (game_state->length <= 0.0f) game_state->length = 1.0f;

    u32 resolution = game_state->resolution;

    // Allocations
    {
        if (game_state->h0tk.data) _free_from_memory_arena(&game_state->transient_memory, game_state->h0tk.data);
        if (game_state->h0tmk.data) _free_from_memory_arena(&game_state->transient_memory, game_state->h0tmk.data);
        if (game_state->h_tilde.data) _free_from_memory_arena(&game_state->transient_memory, game_state->h_tilde.data);
        if (game_state->h_tilde_i.data) _free_from_memory_arena(&game_state->transient_memory, game_state->h_tilde_i.data);
        if (game_state->dx.data) _free_from_memory_arena(&game_state->transient_memory, game_state->dx.data);
        if (game_state->dx_i.data) _free_from_memory_arena(&game_state->transient_memory, game_state->dx_i.data);
        if (game_state->dz.data) _free_from_memory_arena(&game_state->transient_memory, game_state->dz.data);
        if (game_state->dz_i.data) _free_from_memory_arena(&game_state->transient_memory, game_state->dz_i.data);

        u32 resulution_squared = resolution * resolution;

        allocate_array_from_block(game_state->h0tk, resulution_squared, &game_state->transient_memory);
        allocate_array_from_block(game_state->h0tmk, resulution_squared, &game_state->transient_memory);
        allocate_array_from_block(game_state->h_tilde, resulution_squared, &game_state->transient_memory);
        allocate_array_from_block(game_state->h_tilde_i, resulution_squared, &game_state->transient_memory);
        allocate_array_from_block(game_state->dx, resulution_squared, &game_state->transient_memory);
        allocate_array_from_block(game_state->dx_i, resulution_squared, &game_state->transient_memory);
        allocate_array_from_block(game_state->dz, resulution_squared, &game_state->transient_memory);
        allocate_array_from_block(game_state->dz_i, resulution_squared, &game_state->transient_memory);
    }

    game_state->h_tilde.length = game_state->h_tilde.capacity;
    game_state->dx.length = game_state->dx.capacity;
    game_state->dz.length = game_state->dz.capacity;

    // FFTW
    {
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

    f32 length = game_state->length;

    game_state->h0tk.clear();
    game_state->h0tmk.clear();

    for (u32 m = 0; m < resolution; m++) {
        f32 kz = TWO_PI * ((f32)m - resolution / 2.0f) / length;

        for (u32 n = 0; n < resolution; n++) {
            f32 kx = TWO_PI * ((f32)n - resolution / 2.0f) / length;

            Vector2 h0tk = h0_tilde(make_vector2(kx, kz));
            game_state->h0tk.add(h0tk);

            Vector2 h0tmk = h0_tilde(make_vector2(-kx, -kz));
            game_state->h0tmk.add(h0tmk);
        }
    }
}

internal Vector2 h_tilde(Vector2 k, f32 time, Vector2 h0tk, Vector2 h0tmk) {
    f32 omegakt = sqrtf(GRAVITY * length(k)) * time;

    Vector2 rot;
    rot.x = cosf(omegakt);
    rot.y = sinf(omegakt);

    Vector2 roti;
    roti.x = rot.x;
    roti.y = - rot.y;

    Vector2 h0tmk_conj;
    h0tmk_conj.x =  h0tmk.x;
    h0tmk_conj.y = -h0tmk.y;

    return (h0tk * rot) + (h0tmk * roti);
}

internal void _water_update_y(f32 time) {
    u32 resolution = game_state->resolution;
    f32 length = game_state->length;

    {
        game_state->h_tilde_i.clear();

        u32 index = 0;

        for (u32 m = 0; m < resolution; m++) {
            f32 kz = TWO_PI * ((f32)m - resolution / 2.0f) / length;

            for (u32 n = 0; n < resolution; n++) {
                f32 kx = TWO_PI * ((f32)n - resolution / 2.0f) / length;

                Vector2 h0tk = game_state->h0tk[index];
                Vector2 h0tmk = game_state->h0tmk[index];

                Vector2 result = h_tilde(make_vector2(kx, kz), time, h0tk, h0tmk);

                game_state->h_tilde_i.add(result);

                index += 1;
            }
        }
    }

    fftwf_execute(game_state->p_h_tilde);

    {
        f32 signs[2] = { 1.0f, -1.0f };
        u32 index = 0;

        for (u32 m = 0; m < resolution; m++) {
            for (u32 n = 0; n < resolution; n++) {
                game_state->h_tilde[index] *= signs[(m + n) & 1] / NORM;

                index += 1;
            }
        }
    }
}

internal void _water_update_xz() {
    u32 resolution = game_state->resolution;
    f32 length = game_state->length;

    {
        u32 index = 0;

        game_state->dx_i.clear();
        game_state->dz_i.clear();

        for (u32 m = 0; m < resolution; m++) {
            f32 kz = TWO_PI * ((f32)m - resolution / 2.0f) / length;

            for (u32 n = 0; n < resolution; n++) {
                f32 kx = TWO_PI * ((f32)n - resolution / 2.0f) / length;

                f32 k_len = sqrtf(kx * kx + kz * kz);

                if (k_len < 0.000001f) {
                    game_state->dx_i.add(V2_ZERO);
                    game_state->dz_i.add(V2_ZERO);
                }
                else {
                    Vector2 dx_i = game_state->h_tilde_i[index] * make_vector2(0.0f, -kx / k_len);
                    game_state->dx_i.add(dx_i);

                    Vector2 dz_i = game_state->h_tilde_i[index] * make_vector2(0.0f, -kz / k_len);
                    game_state->dz_i.add(dz_i);
                }

                index += 1;
            }
        }
    }

    fftwf_execute(game_state->p_dx);
    fftwf_execute(game_state->p_dz);

    {
        f32 signs[2] = { 1.0f, -1.0f };
        u32 index = 0;

        f32 lambda = game_state->lambda;

        for (u32 m = 0; m < resolution; m++) {
            for (u32 n = 0; n < resolution; n++) {
                game_state->dx[index] *= signs[(m + n) & 1] * lambda / NORM;
                game_state->dz[index] *= signs[(m + n) & 1] * lambda / NORM;

                index += 1;
            }
        }
    }
}

internal void water_update(Mesh *water_plane, StaticArray<Vector3> *texture_data, f32 time) {
    _water_update_y(time);

    u32 resolution = game_state->resolution;

    u32 index = 0;

    for (u32 m = 0; m < resolution; m++) {
        for (u32 n = 0; n < resolution; n++) {
            (*texture_data)[index].x = game_state->dx[index].x;
            (*texture_data)[index].y = game_state->h_tilde[index].x;
            (*texture_data)[index].z = game_state->dz[index].x;

            index += 1;
        }
    }
}
