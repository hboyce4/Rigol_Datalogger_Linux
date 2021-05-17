#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdbool.h>

#define STRING_BUFF_LEN 512
#define NUMBER_OF_CHANNELS 4
#define MAX_MEASUREMENTS 8

typedef struct {
    char type_scope_command_str[STRING_BUFF_LEN];
    char type_human_readable_str[STRING_BUFF_LEN];
    bool is_time_based;

    char source_A_str[STRING_BUFF_LEN];
    char source_B_str[STRING_LBUFF_LEN];// Source B is only needed for delay and phase measurements

}measurement_t;



int32_t prompt_for_number(int32_t*);
void generate_filename(char*);
void select_measurement_type(measurement_t*);
void select_measurements(measurement_t*, uint8_t*, bool*);

#endif // MAIN_H_INCLUDED

