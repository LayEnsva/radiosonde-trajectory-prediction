# Radiosonde Trajectory Prediction Algorithm

Radiosonde trajectory prediction algorithm using Météo-France Arpege and Arome weather models. 

## Prerequisites 

- GCC compiler
- [WGRIB2](https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/) command line tool
- [Météo-France](https://meteo.data.gouv.fr/) weather forecast files (.grib2 files available in direct download)
    + Arpège 0.25° IP1 package (~1.0 GB)
    + Arôme 0.025° IP1 package (~0.5 GB)

## Preparation

GRIB2 files are usually very large, compressed, and contain more information than needed. Thus, before running the simulation, they need to be extracted in binary. To do so, the WGRIB2 command line tool will be used. Here are some example commands that can be used.


```shell
wgrib2 arpege_025_IP1_[...].grib2 -match ":(UGRD|VGRD):" -no_header -lola "3:20:0.25" "43:20:0.25" arpege.bin bin
```

Explanation

- `wgrib` calls the tool
- `arpege_025_IP1_[...].grib2` is the input weather data file 
- `-match` allows to select only some of the data matching a POSIX regex
- `":(UGRD|VGRD):"` is a regex meaning "zonal wind or meridional wind"
- `-no_header` specifies that the output binary file does not contain any header, only the data
- `-lola` is a command to select a smaller geographical zone than what the original file contains. This is very useful knowing that Météo-France provides data for entire Europe, whereas a weather balloon only fly over a few hundreds of kilometers.
- `"3:20:0.25" "43:20:0.25"` is the size of the box with format `'coordinate' : 'number of points' : 'resolution (in degrees)'`
- `arpege.bin` is the output file
- `bin` specifies that output is binary