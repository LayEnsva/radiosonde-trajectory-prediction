#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "./lib/util.c"
#include "./lib/interpolation.c"
#include "./lib/geo.c"
#include "./lib/grib.c"
#include "./lib/kml.c"



/**
 * Arpege binary file generation command
 *
 *     wgrib2 arpege_025_IP1_[...].grib2 -match ":(UGRD|VGRD):" -no_header -lola "3:20:0.25" "43:20:0.25" arpege.bin bin
 */
 
const gridparam_t arpege = {
    
    .west  = 3.0,
    .south = 43.0,
    
    .lng_points = 20,
    .lat_points = 20,
    .alt_points = 34,
    .time_points = 9,
    .var_points = 2,
    
    .lng_angle = 0.25,
    .lat_angle = 0.25,
    .valid_pressure = arpege_ip1_valid_p,
    .fcst_interval = 3.0,
    
    .model = ARPEGE
    
};



/**
 * Arome binary file generation command
 *
 *     wgrib2 arome_0025_IP1_[...].grib2 -match ":(UGRD|VGRD):" -no_header -lola "3:200:0.025" "43:200:0.025" arome.bin bin
 */
 
const gridparam_t arome = {
    
    .west  = 3.0,
    .south = 43.0,
    
    .lng_points = 200,
    .lat_points = 200,
    .alt_points = 24,
    .time_points = 7,
    .var_points = 2,
    
    .lng_angle = 0.025,
    .lat_angle = 0.025,
    .valid_pressure = arome_ip1_valid_p,
    .fcst_interval = 1.0,
    
    .model = AROME
    
};

// Velocity at groung
#define V0      4.8

// Drag coefficent
#define DRAG    V0 * sqrt(1.226614)

state_t update(FILE *f, state_t curr, gridparam_t grid, float timestep) {
    int i,j,k;
    
    float rx, ry, rz, rt;
    float dx, dy, dz, dt;
    uint16_t x, y, z, t;
    
    // Zonal and meridional wind speeds
    float *u = (float *) calloc(16, sizeof(float));
    float *v = (float *) calloc(16, sizeof(float));
    
    // Convert geographical space-time location to corresponding grid
    convert_geo2grib(curr, &rx, &ry, &rz, &rt, grid);
    
    // Chunk location (to get correct data points)
    x = (uint16_t) rx;
    y = (uint16_t) ry;
    z = (uint16_t) rz;
    t = (uint16_t) rt;
    
    // Position in chunk (for interpolation)
    dx = rx - (float) x;
    dy = ry - (float) y;
    dz = rz - (float) z;
    dt = rt - (float) t;
    
    // Get zonal and meridional wind speeds (U, V) all around current location
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            for (k = 0; k < 2; k++) {
                int location_u = 0, location_v = 0;
                int offset = 2 * (k + 2 * (j + (2 * i)));
                
                if(grid.model == ARPEGE) {
                    location_u = (x + grid.lng_points * (y + k + grid.lat_points * (t + i + grid.time_points * (z + j + grid.alt_points * 0))));
                    location_v = (x + grid.lng_points * (y + k + grid.lat_points * (t + i + grid.time_points * (z + j + grid.alt_points * 1))));
                }
                
                else if(grid.model == AROME) {
                    location_u = (x + grid.lng_points * (y + k + grid.lat_points * (z + j + grid.alt_points * (t + i + grid.time_points * 0))));
                    location_v = (x + grid.lng_points * (y + k + grid.lat_points * (z + j + grid.alt_points * (t + i + grid.time_points * 1))));
                }

                fseek(f, sizeof(float) * location_u, SEEK_SET);
                fread(u + offset, sizeof(float), 2, f);

                fseek(f, sizeof(float) * location_v, SEEK_SET);
                fread(v + offset, sizeof(float), 2, f);
            }
        }
    }
    
    // Get the wind speeds (4-dimensional linear interpolation)
    float wind_u = 0, wind_v = 0;
    wind_u = nlerp(4, u, (float []) {dx, dy, dz, dt});
    wind_v = nlerp(4, v, (float []) {dx, dy, dz, dt});
    
    // Longitudinal and latitudinal displacement
    float du, dv;
    du = wind_u * timestep;
    dv = wind_v * timestep;
    
    float dlat, dlng;
    get_fast_distance(curr.lat, &dlat, &dlng);
    
    // Perform Euler numerical integration (is RK4 worth it ? idk)
    state_t next;
    next.lng = curr.lng + (du / dlng);
    next.lat = curr.lat + (dv / dlat);
    next.alt = curr.alt - timestep * DRAG * fisqrtf(get_density(curr.alt));
    next.time= curr.time + (timestep / 3600.f);
    return next;
}

#define KML_FILE "./kml/kml1.kml"

int main() {
    // Weather data files loading
    FILE *f_arpege = fopen("./bin/arpege.bin", "rb");
    FILE *f_arome  = fopen("./bin/arome.bin", "rb");
    
    // Initial state
    state_t gondola = {
        .lng = 5.9572,  // Initial longitude
        .lat = 43.744,  // Initial latitude
        .alt = 30000,   // Initial altitude (in meters)
        .time= 0.0      // Time after weather model reference time (in hours, must be positive)
    };
    
    // KML file initialization. All KML stuff is optional but nice for visualization in Google Earth
    FILE *kml = fopen(KML_FILE, "w");
    start_kml(kml);
    
    int i;
    float timestep = 30.0;
    
    // Actual path computation
    for(i = 0; i < 10000 && gondola.alt > 0 && gondola.alt < 40000; i++) {
        fprintf(kml, "%f,%f,%.2f\n", gondola.lng, gondola.lat, gondola.alt);    // Print gondola's location in KML file
        
        
//        gondola = update(f_arpege, gondola, arpege, timestep);   // Use ARPEGE model only (less resolution)
//        gondola = update(f_arome, gondola, arome, timestep);     // Use AROME model only (more precise but available only below ~16km altitude)
        
        
        // Use ARPEGE until AROME is available
        if (get_pressure(gondola.alt) < 10.0) {
            gondola = update(f_arpege, gondola, arpege, timestep); 
        }
        else {
            gondola = update(f_arome, gondola, arome, timestep);
        }

    }
    
    printf("OK\n");
    
    // Write KML footer
    finish_kml(kml);
    
    // Close all files
    fclose(kml);
    fclose(f_arpege);
    fclose(f_arome);
    
    // Launch Google Earth
    system("start ./" KML_FILE);
    
    return 0;
}

