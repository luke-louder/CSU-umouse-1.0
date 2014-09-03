//Includes
#include <pic18.h>
#include <htc.h>
#include <stdio.h>

#include "main.h"
#include "delay.h"

//Prototypes
void TurnLeft(void);
void TurnRight(void);
//void GoForward(void);	//Not used
void SteerRight(void);
void SteerLeft(void);
void TurnAround(void);

//Caveman functions======================================
void TurnLeft(){
	DelayMs(250);
	LATD = 0b00000110;
	DelayMs(500);
	DelayMs(41);
	LATD = 0b00001001;
	DelayMs(10);
	LATD = 0x00;
	DelayMs(250);
}

void TurnRight(){
	DelayMs(250);
	LATD = 0b00001001;
	DelayMs(500);
	DelayMs(38);
	LATD = 0b00000110;
	DelayMs(10);
	LATD = 0x00;
	DelayMs(250);
}

//NOT USED
/*
void GoForward(){
//	DelayMs(1000);
//	DelayMs(1000);
//	DelayMs(500);
	LATD = 0b00001010;
	DelayMs(500);
	DelayMs(500);
	DelayMs(500);
	DelayMs(100);
	LATD = 0;
	//DelayMs(0);
}
*/
void SteerRight(){
	//DelayMs(300);
	DelayMs(30);
	LATD = 0b00000110;
	DelayMs(8);
	LATD = 0;
	DelayMs(80);
}

void SteerLeft(){
	//DelayMs(300);
	DelayMs(30);
	LATD = 0b00001001;
	DelayMs(8);
	LATD = 0;
	DelayMs(80);
}

void TurnAround(){
	DelayMs(250);
	LATD = 0b00001001;
	DelayMs(500);
	DelayMs(500);
	DelayMs(67);
	LATD = 0b00000110;
	DelayMs(10);
	LATD = 0x00;
	DelayMs(250);
}
//===============================End of Caveman Functions