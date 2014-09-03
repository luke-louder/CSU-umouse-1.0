//Bit Address & Register Declartions
#define R_MOTOR_F	LATD1		//H-Bridge for right motor forward
#define R_MOTOR_B	LATD0		//H-Bridge for right motor backward
#define L_MOTOR_F	LATD3		//H-Bridge for left motor forward
#define L_MOTOR_B	LATD2		//H-Bridge for left motor backward

#define MOTORS_LED		LATD	//Motor latch register
#define MOTORS_LED_TRIS	TRISD	//Motor tri-state register

#define BUTTON	RD4
#define SWITCH	RB5

#define RF_SENSOR	RB1		//Right Front Sensor
#define LF_SENSOR	RB0		//Left Front Sensor
#define F_SENSOR	RB3		//Front Sensor
#define L_SENSOR	RB2		//Left Back Sensor
#define R_SENSOR	RB4		//Right Back Sensor

#define SENSORS_TRIS TRISB	//Sensor tri-state register


#define LED_1	LATD5
#define LED_2	LATD6
#define LED_3	LATD7


//#define BLOCK_LENGTH 18		//length of a block in cm

//#define WHEEL_BASE  .7874	//Wheel Base in cm
//#define WHEEL_RAD	.32		//Wheel Radius in cm
//#define RPS			1.87	//Revolutions/Second


//#define PI	3.141592654
