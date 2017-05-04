/*
* @file main.cpp
* @Group Design Project
* @brief Main file containing functions and int main().
* @author Mateusz Loboda
* @date April 2017
*/

#include "main.h"

int main()
{
    temperature = tmp102.read_temperature(); // read base temperature
    pc.printf("\nBASE T = %.2f ", temperature); // print to cool term
    tmp102.low_threshold(temperature + 0.5F); //setting low temperature threshold
    tmp102.high_threshold(temperature + 1); //setting high temperature threshold
    device.attach(&device_Rx,Serial::RxIrq); //serial interrupt to receive data when it is sent
    tmp102.init(); // initialise TMP102 temperature sensor
    pc.baud(9600);
    device.baud(9600); //make sure baud rate matches cool term setting

    while(1) {

        sleep();
    }
}

void device_Rx()
{
    char buff[50];     //array for any data received over UART
    char str[20];      //array for debugging so can print received messages in coolterm
    if (device.readable()) {   //if BLE is sending any data

        char operation = device.getc();  //get the data and store in operation
        buff[0]=operation;               //stores first character of data in buffer

        if (operation == '?') {          //if ? character received the next character refers to drug chamber
            chamber = device.getc();     //get chamber number i.e. type of drug
            chamber_int = chamber - 48;  //convert the ascii character obtained over UART to integer
            stepperRx = 1;               //set Bool to 1 for drug delivery
            buff[1]=chamber;             //for cool term feedback
            sprintf(str,"%c",buff[1]);   //print chamber number
            pc.printf(str);
        } else if (operation == '!') {   //if K64F receives ! the next character refers to the amount of drug that will be dispensed

            drugAmount = device.getc();  //receives drug amount character 1,2,3,4 corresponding to 25%, 50%, 75%, 100&%
            actuatorRx = 1;              //set actuator Bool to 1
            buff[1]=drugAmount;          //for cool term feedback
            sprintf(str,"%c",buff[1]);
            pc.printf(str);
            drugAmount_int = drugAmount - 48; //convert ascii character obtained via UART to integer
            char sendString[6];
            sprintf(sendString,"?%d!%d00",chamber_int,drugAmount_int);
            device.puts(sendString);     //echo the message back to the app and wait for confirmation
            pc.printf(sendString);       //print the drug delivery message
        } else if (operation == 'o') {   //confirmation received, the message has been received correctly
            pc.printf("ok");
            delivery();                  // deliver the drug
        } else if (operation == 't') {   // app is asking for temp feedback
            temperature = tmp102.read_temperature(); // read temperature from sensor
            char feedback[6];
            sprintf(feedback,"t%.2f",temperature);
            device.puts(feedback);       // send the temperature string to the app
        } else if (operation == 'd') {   // app is asking for feedback of each drug chamber
            char message[6];
            sprintf(message,"d%d%d%d00",drugAmountFeedback,drugAmountFeedback2,drugAmountFeedback3); //drug amounts are represented with integers 1-4, one for each chamber
            device.puts(message);        // send to the app
        } else if (operation == 'h') {   // app is requesting threshold temperature feedback
            int feedbackAlert = tmp102.alert_status();  // obtain the threshold flag
            char message[6];
            sprintf(message,"h%d0000",feedbackAlert);
            device.puts(message);
            pc.printf("feedback temperature"); //send the threshold feedback

        }

    }
}

void delivery()
{
    pc.printf("chamber\n");
    pc.printf("%i, %i,%i", stepperRx, actuatorRx); //printing Bools

    if (stepperRx == 1 && actuatorRx == 1) {   //checks if drug type and amount has been received

        current_drug_chamber = stepper.rotateChamber(chamber_int);  //rotate now if 1 do nothing, if 2 do 120 degrees if 3 240 degrees then return to chamber 1
        pc.printf("FORWARDS\n"); // print to serial connection
        pc.printf("%i", drugAmount_int); //expected amount to be released
        if (chamber_int == 1) {  //if chamber_int = 1 release drug from LA 1
            actuator.actuate(drugAmount_int*8); // actuates a distance of 37 mm -40 to 40m
            wait(2); // delay of 2 seconds during release
            pc.printf("BACKWARDS\n"); // print to serial connection
            actuator.actuate(drugAmount_int*(-8));  //retracting by the same amount using negative value -10
            drugAmountFeedback = drugAmountFeedback - drugAmount_int; //100% - amount released
            deliveryFeedback(); //send feedback to the app

        } else if (chamber_int == 2) { //if chamber_int = 2 release drug from LA 2
            actuator2.actuate(drugAmount_int*8);
            wait(2); // delay of 5 seconds
            pc.printf("BACKWARDS\n"); // print to serial connection
            actuator2.actuate(drugAmount_int*(-8)); //backwards by x mm   //retracting by the same amount using negative value
            drugAmountFeedback2 = drugAmountFeedback2 - drugAmount_int;
            deliveryFeedback();
        } else if (chamber_int == 3) {  //if chamber_int = 3 release drug from LA 3
            actuator3.actuate(drugAmount_int*8);
            wait(2); // delay of 2 seconds
            pc.printf("BACKWARDS\n"); // print to serial connection
            actuator3.actuate(drugAmount_int*(-8)); //backwards by x mm   //retracting by the same amount using negative value
            drugAmountFeedback3 = drugAmountFeedback3 - drugAmount_int;
            deliveryFeedback(); //this sends feedback before chamber is retracted
        }

        pc.printf("ROTATE STEPPER MOTOR TO CHAMBER 1\n"); // print to serial connection
        current_drug_chamber = stepper.rotateChamber(CHAMBER_1);  //can return to drug chamber 1 at the end for reference
        wait(2); // delay of 2 seconds
        stepperRx = 0; //resetting both bools before the next command is received
        actuatorRx = 0;
        pc.printf("%i, %i", stepperRx, actuatorRx); //printing Bools

    }

}
void deliveryFeedback()
{
    char sendString[6];
    sprintf(sendString,"s00000"); // send this as feedback when drug delivery is complete
    device.puts(sendString);
    pc.printf(sendString);
}
