#if !defined(RESOURCES_H)
#define RESOURCES_H

enum ShaderResource {
    ShaderResource_DEFAULT,

    ShaderResource_COUNT
};

struct Resources {
    Program programs[ShaderResource_COUNT];
};

#endif
