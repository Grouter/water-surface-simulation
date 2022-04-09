#if !defined(GRAPHICS_H)
#define GRAPHICS_H

#define SHADER_COMPILATION_CHECK(handle,status,log,log_size,log_type) {\
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);\
    if (!status) {\
        glGetShaderInfoLog(handle, log_size, NULL, log);\
        log_print("%s shader compilation failed:\n\n", log_type);\
        log_print("%s\n", log);\
    }\
}

const char* GLSL_VERSION = "#version 420";
const char* GLSL_VERSION_LINE = "#version 420\n";

struct Viewport {
    i32 left, bottom;
    i32 width, height;
};

struct Program {
    u32 handle;
};

struct Texture {
    u32 handle;
    u32 pixel_width;
    u32 pixel_height;
};

struct VertexData {
    Vector3 position;
    Vector2 uv;
};

struct Mesh {
    // OpenGL buffers
    u32 vao;
    u32 vbo;
    u32 ebo;

    // Loaded data
    StaticArray<VertexData> vertices;
    StaticArray<u32> indicies;
};

#endif
