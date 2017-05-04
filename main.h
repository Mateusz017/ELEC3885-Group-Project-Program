/**
@file main.h
@brief ʜeader file declares functions and variables
@brief Wireless robotic capsule for selective drug delivery - Group design Project
@brief Revision 1.0.
@author Mateusz Loboda
@Date 21/04/17
*/

#ifndef MAIN_H
#define MAIN_H
#include "STEPPER.h"
#include "ACTUATOR.h"
#include "TMP102.h"

#define CHAMBER_1 1
#define CHAMBER_2 2
#define CHAMBER_3 3


/**
@namespace temperature sensor
@brief Object of the TMP102 class
*/
TMP102 tmp102(I2C_SDA,I2C_SCL,PTC10); //change ptb23 ptc12

/**
@namespace stepper motor
@brief Object of the stepper motor class
*/
STEPPER stepper(PTC3,PTC2,PTA2,PTB23);

/**
@namespace linear actuator 
@brief Object of the actuator class
*/
ACTUATOR actuator(PTC8,PTC1,PTB19,PTB18); // 1a , 1b , 2a , 2b (PTB2,PTB3,PTB10,PTB11);

/**
@namespace linear actuator 2
@brief Object of the actuator class
*/
ACTUATOR actuator2(PTC5,PTC7,PTC0,PTC9); //PTC5,PTC7,PTC0,PTC9

/**
@namespace linear actuator 
@brief Object of the actuator class
*/
ACTUATOR actuator3(PTB2,PTB3,PTB10,PTB11);//PTC8,PTC1,PTB19,PTB18 IC5

/**
@namespace raw serial pc 
@brief Object of the rawserial class
*/
RawSerial pc(USBTX, USBRX);

/**
@namespace uart interface
@brief Object of the raw serial class
*/
RawSerial device(PTC17, PTC16);

/**
@namespace r_led
@brief Output for status of red LED
*/
DigitalOut r_led(LED_RED);

/**
@namespace g_led
@brief Output for status of green LED
*/
DigitalOut g_led(LED_GREEN);

/**
@namespace b_led
@brief Output for status of blue LED
*/
DigitalOut b_led(LED_BLUE);

/**
The main function where the code is executed
*/
int main();

/**
Receives commands from the BLE and provides feedback
*/
void device_Rx();

/**
Performs drug delivery by controlling the motor and linear actuator
*/
void delivery();

/**
Provides final feedback once drug delivery is complete
*/
void deliveryFeedback();

//variables
int drugAmount_int;  /*!< Convert the drug amount ascii character obtained over UART to integer */

int chamber_int;  /*!< Convert the chamber ascii character obtained over UART to integer */

int drugAmountFeedback = 4;  /*!< Contains information about the remaining amount of drug 1 after delivery for feedback */

int drugAmountFeedback2 = 4;  /*!< Contains information about the remaining amount of drug 2 after delivery for feedback */

int drugAmountFeedback3 =4;   /*!< Contains information about the remaining amount of drug 3 after delivery for feedback */

volatile int chamber;      /*!< Gets chamber number i.e. type of drug */

volatile int drugAmount;   /*!< Gets drug amount number i.e. type of drug */

bool stepperRx;       /*!< Stepper motor bool indicates that chamber number has been received to start drug delivery */

bool actuatorRx;       /*!< Universal linear Actuator bool indicates that drug amount number has been received to start drug delivery */

float temperature;    /*!< Stores base temperature as a reference for temperature threshold */

int current_drug_chamber = CHAMBER_1;  /*!< Starting position of motor is chamber 1 */

#endif