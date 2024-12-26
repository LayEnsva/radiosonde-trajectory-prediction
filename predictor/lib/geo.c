#define DEGTORAD 2.f * M_PI / 360.f

typedef struct state_t {
    float lng;
    float lat;
    float alt;
    float time;
} state_t;

// Fast version of get_distance using fsinf and neglecting altitude
void get_fast_distance(float lat, float *d_dlat, float *d_dlng) {
    *d_dlat = 111195.f;                         // 2 * pi * earth_radius / 360
    *d_dlng = 111195.f * fsinf(DEGTORAD * (90.f - lat));
}

void get_distance(float lat, float alt, float *d_dlat, float *d_dlng) {
    float theta, r;
    theta = 2.f * M_PI * (90.f - lat) / 360.f;
    r = 6371009.f + alt;
    *d_dlat = (2.f * M_PI) * r / 360.f;
    *d_dlng = (2.f * M_PI) * r * sinf(theta) / 360.f;
}

float get_pressure(float altitude) {
    
    float temp = 0.f, pressure = 0.f;
    
    if (altitude > 25000) {
        temp = -131.21 + 0.00299 * altitude;
        pressure = 2.488 * pow((temp + 273.1) / 216.6, -11.388);
    }
    if (altitude <= 25000 && altitude > 11000) {
        temp = -56.46;
        pressure = 22.65 * exp(1.73 - 0.000157 * altitude);
    }
    if (altitude <= 11000) {
        temp = 15.04 - 0.00649 * altitude;
        pressure = 101.29 * pow((temp + 273.1) / 288.08, 5.256);
    }
    
    return pressure;
}

float get_altitude(float pressure) {
    float altitude = 0.f;
    
    if (pressure < 2.488) { // Above 25,000 m
        altitude = (216.6 / 0.00299) * (pow(pressure / 2.488, -1.0 / 11.388) - 1) - 273.1 / 0.00299 - 131.21 / 0.00299;
    } 
    else if (pressure <= 22.65 && pressure >= 2.488) { // Between 11,000 m and 25,000 m
        altitude = (1.73 - log(pressure / 22.65)) / 0.000157;
    } 
    else if (pressure > 22.65) { // Below 11,000 m
        altitude = (288.08 / 0.00649) * (1 - pow(pressure / 101.29, 1.0 / 5.256)) - 15.04 / 0.00649;
    }

    return altitude;
}

float get_density(float altitude) {
    
    float temp = 0.f, pressure = 0.f;
    
    if (altitude > 25000) {
        temp = -131.21 + 0.00299 * altitude;
        pressure = 2.488 * pow((temp + 273.1) / 216.6, -11.388);
    }
    if (altitude <= 25000 && altitude > 11000) {
        temp = -56.46;
        pressure = 22.65 * exp(1.73 - 0.000157 * altitude);
    }
    if (altitude <= 11000) {
        temp = 15.04 - 0.00649 * altitude;
        pressure = 101.29 * pow((temp + 273.1) / 288.08, 5.256);
    }
    
    return pressure / (0.2869 * (temp + 273.1));
}

const float pressure_coef3[4] = {1.00333335e+02, -1.12497807e-02, 4.48884257e-07, -6.18187296e-12};
const float pressure_coef4[5] = {1.01427625e+02, -1.19826068e-02, 5.58936515e-07, -1.18901878e-11, 9.51385810e-17};
const float pressure_coef5[6] = {1.01264837e+02, -1.18186713e-02, 5.20608328e-07, -8.48068527e-12, -3.27496725e-17, 1.70517671e-21};

float polynomial(float x, const float c[], int n) {
    int i;
    float res = c[n];
    for(i=n-1;i>=0;i--) res = c[i] + x*res;
    return res;
}

// Polynomial approximation of atmospheric pressure, for computational efficiency
float get_fast_pressure(float altitude) {
    return polynomial(altitude, pressure_coef5, 5);
}
