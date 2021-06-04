#ifndef _USER_IO_H_
#define _USER_IO_H_

#include "main.h"
#include "connection.h"


void select_measurements(measurement_t *, uint8_t *, bool *);
void select_measurement_type(measurement_t *, uint8_t, bool *);
void select_measurement_channel(measurement_t *);
void prompt_channel_and_write_string(char *);
void print_measurements(measurement_t *, uint8_t);
int32_t prompt_for_number(int32_t *);
void generate_filename(char *);
void write_file_header(FILE *, measurement_t *, struct connection *, uint8_t);

#endif
