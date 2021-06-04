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

#include "main.h"
#include "connection.h"
#include "user_io.h"


int main(int argc, char **argv)
{

    /********************************************************************Initiate connection  start *****************************************************/

	char device[STRING_BUFF_LEN] = "/dev/usbtmc1"; // sometimes appears as usbtmc0. Change this line if that's the case
	if (argc > 2 && strncmp(argv[1], "-D", 2) == 0) {
		strcpy(device, argv[2]);
		argc -= 2;
		*argv += 2;
	}
	struct connection con;// con is the handle for the connection I guess
	con_open(&con, device);
	if (con.fd < 0) {
		fprintf(stderr, "Could not open device '%s': %s\n", device, strerror(errno));
		//exit(1);
	}



    uint8_t i;
    char temp_string[STRING_BUFF_LEN];

    con_send(&con,"*IDN?");
    con_recv(&con);
    printf("Scope found over USB: %s\n",con.buffer);
    /********************************************************************Initiate connection end *****************************************************/


    /******************************************************************** User prompt start *****************************************************/
    /*Prompt user for sample interval, number of samples and measurement type*/
    int32_t sample_interval_sec, datapoints_to_acquire;
    measurement_t measurements_table[MAX_MEASUREMENTS]; // Maximum number of measurements is 8 (arbitrary, scope can probably take more)
    uint8_t nb_of_measurements;
    bool active_channels_table[5]; // Scope is 4 channels + MATH

    printf("Please enter the desired number of samples:\n");
    prompt_for_number(&datapoints_to_acquire);
    printf("Please enter the desired sample interval in seconds:\n");
    prompt_for_number(&sample_interval_sec);

    select_measurements(measurements_table, &nb_of_measurements, active_channels_table);


    printf("The program will save ");
    printf("%" PRId32,datapoints_to_acquire);
    printf(" points at ");
    printf("%" PRId32, sample_interval_sec);
    printf(" seconds interval of these measurements:\n");
    print_measurements(measurements_table, nb_of_measurements);

    /******************************************************************** User prompt end *****************************************************/


    /******************************************************************** Scope setup start *****************************************************/
     /* Turning ON all channels and setting trigger to AUTO*/
    for (i = 1; i <= NUMBER_OF_CHANNELS; i++) {
        snprintf(temp_string, STRING_BUFF_LEN, ":CHANnel%d:DISPlay 1", i);
        con_send(&con, temp_string);
    }
    con_send(&con,":TRIGger:SWEep AUTO");
    sleep(3);    /* Wait a bit to acquire some waveforms */
    /******************************************************************** Scope setup end *****************************************************/


    /******************************************************************** File initialization start *****************************************************/
    /* Generate the filename*/
    char filename[STRING_BUFF_LEN];
    generate_filename(filename);


    /* Create the file and open it in write mode*/
    FILE *fptr;
    fptr = fopen(filename, "w");
    if (fptr == NULL) {/* If null pointer is returned by fopen because it could not open the file*/
        printf("Error! Could not open file.");
        exit(1);
    }

    printf("Writing file header...\n");
    write_file_header(fptr, measurements_table, &con, nb_of_measurements);

    /******************************************************************** File initialization end *****************************************************/


    /******************************************************************** Acquisition start *****************************************************/
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
        for (i = 0; i < nb_of_measurements; i++) {

             snprintf(temp_string, STRING_BUFF_LEN, ":MEAS:ITEM? %s,%s", measurements_table[i].type_scope_command_str, measurements_table[i].source_A_str);/*Query the measurement to the scope. str_measurement_type contains the meas. type as per the Rigol programming manual */

             if(measurements_table[i].time_property == TIME_BASED_TWO_SOURCES){// If measurement needs 2 sources
                char append_str[STRING_BUFF_LEN];
                snprintf(append_str, STRING_BUFF_LEN, ",%s", measurements_table[i].source_B_str);// copy second source to string after a comma
                strncat(temp_string, append_str, STRING_BUFF_LEN);// Append second source to command

             }

             //printf(temp_string, STRING_BUFF_LEN, ":MEAS:ITEM? %s,CHAN%d", measurement_type.type_scope_command_str, i);// For DEBUG
             con_send(&con, temp_string);
             con_recv(&con);
             fprintf(fptr,"%.*s", (strlen(con.buffer)- 1), con.buffer); /*Use of strlen-1 to truncate the \n at the end of the data received from the scope*/


             fprintf(fptr,",");


        }
        fprintf(fptr,"\n");
        fclose(fptr);

        /* Print the progress status in the terminal*/
        printf("\r");
        printf("%ld/%ld ", (long int)datapoints_acquired, (long int)datapoints_to_acquire);
        fflush(stdout); /* Printf prints nothing if sleep is called right after. Use fflush to prevent that.*/


         sleep(sample_interval_sec-1); /*Sleep for a little less than the sample period*/
    }
    printf("\n");

    printf("Acquisition finished...\n");
	con_close(&con);
	exit(EXIT_SUCCESS);
    /******************************************************************** Acquisition end *****************************************************/
}
