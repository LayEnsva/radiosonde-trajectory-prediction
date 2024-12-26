/**
 * @brief Performs linear interpolation between two values.
 *
 * The function calculates the interpolated value between `a` and `b` 
 * based on the interpolation factor `x`.
 *
 * @param a The starting value of the interpolation.
 * @param b The ending value of the interpolation.
 * @param x The interpolation factor, typically in the range [0.0, 1.0].
 *
 * @return The interpolated value between `a` and `b` based on `x`.
 */

float lerp(float a, float b, float x) {
	return a * (1.0f - x) + b * x;
}

/**
 * @brief Performs multi-dimensional linear interpolation (nlerp).
 *
 * This function computes the n-dimensional linear interpolation of 
 * data points stored in the array `p` based on the interpolation factors 
 * provided in `coord`.
 *
 * @param n      The number of dimensions.
 * @param p      A pointer to the array of 2^n input data points.
 *               The array must be organized such that for each dimension, 
 *               adjacent pairs of points correspond to the interpolation
 *               along that dimension.
 * @param coord  An array of `n` interpolation factors, where:
 *               - `coord[0]` is used for the first dimension,
 *               - `coord[1]` for the second dimension, and so on.
 *               Each value in `coord` is typically in the range [0.0, 1.0],
 *               though extrapolation is possible for values outside this range.
 *
 * @return The interpolated value as a `float`.
 *
 * @note The function dynamically allocates a temporary buffer for intermediate 
 *       results, which is freed before returning the result. Ensure sufficient 
 *       memory is available for this allocation.
 *
 * @note The input array `p` must contain exactly 2^n elements, where `n` 
 *       is the number of dimensions. Undefined behavior may occur if the size 
 *       of `p` does not match this requirement.
 */

float nlerp(int n, float *p, float coord[]) {
    int i, j, points = 1 << (n - 1);
    float *t = (float *) malloc(points * sizeof(float));

    for(j=0;j<points;j++) 
        t[j] = lerp(p[2*j], p[2*j + 1], coord[0]);

    for(i=1;i<n;i++) {
        points >>= 1;
        for (j=0;j<points;j++)
            t[j] = lerp(t[2*j], t[2*j + 1], coord[i]);
    }

    float result = t[0];
    free(t);
    return result;
}

/**
 * @brief Performs cubic Hermite interpolation between four points.
 *
 * This function calculates a smooth interpolated value using cubic Hermite 
 * interpolation. It takes four control points (`p0`, `p1`, `p2`, `p3`) 
 * and an interpolation factor `x` to compute the value at `x` within the 
 * range defined by `p1` and `p2`.
 *
 * This method provides a smooth curve through `p1` and `p2`, influenced by
 * the neighboring points `p0` and `p3`.
 *
 * @param p0 The point before the range of interpolation (influences the curve).
 * @param p1 The starting point of the interpolation range.
 * @param p2 The ending point of the interpolation range.
 * @param p3 The point after the range of interpolation (influences the curve).
 * @param x  The interpolation factor, typically in the range [0.0, 1.0].
 *
 * @return The interpolated value at `x` based on the cubic Hermite formula.
 */

float cerp(float p0, float p1, float p2, float p3, float x) {
	return p1 + 0.5f * x*(p2 - p0 + x*(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3 + x*(3.0f * (p1 - p2) + p3 - p0)));
}

/**
 * @brief Performs multi-dimensional cubic Hermite interpolation (ncerp).
 *
 * This function computes the n-dimensional cubic Hermite interpolation 
 * of data points stored in the array `p` based on the interpolation factors 
 * provided in `coord`.
 *
 * @param n      The number of dimensions.
 * @param p      A pointer to the array of 4^n input data points.
 *               The array must be organized such that for each dimension, 
 *               groups of four points correspond to the interpolation along 
 *               that dimension.
 * @param coord  An array of `n` interpolation factors, where:
 *               - `coord[0]` is used for the first dimension,
 *               - `coord[1]` for the second dimension, and so on.
 *               Each value in `coord` is typically in the range [0.0, 1.0],
 *               though extrapolation is possible for values outside this range.
 *
 * @return The interpolated value as a `float`.
 *
 * @note The function dynamically allocates a temporary buffer for intermediate 
 *       results, which is freed before returning the result. Ensure sufficient 
 *       memory is available for this allocation.
 *
 * @note The input array `p` must contain exactly 4^n elements, where `n` 
 *       is the number of dimensions. Undefined behavior may occur if the size 
 *       of `p` does not match this requirement.
 */

float ncerp(int n, float *p, float coord[]) {
    int i, j, points = 1lu << ((2*n) - 2);
    float *t = (float *) malloc(points * sizeof(float));
    
    for(j=0;j<points;j++)
        t[j] = cerp(p[4*j], p[4*j + 1], p[4*j + 2], p[4*j + 3], coord[0]);
    
    for(i=1;i<n;i++) {
        points >>= 2;
        for(j=0;j<points;j++)
            t[j] = cerp(t[4*j], t[4*j + 1], t[4*j + 2], t[4*j + 3], coord[i]);
    }

    float result = t[0];
    free(t);
    return result;
}
