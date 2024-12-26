// Fast Inverse Square Root
float fisqrtf(float number) {
    union {float f; uint32_t i;} conv = {.f = number};
    conv.i  = 0x5f3759df - (conv.i >> 1);                 // If return here, low precision but still not so bad
    conv.f *= 1.5F - (number * 0.5F * conv.f * conv.f);   // Newton step for better precision
    return conv.f;
}

#define ONESIXTH 1.f / 6.f

// Fucking bad sine approximation
float fsinf(float x) {
    return x - ONESIXTH * x * x * x;
}
