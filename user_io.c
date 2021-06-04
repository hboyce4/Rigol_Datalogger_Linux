
#include "user_io.h"
#include "main.h"

void select_measurements(measurement_t *measurements_table, uint8_t *nb_of_measurements_ptr, bool *active_channels_table_ptr){

    bool selection_is_finished = false;
    *nb_of_measurements_ptr = 0;

    while (!selection_is_finished && (*nb_of_measurements_ptr < MAX_MEASUREMENTS)){

        select_measurement_type(&measurements_table[*nb_of_measurements_ptr], *nb_of_measurements_ptr, &selection_is_finished);
        if (!selection_is_finished){
            select_measurement_channel(&measurements_table[*nb_of_measurements_ptr]);
            (*nb_of_measurements_ptr)++;
        }

    }

}


void select_measurement_type(measurement_t *measurement, uint8_t nb_of_measurements, bool *selection_is_finished_ptr){

    int32_t measurement_number;
    printf("Please enter the measurement type number for measurement %d:\n", (nb_of_measurements+1));
    printf("0. No more measurements\n");
    printf("1. Average value\n");
    printf("2. RMS value\n");
    printf("3. Pk-pk value\n");
    printf("4. Frequency\n");
    printf("5. Pos. duty cycle\n");
    printf("6. Phase (measured on rising edge)\n");
    printf("7. Delay (measured on rising edge)\n");
    prompt_for_number(&measurement_number);
    //printf("Selected number is %d\n", measurement_type);

    if (measurement_number){

        //(*nb_of_measurements_ptr)++;// One more measurement has been selected
        switch (measurement_number){

            case 1 :// If choice no. 1
                strcpy(measurement->type_scope_command_str, "VAVG");// Average voltage
                strcpy(measurement->type_human_readable_str, "Average");
                measurement->time_property = NOT_TIME_BASED;
                break;

            case 2 :// If choice no. 2
                strcpy(measurement->type_scope_command_str, "VRMS");// RMS voltage
                strcpy(measurement->type_human_readable_str, "RMS");
                measurement->time_property = NOT_TIME_BASED;
                break;

            case 3 :// If choice no. 3
                strcpy(measurement->type_scope_command_str, "VPP");// Peak to peak voltage
                strcpy(measurement->type_human_readable_str, "Pk-pk");
                measurement->time_property = NOT_TIME_BASED;
                break;

            case 4 :// If choice no. 4
                strcpy(measurement->type_scope_command_str, "FREQ");// Frequency
                strcpy(measurement->type_human_readable_str, "frequency");
                measurement->time_property = TIME_BASED;
                strcpy(measurement->unit_str, "Hz");
                break;

            case 5 :// If choice no. 5
                strcpy(measurement->type_scope_command_str, "PDUT");// Positive duty cycle
                strcpy(measurement->type_human_readable_str, "duty cycle");
                measurement->time_property = TIME_BASED;
                strcpy(measurement->unit_str, "D");
                break;

            case 6 :// If choice no. 6
                strcpy(measurement->type_scope_command_str, "RPH");// Positive duty cycle
                strcpy(measurement->type_human_readable_str, "phase");
                measurement->time_property = TIME_BASED_TWO_SOURCES;
                strcpy(measurement->unit_str, "deg");
                break;

            case 7 :// If choice no. 7
                strcpy(measurement->type_scope_command_str, "RDEL");// Positive duty cycle
                strcpy(measurement->type_human_readable_str, "delay");
                measurement->time_property = TIME_BASED_TWO_SOURCES;
                strcpy(measurement->unit_str, "sec");
                break;

            default:
                strcpy(measurement->type_scope_command_str, "VAVG");// Average voltage
                strcpy(measurement->type_human_readable_str, "Average");
                measurement->time_property = NOT_TIME_BASED;

        }

    }else{

        *selection_is_finished_ptr = true;
    }

}

void select_measurement_channel(measurement_t *measurement){

    if(measurement->time_property != TIME_BASED_TWO_SOURCES){

        printf("Please enter the number of the desired channel (1-4, 5 for MATH):\n");
        prompt_channel_and_write_string(measurement->source_A_str);

    }else{

        printf("Please enter the number of the first channel (1-4, 5 for MATH):\n");
        prompt_channel_and_write_string(measurement->source_A_str);

        printf("Please enter the number of the second channel (1-4, 5 for MATH):\n");
        prompt_channel_and_write_string(measurement->source_B_str);
    }


}

void prompt_channel_and_write_string(char *channel_str){

    int32_t selection = 0;
    while ((selection < FIRST_CHANNEL) || (selection > LAST_CHANNEL)){
        prompt_for_number(&selection);
    }
    switch (selection){

        case 1 :// If choice no. 1
            strcpy(channel_str, "CHAN1");
            break;

        case 2 :// If choice no. 2
            strcpy(channel_str, "CHAN2");
            break;

        case 3 :// If choice no. 3
            strcpy(channel_str, "CHAN3");
            break;

        case 4 :// If choice no. 4
            strcpy(channel_str, "CHAN4");
            break;

        case 5 :// If choice no. 5 (MATH)
            strcpy(channel_str, "MATH");
            break;
    }


}


void print_measurements(measurement_t *measurements_table, uint8_t nb_of_measurements){

    uint8_t count = 0;
//    char temp_string[STRING_BUFF_LEN];

    while(count < nb_of_measurements){


        printf("%s,%s,", measurements_table[count].type_human_readable_str, measurements_table[count].source_A_str);
        if(measurements_table[count].time_property == TIME_BASED_TWO_SOURCES){// If two sources
            printf("%s,",measurements_table[count].source_B_str);// Print second source
        }
        printf("\n");
        count++;


    }

}

int32_t prompt_for_number(int32_t *number){


    /*Copied from: http://sekrit.de/webdocs/c/beginners-guide-away-from-scanf.html */
    /* Thank you Felix Palmen*/

    char buf[STRING_BUFF_LEN]; // use big buffer
    int32_t success; // flag for successful conversion

    do
    {

        if (!fgets(buf, STRING_BUFF_LEN, stdin))
        {
            // reading input failed:
            return 1;
        }

        // have some input, convert it to integer:
        char *endptr;

        errno = 0; // reset error number
        *number = strtol(buf, &endptr, 10);
        if (errno == ERANGE)
        {
            printf("Sorry, this number is too small or too large.\n");
            success = 0;
        }
        else if (endptr == buf)
        {
            // no character was read
            success = 0;
        }
        else if (*number < 0)
        {
            printf("Sorry, this number is negative.\n");
            success = 0;
        }
        else if (*endptr && *endptr != '\n')
        {
            // *endptr is neither end of string nor newline,
            // so we didn't convert the *whole* input
            success = 0;
        }
        else
        {
            success = 1;
        }
    } while (!success); // repeat until we got a valid number

    return 0;
}

void generate_filename(char *filename){

    time_t rawtime = time(NULL);

    if (rawtime == -1) {

        puts("The time() function failed");

    }

    struct tm *ptm = localtime(&rawtime);

    if (ptm == NULL) {

        puts("The localtime() function failed");
    }

    sprintf(filename,"Scope Capture %04d-%02d-%02d %02d:%02d:%02d.csv", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

}

void write_file_header(FILE *fptr, measurement_t *measurements_table, struct connection *con, uint8_t nb_of_measurements){

    char temp_string[STRING_BUFF_LEN];
    uint8_t i;
     /* Print a list of the channels */
    fprintf(fptr,"time");
    for(i = 0; i < nb_of_measurements;i++){// For every measurement
        if(measurements_table[i].time_property == TIME_BASED_TWO_SOURCES){// If measurement has two sources
            fprintf(fptr, ",%s-%s", measurements_table[i].source_A_str, measurements_table[i].source_B_str);// Print two sources
        }else{
            fprintf(fptr, ",%s",measurements_table[i].source_A_str);// else print the one source
        }
    }
    fprintf(fptr,"\n");


    /*Query and print units*/
   fprintf(fptr,"Unix Time,");

    for (i = 0;i < nb_of_measurements;i++) {// For all measurements

        if(measurements_table[i].time_property == NOT_TIME_BASED){// If the measurement is not time based, query unit from scope

            if(strcmp(measurements_table[i].source_A_str, "MATH")){// If the channel is not MATH

                snprintf(temp_string, STRING_BUFF_LEN, ":%s:UNIT?", measurements_table[i].source_A_str);
                con_send(con, temp_string);
                con_recv(con);
                fprintf(fptr,"%.*s", (strlen(con->buffer) - 1), con->buffer);

            }else{// else the channel is MATH and we can't query it (the scope doesn't support it...)
                fprintf(fptr,"unknown");
            }

        }else{// else the unit is time based, so we know the unit. Use the unit string from measurement info
            fprintf(fptr,"%s",measurements_table[i].unit_str);
        }

        fprintf(fptr,",");// Comma to separate fields


    }
    fprintf(fptr,"\n");// After all measurement units, new line

    /* Write measurement types*/
    fprintf(fptr,",");// First cell is empty
    for (i = 0;i < nb_of_measurements;i++) {// For all measurements

            fprintf(fptr,"%s,",measurements_table[i].type_human_readable_str);// Read the measurement type from the measurement info struct and write it to file

    }
    fprintf(fptr,"\n");// After all measurement types are written, new line

    /*Query and print probe ratios*/ // Not doing that anymore, not very useful and can be confusing later
    /*
    double temp_float = 0;
    fprintf(fptr,"1X,");
    for (i = 1; i <= NUMBER_OF_CHANNELS; i++) {
        snprintf(temp_string, STRING_BUFF_LEN, ":CHAN%d:PROBe?", i);

        con_send(con, temp_string);
        con_recv(con);
        sscanf((const char*)con->buffer, "%lf", &temp_float);
        fprintf(fptr,"%gX", temp_float);
        if (i < NUMBER_OF_CHANNELS) {
            fprintf(fptr,",");
        }

    }
    fprintf(fptr,"\n");
    */

    fclose(fptr);

}
