
#ifndef SERVO_TIMER0_H
#define SERVO_TIMER0_H

#include <inttypes.h>

/* Port Definitions. Must be defined as OC0B */
#define SERVO_ddr DDRD
#define SERVO_port PORTD
#define SERVO_bit PORTD5 //OC0B

/**
 * Duty Cycles and OCR Definitions. 
 * Determines the angle of rotation
 * InitPosition = 7.5% -> OCR0B = 12
 * AllLeft = 12.1% -> OCR0B = 19
 *AllRight = 3.8% -> OCR0B = 6
 */
#define TIMER0_PWM_TOP 156
#define servoInitDC 7.5
#define servoAllLeftDC 12.1
#define servoAllRightDC 3.8
#define servoInitPosOCR 12
#define servoAllRightPosOCR 6
#define servoAllLeftPosOCR 19

#define servoDiscretePositions (servoAllLeftPosOCR - servoAllRightPosOCR + 1)

/* Functions */
void servoT0_init(void);
void servoReset(void);
void servoSetDeg(double degrees);
void servoScan(uint8_t direction, double rotationAngleTime);
void delayXms(double x);

#endif