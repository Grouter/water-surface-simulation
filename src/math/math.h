#if !defined(MATH_H)
#define MATH_H

const f32 PI              = 3.14159265358979f;
const f32 TWO_PI          = PI * 2.0f;
const f32 HALF_PI         = PI * 0.5f;
const f32 ONE_AND_HALF_PI = PI * 1.5f;

const f32 _TO_RADIANS = PI / 180.0f;
const f32 _TO_DEGREES = 180.0f / PI;

#define TO_RADIANS(v) ((v) * _TO_RADIANS)
#define TO_DEGREES(v) ((v) * _TO_DEGREES)

internal f32 lerp(f32 a, f32 b, f32 t) {
    return (a + ((b - a) * t));
}

// @Speed: too many branches!!
internal i32 sign(i32 a) {
    if (a > 0) return  1;
    if (a < 0) return -1;
    return  0;
}

// Min and Max values are inclusive! Value is clamped in interval: <min_value, max_value>.
internal f32 clamp(f32 value, f32 min_value, f32 max_value) {
    return max(min_value, min(max_value, value));
}

internal f32 rand_f() {
    return (f32)rand() / (f32)RAND_MAX;
}

internal f32 rand_f_range(f32 start, f32 end) {
    f32 delta = end - start;

    assert(delta > 0);  // Start must be less than end

    return (rand_f() * delta) + start;
}

#endif
