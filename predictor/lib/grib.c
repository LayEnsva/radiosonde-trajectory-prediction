/*
IMPORTANT
The binary output coming from the grib file is a very long 1D array, but is in fact 5D. 

  - lng_points
  - lat_points
  - time_points
  - alt_points
  - var_points

Those variables are determined by the user at the moment of generating the binary file.
The order is very important but it differs from one model to the other, that's why there is an additional "model" value
*/

float arpege_ip1_valid_p[34] = {1.0,2.0,3.0,5.0,7.0,10.0,20.0,30.0,50.0,70.0,100.0,125.0,150.0,175.0,200.0,225.0,250.0,275.0,300.0,350.0,400.0,450.0,500.0,550.0,600.0,650.0,700.0,750.0,800.0,850.0,900.0,925.0,950.0,1000.0};
float arome_ip1_valid_p[24]  = {100.0,125.0,150.0,175.0,200.0,225.0,250.0,275.0,300.0,350.0,400.0,450.0,500.0,550.0,600.0,650.0,700.0,750.0,800.0,850.0,900.0,925.0,950.0,1000.0};

typedef enum {
    ARPEGE,
    AROME
} model_t;

typedef struct gridparam_t {
    
    // User definied parameters (in WGRIB2)
    float west;
    float south;
    
    uint16_t lng_points;
    uint16_t lat_points;
    uint16_t alt_points;
    uint16_t time_points;
    uint16_t var_points;
    
    // Model specific parameters
    float lng_angle;
    float lat_angle;
    float *valid_pressure;  // Valid pressure values
    float fcst_interval;    // Forecast interval time (in hours)
    
    model_t model;          // ARPEGE or AROME
    
} gridparam_t;

float pressure_level(float p, const float valid[], int size) {
    int low = 0, high = size - 1, mid;
    while (low <= high) {
        mid = low + ((high - low) >> 1);
        if (valid[mid] < p) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
//    return (low < size) ? low : -1;
    return (low < size) ? low - (p - valid[low]) / (valid[low-1] - valid[low]) : low-1;
}

void convert_geo2grib(state_t state, float *rx, float *ry, float *rz, float *rt, gridparam_t grid) {
    (*rx) = ((state.lng - grid.west)  * (float) grid.lng_points) / ((float) grid.lng_points * grid.lng_angle);
    (*ry) = ((state.lat - grid.south) * (float) grid.lat_points) / ((float) grid.lat_points * grid.lat_angle);
    (*rz) = pressure_level(get_pressure(state.alt) * 10.0, grid.valid_pressure, grid.alt_points);
    (*rt) = state.time / grid.fcst_interval;
}
