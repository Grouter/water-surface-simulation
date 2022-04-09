internal bool load_program(const char *path, u32 &handle, u32 input_count = 0, char **inputs = nullptr) {
    Array<char> shader_source;

    bool success = read_whole_file(path, shader_source);
    if (!success) return false;

    i32 compile_status;
    static char compile_log[512];

    u32 vertex_handle;
    u32 fragment_handle;

    Array<const char *> shader_inputs;
    allocate_array(shader_inputs, 8);

    shader_inputs.add(GLSL_VERSION_LINE);
    for (u32 i = 0; i < input_count; i++) shader_inputs.add(inputs[i]);

    // Compile the vertex part
    {
        vertex_handle = glCreateShader(GL_VERTEX_SHADER);

        shader_inputs.add("#define VERTEX\n");
        shader_inputs.add(shader_source.data);

        glShaderSource(vertex_handle, (i32)shader_inputs.length, shader_inputs.data, NULL);
        glCompileShader(vertex_handle);

        shader_inputs.length -= 2;

        SHADER_COMPILATION_CHECK(vertex_handle, compile_status, compile_log, 512, "Vertex");

        if (!compile_status) {
            glDeleteShader(vertex_handle);
            free_array(shader_source);
            free_array(shader_inputs);
            return false;
        }
    }

    // Compile the fragment part
    {
        fragment_handle = glCreateShader(GL_FRAGMENT_SHADER);

        shader_inputs.add("#define FRAGMENT\n");
        shader_inputs.add(shader_source.data);

        glShaderSource(fragment_handle, (i32)shader_inputs.length, shader_inputs.data, NULL);
        glCompileShader(fragment_handle);

        shader_inputs.length -= 2;

        SHADER_COMPILATION_CHECK(fragment_handle, compile_status, compile_log, 512, "Fragment");

        if (!compile_status) {
            glDeleteShader(fragment_handle);
            free_array(shader_source);
            free_array(shader_inputs);
            return false;
        }
    }

    // Initialize and link
    {
        handle = glCreateProgram();
        glAttachShader(handle, vertex_handle);
        glAttachShader(handle, fragment_handle);

        glLinkProgram(handle);
    }

    // Check link status
    {
        glGetProgramiv(handle, GL_LINK_STATUS, &compile_status);

        if (!compile_status) {
            glGetProgramInfoLog(handle, 512, NULL, compile_log);

            log_print("Program link error!\n");
            log_print("%s\n", compile_log);

            glDeleteShader(vertex_handle);
            glDeleteShader(fragment_handle);

            glDeleteProgram(handle);

            free_array(shader_source);
            free_array(shader_inputs);

            return false;
        }
    }

    glDetachShader(handle, vertex_handle);
    glDetachShader(handle, fragment_handle);

    glDeleteShader(vertex_handle);
    glDeleteShader(fragment_handle);

    free_array(shader_source);
    free_array(shader_inputs);

    return true;
}

internal bool load_shader(const char *path, Program &shader, u32 input_count = 0, char **inputs = nullptr) {
    u32 handle;

    bool success = load_program(path, handle, input_count, inputs);
    if (!success) return false;

    shader.handle = handle;

    log_print("Loaded shader: %s\n", path);

    return true;
}

internal void bind_mesh_buffer_objects(Mesh &mesh) {
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    {   // Index data
        glGenBuffers(1, &mesh.ebo);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indicies.length * sizeof(u32), mesh.indicies.data, GL_STATIC_DRAW);
    }

    {   // Vertex data
        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.length * sizeof(VertexData), mesh.vertices.data, GL_STATIC_DRAW);

        i32 stride = sizeof(VertexData);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)sizeof(Vector3));
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

internal Mesh create_water_plane(f32 width, f32 height, u32 splits, MemoryArena *memory) {
    Mesh water_plane = {};

    u32 quad_count = (1 + splits) * (1 + splits);
    u32 triangle_count = 2 * quad_count;
    u32 vertex_count = (2 + splits) * (2 + splits);

    allocate_array_from_block(water_plane.vertices, vertex_count, memory);
    allocate_array_from_block(water_plane.indicies, triangle_count * 3, memory);

    log_print("Created water plane: verts: %u, indices: %u\n", vertex_count, triangle_count * 3);


    // Generate vertices
    {
        f32 vertex_distance_x = width / (1.0f + splits);
        f32 vertex_distance_z = height / (1.0f + splits);

        f32 draw_x = -width * 0.5f;
        f32 draw_z = -height * 0.5f;

        for (u32 z_index = 0; z_index < (2 + splits); z_index++) {
            for (u32 x_index = 0; x_index < (2 + splits); x_index++) {
                VertexData new_vert;

                new_vert.position.x = draw_x;
                new_vert.position.y = 0.0f;
                new_vert.position.z = draw_z;

                new_vert.uv.x = ((width * 0.5f) + draw_x) / width;
                new_vert.uv.y = ((height * 0.5f) + draw_z) / height;

                water_plane.vertices.add(new_vert);

                draw_x += vertex_distance_x;
            }

            draw_x = -width * 0.5f;
            draw_z += vertex_distance_z;
        }
    }

    // Generate indices
    {
        u32 index_width = (2 + splits);

        for (u32 x_index = 0; x_index < index_width - 1; x_index++) {
            for (u32 y_index = 0; y_index < index_width - 1; y_index++) {
                u32 index = x_index + (y_index * index_width);

                water_plane.indicies.add(index + 0);
                water_plane.indicies.add(index + index_width);
                water_plane.indicies.add(index + index_width + 1);

                water_plane.indicies.add(index + index_width + 1);
                water_plane.indicies.add(index + 1);
                water_plane.indicies.add(index + 0);
            }
        }
    }

    bind_mesh_buffer_objects(water_plane);

    return water_plane;
}
