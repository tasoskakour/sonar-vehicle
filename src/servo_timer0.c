/*
 * servo_timer0.c
 *
 * Created: 20/11/2016 1:08:08 μμ
 * Author : Tasos Kakouris
 * 
 * A Library to control your servo from timer0
 */

#include <avr/io.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <util/delay.h>
#include <math.h>
#include "servo_timer0.h"

/**
 * Function: servoT0_init
 * ---------------------
 * Inits Timer0 with settings:
 * 		- Phase Correct Mode
 * 		- OCR0A as TOP
 * 		- 1024 Prescaler
 * 		- Frequency of PWM = ~50kHZ
 *		- OCR0B produces PWM and determines duty cycle
 *
 */
void servoT0_init(void)
{
	TCCR0A = 0b00100001;
	TCCR0B = 0b00001101;
	OCR0A = TIMER0_PWM_TOP;

	TCNT0 = 0;
	OCR0B = 0;

	/* Servo port as output */
	SERVO_ddr |= (1 << SERVO_bit);

	/* Servo at initial position */
	servoSetDeg(0);
	_delay_ms(500);

	return;
}

/**
 * Function: servoReset
 * --------------------
 * Resets servo to initial position
 */
void servoReset(void)
{
	OCR0B = servoInitPosOCR;
	return;
}

/**
 * Function: servoSetDeg
 * ---------------------
 * Sets servo at a specific rotational position according to degrees.
 * 		Due to limited resolution of Timer0, servo can only move at specific angles
 * 
 * degrees: Degrees to rotate (-90, 90)
 */
void servoSetDeg(double degrees)
{
	double dutyCycle;
	if (degrees > 0)
	{
		/* Left Rotation: Linear Interpolation between 0deg and 90deg */
		double A = (double)(degrees / 90);
		dutyCycle = (double)(A * 12.1 + (1 - A) * 7.5);
	}
	else if (degrees < 0)
	{
		/* Right Rotation Linear Interpolation between 0deg and -90deg */
		double A = (double)((degrees + 90) / 90);
		dutyCycle = (double)(A * 7.5 + (1 - A) * 3.8);
	}
	else
	{
		/* Initial position */
		servoReset();
		return;
	}

	/* Finally adjust OCR */
	OCR0B = (uint8_t)(round(dutyCycle / 100 * TIMER0_PWM_TOP));

	/* Check if servo stayed at initial position */
	if (OCR0B == servoInitPosOCR)
	{
		if (dutyCycle > servoInitDC)
		{
			OCR0B++;
		}
		else
		{
			OCR0B--;
		}
	}

	return;
}

/**
 * Function: servoScan
 * -------------------
 * Rotates servo from left to right, or right to left in certain time period.
 * 
 * direction: 0 for clockwise, 1 for anti-clockwise
 * rotationAngleTime: time for servo to be locked in positions at scan (ms)
 * 
 */
void servoScan(uint8_t direction, double rotationAngleTime)
{
	/* Set starting position and ending position according to direction */
	uint8_t startingPos = servoAllLeftPosOCR;
	int8_t offset = -1;
	if (direction)
	{
		startingPos = servoAllRightPosOCR;
		offset = 1;
	}
	OCR0B = startingPos;
	for (uint8_t i = 0; i < (servoDiscretePositions - 1); i++)
	{

		delayXms(rotationAngleTime);
		OCR0B += offset;
	}
	return;
}

/**
 * Function: delayXms
 * ------------------
 * Delays for X ms
 * 
 * x: in ms
 * 
 */
void delayXms(double x)
{
	for (uint8_t l = 0; l < x; l++)
	{
		_delay_ms(1);
	}
	return;
}
