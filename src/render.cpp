internal void init_renderer() {}

internal void set_shader(ShaderResource shader) {
    Program *to_set = &resources->programs[shader];

    render_state->current_shader = to_set;

    glUseProgram(to_set->handle);
}

internal void set_shader_int(const char *attribute, i32 value) {
    i32 loc = glGetUniformLocation(render_state->current_shader->handle, attribute);

    if (loc >= 0) {
        glUniform1i(loc, value);
    }
    else {
        log_print("Shader set int loc error! (attribute: %s)\n", attribute);
    }
}

internal void set_shader_mat4x4(const char *attribute, Matrix4x4 perspective) {
    i32 loc = glGetUniformLocation(render_state->current_shader->handle, attribute);

    if (loc >= 0) {
        glUniformMatrix4fv(loc, 1, false, perspective.raw);
    }
    else {
        log_print("Shader set mat4x4 loc error! (attribute: %s)\n", attribute);
    }
}

internal void set_shader_vector4(const char *attribute, Vector4 color) {
    i32 loc = glGetUniformLocation(render_state->current_shader->handle, attribute);

    if (loc >= 0) {
        glUniform4f(loc, color.r, color.g, color.b, color.a);
    }
    else {
        log_print("Shader set Vector4 loc error! (attribute: %s)\n", attribute);
    }
}

internal void set_shader_texture(const char *attribute, u32 texture_handle) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_handle);

    i32 loc = glGetUniformLocation(render_state->current_shader->handle, attribute);

    if (loc >= 0) {
        glUniform1i(loc, 0);
    }
    else {
        log_print("Shader set texture loc error! (attribute: %s)\n", attribute);
    }
}

internal void set_shader_texture_2(const char *attribute, u32 texture_handle) {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_handle);

    i32 loc = glGetUniformLocation(render_state->current_shader->handle, attribute);

    if (loc >= 0) {
        glUniform1i(loc, 0);
    }
    else {
        log_print("Shader set texture loc error! (attribute: %s)\n", attribute);
    }
}

internal void draw_mesh(Mesh *mesh, Vector3 position) {
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, (i32)mesh->indicies.length, GL_UNSIGNED_INT, nullptr);
}
