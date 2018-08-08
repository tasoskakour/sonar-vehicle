/*
 * Obstacle Avoidance Vehicle.c
 *
 * Created: 20/11/2016 1:44:12 μμ
 * Author : Tacos
 * 
 * A vehicle that avoids obstacles with sonar.
 */

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "main.h"
#include "drv8835.h"
#include "servo_timer0.h"

volatile uint8_t initDutyCycle = STARTING_SPEED;
volatile uint8_t vehicleEnableMovementFlag = 0, speedTableIndex = STARTING_SPEEDindex;

int main(void)
{
	motorDriver_init();
	servoT0_init();
	sonar_init();
	/* Vehicle is moving and avoiding obstacles */
	while (1)
	{
		vehicleExplorate();
	}
}

/**
 * Function: sonar_init
 * -------------------
 * Initializes sonar, timer2 is used. Can measure up to 2.7 meters (~ 16.3ms)
 * 		- CTC Mode
 *		- OCR2A = 0xFF
 *		- Prescaler = 1024
 *		- No interrupts
 *
 */
void sonar_init(void)
{
	/* Adjust Data Direction Registers */
	TRIGGER_ddr |= (1 << TRIGGER_bit);
	ECHO_ddr &= ~(1 << ECHO_bit);

	/* Set Timer2 */
	TCCR2A = 0b00000010;
	TCCR2B = 0b00000111;
	OCR2A = 0xFF;

	TCNT2 = 0;
	TIMSK2 = 0;

	return;
}

/**
 * Function: sonarDistanceDetected
 * -------------------------------
 * Returns distance from sonar sensor at object detected (in cm)
 * 
 */
double sonarDistanceDetected(void)
{
	double timer;

	/* Shoot supersonic wave */
	TRIGGER_port &= ~(1 << TRIGGER_bit);
	_delay_us(2);
	TRIGGER_port |= (1 << TRIGGER_bit);
	_delay_us(10);
	TRIGGER_port &= ~(1 << TRIGGER_bit);

	//cli();  // if interrupts are enabled

	/* Wait for echo to go high */
	while (((ECHO_pin >> ECHO_bit) & 1) == 0)
	{
	};

	TCNT2 = 0;

	/* Countdown Started, wait for echo wave */
	while ((ECHO_pin >> ECHO_bit) & 1)
	{
	};

	/* Convert timer2 clock cycles to cm */
	timer = TCNT2;		// store cycles
	timer = 64 * timer; // cycles to uS
	timer = timer / 58; // uS to cm

	//sei(); // if interrupts are enabled
	_delay_ms(60); //safety for other measurement

	return timer;
}

/**
 * Function: vehicleExplorate
 * --------------------------
 * Controls Vehicle Movement
 *  
 */
void vehicleExplorate(void)
{
	/* Move forward until object is detected in under 20cm */
	moveForward();

	/* Move servo fast right and left and collect distances while moving forward */
	while (1)
	{
		servoSetDeg(0);
		_delay_ms(200);
		if (sonarDistanceDetected() < FORWARD_DISTANCE_COLLISION)
			break;

		servoSetDeg(45);
		_delay_ms(200);
		if (sonarDistanceDetected() < FORWARD_DISTANCE_COLLISION)
			break;

		servoSetDeg(0);
		_delay_ms(200);
		if (sonarDistanceDetected() < FORWARD_DISTANCE_COLLISION)
			break;

		servoSetDeg(-45);
		_delay_ms(200);
		if (sonarDistanceDetected() < FORWARD_DISTANCE_COLLISION)
			break;
	}

	/* Object detected */
	avoidCollision();

	return;
}

/**
 * Function: avoidCollision
 * ------------------------
 * Avoids collision with obstacle
 * 
 */
void avoidCollision(void)
{
	/* At that point an object is detected. */
	vehicleStop();
	double objectDistance[2];

	/* Look right and left and collect distances */
	objectDistance[LEFT_distance] = lookRightLeft(LEFT_distance);
	objectDistance[RIGHT_distance] = lookRightLeft(RIGHT_distance);

	/* Restore servo position */
	servoSetDeg(0);
	_delay_ms(200);

	/* Decide next move */
	if (objectDistance[LEFT_distance] > objectDistance[RIGHT_distance])
	{
		/* Move left till space is freed */
		do
		{
			turnLeft();
		} while (sonarDistanceDetected() < (FORWARD_DISTANCE_COLLISION + 20));
	}
	else
	{
		/* Move right till space is freed */
		do
		{
			turnRight();
		} while (sonarDistanceDetected() < (FORWARD_DISTANCE_COLLISION + 20));
	}

	vehicleStop();

	return;
}

/**
 * Function: lookRightLeft
 * ----------------------
 * Looks right or left and returns distance of obstacle
 * 
 * direction: right or left
 * 
 * returns distance in cm
 * 
 */
double lookRightLeft(uint8_t direction)
{
	double dis;
	if (direction == LEFT_distance)
	{
		/* Move servo left */
		servoSetDeg(90);
	}
	else
	{
		/* Move servo right */
		servoSetDeg(-90);
	}

	/* Collect distance */
	_delay_ms(500); // wait for servo to adjust
	dis = sonarDistanceDetected();

	return dis;
}

/**
 * Function: moveForward
 * ---------------------
 * Moves vehicle forwards
 * 
 */
void moveForward(void)
{

	motorA_setPhase(1);
	motorB_setPhase(0);

	motors_setDutyCycle(initDutyCycle);

	return;
}

/**
 * Function: moveBackward
 * ---------------------
 * Moves vehicle backwards
 * 
 */
void moveBackward(void)
{

	motorA_setPhase(0);
	motorB_setPhase(1);

	motors_setDutyCycle(initDutyCycle);

	return;
}

/**
 * Function: turnRight
 * ---------------------
 * Moves vehicle right
 * 
 */
void turnRight(void)
{

	motorA_setPhase(0);
	motorB_setPhase(0);

	motors_setDutyCycle(initDutyCycle);

	return;
}

/**
 * Function: turnLeft
 * ---------------------
 * Moves vehicle left
 * 
 */
void turnLeft(void)
{

	motorA_setPhase(1);
	motorB_setPhase(1);

	motors_setDutyCycle(initDutyCycle);

	return;
}

/**
 * Function setSpeed
 * -----------------
 * Change speed accordingly. (Duty cycle of timer1 PWM is changed)
 * 
*/
void setSpeed(uint8_t speedSelect)
{
	switch (speedSelect)
	{
	case 0:
		/* Decrease speed */
		if (speedTableIndex >= 1)
			speedTableIndex--;
		initDutyCycle = speedTable[speedTableIndex];
		break;
	case 1:
		/* Increase speed */
		if (speedTableIndex <= (SPEEDS_NUM - 2))
			speedTableIndex++;
		initDutyCycle = speedTable[speedTableIndex];
		break;
	}

	// Check if vehicle is stopped
	uint16_t ocr_val = ((OCR1AH << 8) || OCR1AL);
	if (ocr_val != 0)
		motors_setDutyCycle(initDutyCycle);

	return;
}

/**
 * Function: vehicleStop
 * --------------------
 * Vehicle stops.
 * 
 */
void vehicleStop(void)
{
	motors_setDutyCycle(0);
	return;
}