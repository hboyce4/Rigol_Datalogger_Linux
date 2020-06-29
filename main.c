/**
 * Rigol_Datalogger_Linux, Copyright (C) 2020 Hugo Boyce
 *
 *
 * Based on rigol.c - a simple terminal program to control Rigol DS1000 series scopes,
 * Originally by:
 * Copyright (C) 2010 Mark Whitis
 * Copyright (C) 2012 Jiri Pittner <jiri@pittnerovi.com>
 * Copyright (C) 2013 Ralf Sternberg <ralfstx@gmail.com>
 * Many thanks to them!!!
 *
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see [http://www.gnu.org/licenses/].
 */


 /* If you get something like "usbtmc0: Permission denied", you have to enable a thing called "udev permissions" on your user account as per
 http://falsecolour.com/aw/computer/udev-usbtmc/index.html. Or see howto_udev_permissions.txt. Many thanks to awebster.*/


#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <time.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "connection.h"
#include "main.h"

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

    con_send(&con,"*IDN?");
    con_recv(&con);
    printf("Scope found over USB: %s\n",con.buffer);


    /*Prompt user for sample interval and number of samples*/
    int32_t sample_interval_sec, datapoints_to_acquire;
    printf("Please enter the desired number of samples:\n");
    prompt_for_number(&datapoints_to_acquire);
    printf("Please enter the desired sample interval in seconds:\n");
    prompt_for_number(&sample_interval_sec);
    printf("The program will save ");
    printf("%" PRId32,datapoints_to_acquire);
    printf(" points at ");
    printf("%" PRId32, sample_interval_sec);
    printf(" seconds interval.\n");



     /* Turning ON all channels and setting trigger to AUTO*/
    for (i = 1; i <= NUMBER_OF_CHANNELS; i++) {
        snprintf(temp_string, STRING_BUFF_LEN, ":CHANnel%d:DISPlay 1", i);
        con_send(&con, temp_string);
    }
    con_send(&con,":TRIGger:SWEep AUTO");
    sleep(3);    /* Wait a bit to acquire some waveforms */

    /* Generate the filename*/
    char filename[STRING_BUFF_LEN];
    generate_filename(filename);


    /* Create the file and open it in write mode*/
    FILE* fptr;
    fptr = fopen(filename, "w");
    if (fptr == NULL) {/* If null pointer is returned by fopen because it could not open the file*/
        printf("Error!");
        exit(1);
    }

    printf("Writing file header...\n");


     /* Print a list of the channels */
    fprintf(fptr,"time,CH1,CH2,CH3,CH4\n");


    /*Query and print units*/
   fprintf(fptr,"seconds,");

    for (i = 1;i <= NUMBER_OF_CHANNELS;i++) {
        snprintf(temp_string, STRING_BUFF_LEN, ":CHAN%d:UNIT?", i);

        con_send(&con, temp_string);
        con_recv(&con);
        fprintf(fptr,"%.*s", (strlen(con.buffer) - 1), con.buffer);
        if (i < NUMBER_OF_CHANNELS) {
            fprintf(fptr,",");
        }

    }
    fprintf(fptr,"\n");


    double temp_float = 0;

    /*Query and print probe ratios*/
    fprintf(fptr,"1X,");
    for (i = 1; i <= NUMBER_OF_CHANNELS; i++) {
        snprintf(temp_string, STRING_BUFF_LEN, ":CHAN%d:PROBe?", i);

        con_send(&con, temp_string);
        con_recv(&con);
        sscanf((const char*)con.buffer, "%lf", &temp_float);
        fprintf(fptr,"%gX", temp_float);
        if (i < NUMBER_OF_CHANNELS) {
            fprintf(fptr,",");
        }

    }
    fprintf(fptr,"\n");
    fclose(fptr);




    printf("Starting acquisition...\n");

     time_t time_current;
     time(&time_current);
     time_t time_last_sample = 0;

     int32_t datapoints_acquired = 0;


     while (datapoints_acquired < datapoints_to_acquire) { /* Main loop that acquires all the points*/
         datapoints_acquired++;

        while (!(time_current >= (time_last_sample + sample_interval_sec))) { /*While the time to get a new sample hasn't arrived yet*/
           time(&time_current); /* Check the time continuously*/
        }
        time_last_sample = time_current;

            /*Once the time for a new sample has arrived...*/
            /*Append one new data point(time plus all channels) to the file*/
        fptr = fopen(filename, "a");
        fprintf(fptr,"%ld", time_current); /*Print the time. %ld to print long decimal*/
        fprintf(fptr,",");
        for (i = 1; i <= NUMBER_OF_CHANNELS; i++) {

             snprintf(temp_string, STRING_BUFF_LEN, ":MEAS:ITEM? VAVG,CHAN%d", i);/*Query the average voltage to the scope. Change this command as per the Rigol programming manual to query a different measurement */
             con_send(&con, temp_string);
             con_recv(&con);
             fprintf(fptr,"%.*s", (strlen(con.buffer)- 1), con.buffer); /*Use of strlen-1 to truncate the \n at the end of the data received from the scope*/

             if (i < NUMBER_OF_CHANNELS) {
                 fprintf(fptr,",");
             }

        }
        fprintf(fptr,"\n");
        fclose(fptr);

        /* Print the progress status in the terminal*/
        printf("\r");
        printf("%ld/%ld ",datapoints_acquired, datapoints_to_acquire);
        fflush(stdout); /* Printf prints nothing if sleep is called right after. Use fflush to prevent that.*/


         sleep(sample_interval_sec-1); /*Sleep for a little less than the sample period*/
    }
    printf("\n");

    printf("Acquisition finished...\n");
	con_close(&con);
	exit(EXIT_SUCCESS);

}


int32_t prompt_for_number(int32_t* number){


    /*Copied from: http://sekrit.de/webdocs/c/beginners-guide-away-from-scanf.html */
    /* Thank you Felix Palmen*/

    char buf[STRING_BUFF_LEN]; // use 1KiB just to be sure
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

void generate_filename(char * filename){

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
