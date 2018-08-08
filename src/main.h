
#ifndef MAIN_H
#define MAIN_H

#ifndef F_CPU
#warning "F_CPU undefined, set to 16MHz"
#define F_CPU 16000000UL
#endif

#include <inttypes.h>

/* Sonar Ports Definitions */
#define TRIGGER_ddr DDRB
#define TRIGGER_port PORTB
#define TRIGGER_bit PORTB4
#define ECHO_ddr DDRB
#define ECHO_pin PINB
#define ECHO_bit PINB5

#define LEFT_distance 0
#define RIGHT_distance 1

#define FORWARD_DISTANCE_COLLISION 30

/* Vehicle Speed Definitions */
#define STARTING_SPEED 90
#define STARTING_SPEEDindex 7 // according to starting speed
#define SPEEDS_NUM 9          // how many speeds available

volatile uint8_t speedTable[SPEEDS_NUM] = {25, 35, 45, 55, 65, 75, 85, 90, 95};

/* Main Sub routines declarations */
void sonar_init(void);
double sonarDistanceDetected(void);
void avoidCollision(void);
double lookRightLeft(uint8_t direction);
void vehicleExplorate(void);
void moveForward(void);
void moveBackward(void);
void turnRight(void);
void turnLeft(void);
void setSpeed(uint8_t speedSelect);
void vehicleStop(void);

#endif