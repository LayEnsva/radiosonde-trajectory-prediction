void start_kml(FILE *kml_file) {
    FILE* kml_header;
    char c;
    
    kml_header = fopen("./kml/kml_header", "r");
    
    while (!feof(kml_header)) {
      c = fgetc(kml_header);
      if (ferror(kml_header)) {
        fprintf(stderr, "ERROR: error reading KML header file\n");
        exit(1);
      }
      if (!feof(kml_header)) fputc(c, kml_file);
      if (ferror(kml_file)) {
        fprintf(stderr, "ERROR: error writing header to KML file\n");
        exit(1);
      }
    }
    
    fclose(kml_header);
}

void finish_kml(FILE *kml_file) {
    FILE* kml_footer;
    char c;
    
    kml_footer = fopen("./kml/kml_footer", "r");
    
    while (!feof(kml_footer)) {
      c = fgetc(kml_footer);
      if (ferror(kml_footer)) {
        fprintf(stderr, "ERROR: error reading KML footer file\n");
        exit(1);
      }
      if (!feof(kml_footer)) fputc(c, kml_file);
      if (ferror(kml_file)) {
        fprintf(stderr, "ERROR: error writing footer to KML file\n");
        exit(1);
      }
    }
    
    fclose(kml_footer);
}
