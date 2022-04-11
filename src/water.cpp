internal Vector2 complex_mult(Vector2 a, Vector2 b) {
    Vector2 result;

    result.x = (a.x * b.x) - (a.y * b.y);
    result.y = (a.y * b.x) + (a.x * b.y);

    return result;
}

internal f32 phillips(Vector2 k) {
    f32 wind_speed = game_state->wind_speed;
    f32 smoothing = game_state->smoothing;
    f32 amplitude = game_state->amplitude;
    Vector2 wind_direction = game_state->wind_direction;

    f32 L = wind_speed * wind_speed / GRAVITY;

    f32 k_length = length(k);
    if (k_length < 0.0001f) return 0.0f;

    f32 k_length_2 = k_length * k_length;

    f32 k_dot_wind = dot(normalized(k), normalized(wind_direction));
    f32 k_dot_wind_2 = k_dot_wind * k_dot_wind;

    return
        amplitude *
        (
            expf(-1.0f / (k_length_2 * L * L))
            /
            (k_length_2 * k_length_2)
        )
        *
        k_dot_wind_2
        *
        expf(-k_length_2 * smoothing * smoothing);
}

internal Vector2 h0_tilde(Vector2 k) {
    Vector2 gauss_pick;
    gauss_pick.x = game_state->normal_dist(game_state->random_engine);
    gauss_pick.y = game_state->normal_dist(game_state->random_engine);

    f32 p = phillips(k);
    f32 sqrt_of_2 = 1.4142135623730950488f;
    f32 coef = 1.0f / sqrt_of_2;

    Vector2 result = gauss_pick * coef * sqrtf(p);
    return result;
}

internal void water_precalculate() {
    if (game_state->length < 1.0f) game_state->length = 1.0f;

    u32 resolution = game_state->resolution;
    u32 resolution_squared = resolution * resolution;

    game_state->h_tilde.clear();
    game_state->dx.clear();
    game_state->dz.clear();

    for (u32 i = 0; i < resolution_squared; i++) {
        game_state->h_tilde.add(V2_ZERO);
        game_state->dx.add(V2_ZERO);
        game_state->dz.add(V2_ZERO);
    }

    f32 length = game_state->length;

    game_state->h0tk.clear();
    game_state->h0tmk.clear();

    for (u32 m = 0; m < resolution; m++) {
        f32 kz = PI * (2.0f * (f32)m - (f32)resolution) / length;

        for (u32 n = 0; n < resolution; n++) {
            f32 kx = PI * (2.0f * (f32)n - (f32)resolution) / length;

            Vector2 h0tk = h0_tilde(make_vector2(kx, kz));
            game_state->h0tk.add(h0tk);

            Vector2 h0tmk = h0_tilde(make_vector2(-kx, -kz));
            game_state->h0tmk.add(h0tmk);
        }
    }
}

internal Vector2 h_tilde(Vector2 k, f32 time, Vector2 h0tk, Vector2 h0tmk) {
    f32 w_0 = 2.0f * PI / 200.0f;
    f32 omegakt = floor(sqrtf(GRAVITY * length(k)) / w_0) * w_0 * time;

    Vector2 rot;
    rot.x = cosf(omegakt);
    rot.y = sinf(omegakt);

    Vector2 roti;
    roti.x =  rot.x;
    roti.y = -rot.y;

    Vector2 h0tmk_conj;
    h0tmk_conj.x =  h0tmk.x;
    h0tmk_conj.y = -h0tmk.y;

    return (complex_mult(h0tk, rot) + complex_mult(h0tmk_conj, roti));
}

internal void _water_update_y(f32 time) {
    u32 resolution = game_state->resolution;
    f32 length = game_state->length;

    {
        game_state->h_tilde_i.clear();

        u32 index = 0;

        for (u32 m = 0; m < resolution; m++) {
            f32 kz = PI * (2.0f * (f32)m - (f32)resolution) / length;

            for (u32 n = 0; n < resolution; n++) {
                f32 kx = PI * (2.0f * (f32)n - (f32)resolution) / length;

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
            f32 kz = PI * (2.0f * (f32)m - (f32)resolution) / length;

            for (u32 n = 0; n < resolution; n++) {
                f32 kx = PI * (2.0f * (f32)n - (f32)resolution) / length;

                f32 k_len = sqrtf(kx * kx + kz * kz);

                if (k_len < 0.000001f) {
                    game_state->dx_i.add(V2_ZERO);
                    game_state->dz_i.add(V2_ZERO);
                }
                else {
                    Vector2 dx_i = complex_mult(game_state->h_tilde_i[index], make_vector2(0.0f, -kx / k_len));
                    game_state->dx_i.add(dx_i);

                    Vector2 dz_i = complex_mult(game_state->h_tilde_i[index], make_vector2(0.0f, -kz / k_len));
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
    _water_update_xz();

    u32 resolution = game_state->resolution;
    u32 resolution_squared = resolution * resolution;

    for (u32 index = 0; index < resolution_squared; index++) {
        (*texture_data)[index].x = game_state->dx[index].x;
        (*texture_data)[index].y = game_state->h_tilde[index].x;
        (*texture_data)[index].z = game_state->dz[index].x;
    }
}
