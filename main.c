/**
 * rigol.c - a simple terminal program to control Rigol DS1000 series scopes
 *
 * Copyright (C) 2010 Mark Whitis
 * Copyright (C) 2012 Jiri Pittner <jiri@pittnerovi.com>
 * Copyright (C) 2013 Ralf Sternberg <ralfstx@gmail.com>
 *
 * Improvements by Jiri Pittner:
 *   readline with history, avoiding timeouts, unlock scope at CTRL-D,
 *   read large sample memory, specify device at the command line
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see [http://www.gnu.org/licenses/].
 */


 /* You have to have a program/library called readline installed for this program to work. Then, in the build options, add the -lreadline compiler flag.
 Then, add the readline library to the linker. the libraries are the libreadline.a, libreadline.so , as well as any other file called libreadline in /usr/lib/x86_64-linux-gnu/ */

 /* On my machine I also needed a library called tinfo. Same procedure as readline*/

 /* If you get something like usbtmc0: Permission denied, you have to enable a thing called "udev permissions" on your user account as per
 http://falsecolour.com/aw/computer/udev-usbtmc/index.html. Or see howto_udev_permissions.txt */


#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "connection.h"

#define STRING_BUFF_LEN 512
#define NUMBER_OF_CHANNELS 4


int main(int argc, char **argv)
{

    /*Initiate connection */
	char device[STRING_BUFF_LEN] = "/dev/usbtmc0";
	if (argc > 2 && strncmp(argv[1], "-D", 2) == 0) {
		strcpy(device, argv[2]);
		argc -= 2;
		*argv += 2;
	}
	struct connection con;// con is the handle for the connection I guess
	con_open(&con, device);
	if (con.fd < 0) {
		fprintf(stderr, "Could not open device '%s': %s\n", device,
				strerror(errno));
		exit(1);
	}



    uint8_t i;
    char temp_string[STRING_BUFF_LEN];

    /*Prompt user for sample interval and number of samples*/
    int32_t sample_interval_sec, datapoints_to_acquire;
    printf("Please enter the desired number of samples:\n");
    prompt_for_number(&datapoints_to_acquire);
    printf("Please enter the desired sample interval in seconds:\n");
    prompt_for_number(&sample_interval_sec);
    printf("The program will print %ld points at %ld seconds interval.",datapoints_to_acquire,sample_interval_sec);

     /* Turning ON all channels and setting trigger to AUTO*/
    for (i = 1; i <= NUMBER_OF_CHANNELS; i++) {
        snprintf(temp_string, STRING_BUFF_LEN, ":CHANnel%d:DISPlay 1", i);
        con_send(&con, temp_string);
    }
    con_send(&con,":TRIGger:SWEep AUTO");
    sleep(3);    /* Wait a bit to acquire some waveforms */



     /* Print a list of the channels */
    printf("\n");
    printf("time,CH1,CH2,CH3,CH4\n");


    /*Query and print units*/
    printf("seconds,");

    for (i = 1;i <= NUMBER_OF_CHANNELS;i++) {
        snprintf(temp_string, STRING_BUFF_LEN, ":CHAN%d:UNIT?", i);

        con_send(&con, temp_string);
        con_recv(&con);
        printf("%.*s", (strlen(con.buffer) - 1), con.buffer);
        if (i < NUMBER_OF_CHANNELS) {
            printf(",");
        }

    }
    printf("\n");



    double temp_float = 0;

    /*Query and print probe ratio*/
    printf("1X,");
    for (i = 1; i <= NUMBER_OF_CHANNELS; i++) {
        snprintf(temp_string, STRING_BUFF_LEN, ":CHAN%d:PROBe?", i);

        con_send(&con, temp_string);
        con_recv(&con);
        sscanf((const char*)con.buffer, "%lf", &temp_float);
        printf("%gX", temp_float);
        if (i < NUMBER_OF_CHANNELS) {
            printf(",");
        }

    }
    printf("\n");


     time_t time_current;
     time(&time_current);
     time_t time_last_sample = 0;

     int32_t datapoints_left_to_acquire = datapoints_to_acquire;


     while (datapoints_left_to_acquire > 0) {
         datapoints_left_to_acquire--;

         while (!(time_current >= (time_last_sample + sample_interval_sec))) { /*While the time to get a new sample hasn't arrived yet*/
             time(&time_current); /* Check the time continuously*/
         }
         time_last_sample = time_current;

            /*Once the time for a new sample has arrived...*/
            /*Print one data point (time plus all channels)*/

         printf("%ld", time_current); /*Print the time. %ld to print long decimal*/
         printf(",");
         for (i = 1; i <= NUMBER_OF_CHANNELS; i++) {

             snprintf(temp_string, STRING_BUFF_LEN, ":MEAS:ITEM? VAVG,CHAN%d", i);
             con_send(&con, temp_string);
             con_recv(&con);
             printf("%.*s", (strlen(con.buffer)- 1), con.buffer); /*Use of strlen-1 to truncate the \n at the end of the data received from the scope*/

             if (i < NUMBER_OF_CHANNELS) {
                 printf(",");
             }

         }
         printf("\n");

         sleep(sample_interval_sec-1); /*Sleep for a little less than the sample period*/
     }


	con_close(&con);
	exit(EXIT_SUCCESS);

}


int32_t prompt_for_number(int32_t* number){


    /*Copied from: http://sekrit.de/webdocs/c/beginners-guide-away-from-scanf.html */
    /* Thank you Felix Palmen*/

    char buf[STRING_BUFF_LEN]; // use 1KiB just to be sure
    int success; // flag for successful conversion

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

}
