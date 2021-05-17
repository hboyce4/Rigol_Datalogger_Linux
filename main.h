#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdbool.h>

#define STRING_BUFF_LEN 512
#define NUMBER_OF_CHANNELS 4

typedef struct {
    char scope_command_str[STRING_BUFF_LEN];
    char human_readable_str[STRING_BUFF_LEN];
    bool is_time_based;
}measurement_properties_t;




int32_t prompt_for_number(int32_t*);
void generate_filename(char*);
void select_measurement_type(measurement_properties_t*);


#endif // MAIN_H_INCLUDED

