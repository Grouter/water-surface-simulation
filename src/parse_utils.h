#if !defined(PARSE_UTILS_H)
#define PARSE_UTILS_H

internal bool read_whole_file(const char *name, Array<char> &buffer) {
    FILE *f;

    errno_t open_err = fopen_s(&f, name, "rb");

    if (open_err) {
        log_print("Error opening file: %s (err no.: %d)\n", name, open_err);
        return false;
    }

    fseek(f, 0, SEEK_END);
    u64 size = ftell(f);
    rewind(f);

    allocate_array(buffer, size + 1);
    memset(buffer.data, 0, size + 1);

    fread(buffer.data, sizeof(char), size, f);

    buffer.length = size + 1;

    fclose(f);

    return true;
}

#endif // PARSE_UTILS_H
