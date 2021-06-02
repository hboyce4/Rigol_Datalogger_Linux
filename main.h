#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdbool.h>

#define STRING_BUFF_LEN 512
#define NUMBER_OF_CHANNELS 4
#define MAX_MEASUREMENTS 8
#define FIRST_CHANNEL 1
#define LAST_CHANNEL 5 // last channel (5) is MATH

typedef enum {
    NOT_TIME_BASED,
    TIME_BASED,
    TIME_BASED_TWO_SOURCES
} time_property_t;

typedef struct {
    char type_scope_command_str[STRING_BUFF_LEN];
    char type_human_readable_str[STRING_BUFF_LEN];
    time_property_t time_property;
    char unit_str[STRING_BUFF_LEN];

    char source_A_str[STRING_BUFF_LEN];
    char source_B_str[STRING_BUFF_LEN];// Source B is only needed for delay and phase measurements, where time_property is TIME_BASED_TWO_SOURCES.

}measurement_t;



int32_t prompt_for_number(int32_t *);
void generate_filename(char *);
void select_measurement_type(measurement_t *, uint8_t *, bool *);
void select_measurements(measurement_t *, uint8_t *, bool *);
void select_measurement_channel(measurement_t *);
void prompt_channel_and_write_string(char *);
void print_measurements(measurement_t *, uint8_t);

#endif // MAIN_H_INCLUDED

