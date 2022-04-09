#define STATIC_ARRAY_LEN(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define KILOBYTES(value) ((value) * 1024)
#define MEGABYTES(value) (KILOBYTES((value)) * 1024)
#define GIGABYTES(value) (MEGABYTES((value)) * 1024)

#if defined(DEVELOPER)
#define log_print(format, ...) _log_print(format, __VA_ARGS__)
#else
#define log_print(format, ...) (0)
#endif

#define assert_m(expr, msg) assert((msg, expr))

#define invalid_code_path(msg) assert_m(false, msg)

internal void _log_print(const char* format, ...) {
    static char print_buffer[4096];

    va_list args;
    va_start(args, format);
    _vsnprintf(print_buffer, 4096, format, args);
    va_end(args);

    OutputDebugStringA(print_buffer);
}

inline u64 millis() {
    timespec t;

    timespec_get(&t, TIME_UTC);

    return (t.tv_sec * 1000) + (t.tv_nsec / 1000000);
}

internal u64 hash_string(const char *key) {
    if (!key || !key[0]) return 1;

    return key[0] * 33 * hash_string(key + 1);
}
