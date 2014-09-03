//Set Clock Speed
#define XTAL_FREQ	8MHZ	//clock frequency

//Includes
#include <pic18.h>	//pic header file
#include <htc.h>	//high tech C header
#include <stdio.h>	
#include <time.h>
#include <stdlib.h>

#include "main.h"	//pin defs
#include "delay.h"	//internet delay func

//Set Fuse Bits
__CONFIG(1, OSC_ECIO6 & FCMEN_OFF & IESO_OFF);	
__CONFIG(2, PWRT_OFF & BOREN_OFF & WDT_OFF);	
__CONFIG(3, PBADEN_OFF & LPT1OSC_OFF & MCLRE_ON);	//PBADEN Changes: analogue pull up resistors on port B
__CONFIG(4, STVREN_OFF & LVP_OFF & XINST_OFF & DEBUG_ON);	//DEBUG changes: ask taylor later
__CONFIG(5, CP0_OFF & CP1_OFF & CP2_OFF & CPB_OFF & CPD_OFF);
__CONFIG(6, WRT0_OFF & WRT1_OFF & WRT2_OFF & WRTC_OFF & WRTB_OFF & WRTD_OFF);
__CONFIG(7, EBTR0_OFF & EBTR1_OFF & EBTR2_OFF & EBTRB_OFF);

//Prototypes=======================
//Maze Solving Functions-----
void MazeSetup(void);	//loads old maze or sets up maze for new run
void SolveMaze(void);	//runtime code to map and solve maze
void EndMaze(void);		//Once centre is found, save maze and return to origin

void CheckWalls(void);	//Check and store what walls are in current cell
void CheckAdjacentCells(short, short);	//once squares are updated, ensure that all squares as consistent for flood fill
//-----Maze Solving Functions

//Various Functions----------
void LED(short);					//turns on led
void ConfigureInterrupts(void);		//enables interrupts for port B
void DisableInterrupts(void);		//disables interrupts for port B
//----------Various Functions

//Memory Functions-----------
void ClearMaze(short, short);		//resets the maze values to 1

void SaveMaze(void);				//writes to EEProm
void RestoreMaze(void);				//reads from EEProm
//-----------Memory Functions

//Movement Functions---------
void MOVE(void);					//forward
void GoForward(void);				//UNUSED- SUCKS
void TurnRight(void);				//RIGHT TURN
void TurnLeft(void);				//LEFT TURN
void TurnAround(void);				//TURN AROUND

void CheckOrientation(void);		//CHECK if centred
void SteerLeft(void);				//UNUSED curve left
void SteerRight(void);				//UNUSED curve right

void TurnToDirection(int);			//TURNS TO SPECIFIED DIRECTION
//---------Movement Functions
//=======================Prototypes


//Defines
#define UNKNOWN	1	//FOR WALL ARRAY- MAZE MAP
#define NORTH	3	//FOR WALL ARRAY- MAZE MAP
#define EAST	5	//FOR WALL ARRAY- MAZE MAP
#define SOUTH	7	//FOR WALL ARRAY- MAZE MAP
#define WEST	13	//FOR WALL ARRAY- MAZE MAP

#define OLD_RUN		1	//Loads from EEPROM
#define NEW_RUN		2	//Clears runtime memory - not EEProm
#define CLEARMODE	3	//Clear EEProm


//Mode Variables
short runTimeMode = 1;		//1 = solve from memory, 2 = start fresh (INCREMENTED by pressing left-most switch)
							//3 = clear EEPROM
short switchPosition = 1;	//waits for user input: switch flip (map-solve)


//Variables for Maze Solving----------------------------------
//short target;
short mouseCellValue;	//Current moves away from goal
short mouseX;			//current x location in map array
short mouseY;			//current y location in map array
short mouseDirection;	//current direction mouse is facing

//int mazeCenter = 119;	//[[119,120],[135,136]]
short mazeStart = 1;		//1 = TL, 2 = TR, 3 = BL, 4=BR
//Used to select starting corner - REMOVE THIS- used later on in main function

short mazeMap[16][16];	//Store distance from goal for each cell
						//={{4,5,6,7,8,7},{3,2,3,4,5,6},{15,1,0,1,6,7},{14,2,1,2,7,8},{12,11,10,9,8,9},{13,12,13,14,15,10}};
short wallMap[16][16];	//Where are walls using direction and % with prime numbers

//DAFUQ is this- find way back out of maze
short backMove[512];
short backMoveCount = -1;

int intFlag = 0;	//interrupt flag

short directionFlag;	//used in turntodirection() function
//----------------------------------Variables for Maze Solving


//-----
//Main Function
//-----
void main(){
//Enable pull-up resistors
RBPU = 0;					//ENABLE WHEN NOT DEBUGING (1=DEBUG)(0=RUN)

//Set pins for sensor input
SENSORS_TRIS = 0xFF;						//ALL SENSOR AS INPUTS
//Set pins for output to h-brdiges & LEDS
MOTORS_LED_TRIS = 0b00010000;
//Turn off LEDS & motors
MOTORS_LED = 0b11100000;

//runTimeMode = NEW_RUN;
//MazeSetup();
//Mode selection====================
LED(runTimeMode);	//display what mode we are in based on switch: 1 on startup
short runFlag = 0;	//
switchPosition = SWITCH;
for(;;){	//infinite loop
	for(int buttonCount = 0; buttonCount <=1;){	//wait for button press leftmost button (changes mode)
		DelayMs(1000);
		if(BUTTON == 0)
			buttonCount++;
		else
			buttonCount = 0;

		if(switchPosition != SWITCH){	//flip switch to GO
			runFlag = 1;
			break;
		}
	}
	if(runFlag == 0)
		runTimeMode++;
	if(runTimeMode > 3)
		runTimeMode = 1;

	LED(runTimeMode);	
	
	//WARNING!!! THIS WILL DELETE ALL EEEPROM MEMORY!!!
	if(runTimeMode == 3){
		for(int i = 0; i < 10; i++){
			LED(0);
			DelayMs(1000);
			LED(runTimeMode);
			DelayMs(10000);
		}
	}
	if(switchPosition != SWITCH)
		break;
}
runFlag = 0;
//====================Mode selection

//Start Selected Mode================
switch (runTimeMode){
case 1:	//OLD MAZE
	MazeSetup();
	break;
case 2:	//NEW MAZE
	MazeSetup();

	//Let the mouse know which block it is starting in through switch
	//binary counter like MODE selection -MAYBE REMOVE
	LED(mazeStart);
	switchPosition = SWITCH;
	for(;;){
		for(int buttonCount = 0; buttonCount <=1;){
			DelayMs(1000);
			if(BUTTON == 0)
				buttonCount++;
			else
				buttonCount = 0;

			if(switchPosition != SWITCH){
				runFlag = 1;
				break;
			}
		}
		if(runFlag == 0)
			mazeStart++;

		if(mazeStart > 4)
			mazeStart = 1;

		LED(mazeStart);
		if(switchPosition != SWITCH)
			break;
	}

	break;
case 3:	//CLEAR ALL MAZE
	ClearMaze(1,1);
	for(int i = 0;i < 1000;i++){
		LED(7);
		DelayMs(1000);
		LED(0);
		DelayMs(1000);
	}
	break;
}
//================Start Selected Mode


ConfigureInterrupts();	//Turns on those interrupts

//Always start facing north
mouseDirection = NORTH;

DisableInterrupts();	//Just cause

DelayMs(500);
DelayMs(500);
DelayMs(500);
DelayMs(500);

ConfigureInterrupts();	//Just cause some more

SolveMaze();	//Start maze map run

}


//-----
//Interrupt Function
//-----
void interrupt SensorTriggered(void){
	static unsigned char last_portb;
	unsigned char change;

	//Read PORTB just in case
	PORTB = PORTB;

	if ((INT0IE) && (INT0IF)){		//RF_SENSOR triggered
		//SteerLeft();
		intFlag = 1;
		//Clear Flag
		INT0IF = 0;
	}
	if ((INT1IE) && (INT1IF)){		//LF_SENSOR triggered
		//SteerRight();
		intFlag = 2;
		//Clear Flag
		INT1F=0;
	}
	if ((INT2IF)){					//F_SENSOR triggered

		//Clear Flag
		INT2IF = 0;					//Clear Flag just in case
	}
}






//+++++++++++++++++++++++++++++++SENSORS++++++++++++++++++++++++++++++++++
void CheckOrientation(){	//REPLACED BY PID
	if(LF_SENSOR == 0){		//Too close to left wall
		//DelayMs(500);
		SteerLeft();
		LED(4);
	}
	if(RF_SENSOR == 0){		//Too close to right wall
		//DelayMs(500);
		SteerRight();
		LED(1);
	}
}
//+++++++++++++++++++++++++++++++SENSORS++++++++++++++++++++++++++++++++++





//++++++++++++++++++++++++++++++++VARIOUS+++++++++++++++++++++++++++++++++
void LED(short led){
	LED_1 = 1;
	LED_2 = 1;
	LED_3 = 1;
	
	LATD &= ~(led << 5);
}

void ConfigureInterrupts(){
	//Set up External Interrupts
	GIE=1;
	IPEN=0;

	//External Interrupt Enable Bits
	INT0IE=1;
	INT1IE=1;
	INT2IE=0;					//We are not using INT2

	//Interrupts set on falling edge
	INTEDG0 = 0;
	INTEDG1 = 0;
	INTEDG2 = 0;				//We are not using INT2

	//Interrupt Priority bits  NOT NEEDED SINCE IPEN IS CLEARED
	INT2IP = 1;
	INT1IP = 1;
}

void DisableInterrupts(void){
	//Disable External Interrupts
	GIE=0;
	IPEN=0;

	//External Interrupt Disable Bits
	INT0IE=0;
	INT1IE=0;
	INT2IE=0;					//We are not using INT2

	//Interrupts set on falling edge
	INTEDG0 = 0;
	INTEDG1 = 0;
	INTEDG2 = 0;				//We are not using INT2

	//Interrupt Priority bits  NOT NEEDED SINCE IPEN IS CLEARED
	INT2IP = 1;
	INT1IP = 1;
}
//++++++++++++++++++++++++++++++++VARIOUS+++++++++++++++++++++++++++++++++





//++++++++++++++++++++++++++++++++++++MAZE+++++++++++++++++++++++++++++++++
void MazeSetup(){
if(runTimeMode == OLD_RUN)
	RestoreMaze();

else if (runTimeMode == NEW_RUN){
	ClearMaze(1, 0);
	//Initialize distance of squares from center
	for(short i = 0; i < 16; i++){
		for(short j = 0; j < 16; j++){
			if(i < 8){
				if(j < 8 ){
					mazeMap[i][j] = 16 - 2 - j - i;
				}
				else{
					mazeMap[i][j] = j - i;
				}
			}
			else{
				if(j < 8){
					mazeMap[i][j] = i - j;
				}
				else{
					mazeMap[i][j] = (j - 7) + (i - 7);
				}
			}

		//Initialize wallMap
		wallMap[i][j] = UNKNOWN;
		}
	}
}

}

void SolveMaze(){
switch(mazeStart){	//What is our starting square
case 1:	//TL
	mouseX = 0;
	mouseY = 0;
	TurnToDirection(SOUTH);
	wallMap[mouseY][mouseX] *= NORTH;
	break;

case 2:	//TR
	mouseX = 15;
	mouseY = 0;
	TurnToDirection(SOUTH);
	wallMap[mouseY][mouseX] *= NORTH;
	break;

case 3:	//BL
	mouseX = 0;
	mouseY = 15;
	wallMap[mouseY][mouseX] *= SOUTH;
	break;

case 4:	//BR
	mouseX = 15;
	mouseY = 15;
	wallMap[mouseY][mouseX] *= SOUTH;
	break;
}


if(runTimeMode == NEW_RUN){	//run mode 2
int tempVar;
int tempDir;	//used for deciding what direction to turn (left/right) based on current direction and desired direction

for(;;){
//mouseCellValue = mazeMap[mouseY][mouseX];
CheckOrientation();
//order of neighbours
	mouseCellValue = mazeMap[mouseY][mouseX];	//How far are we from goal?
	//Check to see if mouse is at end of the maze
	if(mouseCellValue == 0)	//YAY we are at centre!
		break;

		CheckWalls();						//check what walls are 
		CheckAdjacentCells(mouseY, mouseX);	//updates current cell value in array
		tempDir = directionFlag;			
		mouseCellValue = mazeMap[mouseY][mouseX];	//updates current distance value

//FLOOD FILL UPDATE ALL SQUARES -PROBABLY SHOULD BE A FUNCTION
//Brute Force check consistency-------------------
//Don't Modify the MIDDLE SQUARE!!!!
	//Left and up-----
	for(short a = mouseX-1; a >=0; a--){
		if((mouseY == 7) && (a == 7))
			continue;
		CheckAdjacentCells(mouseY, a);
		
	}

	for(short a = mouseY-1; a >= 0; a--){
		for(short b = 15; b >= 0; b--){
			if((a == 7) && (b == 7))
				continue;
			CheckAdjacentCells(a, b);
		}
	}
	//-----Left and up

	//right and down--
	for(short a = mouseX+1; a < 16; a++){
		if((mouseY == 7) && (a == 7))
			continue;
		CheckAdjacentCells(mouseY, a);
	}

	for(short a = mouseY+1; a < 16; a++){
		for(short b = 0; b < 16; b++){
			if((a == 7) && (b == 7))
				continue;
			CheckAdjacentCells(a, b);
		}
	}
	//--right and down

//-------------------Brute Force check consistency
//Handle Turns

	DisableInterrupts();

	TurnToDirection(tempDir);

	ConfigureInterrupts();

	MOVE();
}
	EndMaze();
}
else if(runTimeMode == OLD_RUN){
int tempVar;
int tempDir;

for(;;){
	mouseCellValue = mazeMap[mouseY][mouseX];
	//tempVar holds location of adjacent square with lowest value
	tempVar = mouseCellValue;

	if(mouseCellValue == 0)
		break;

	CheckOrientation();

//NEED TO WORRY ABOUT BOUNDARIES (&& statements take care of this)!!!
	if ((mouseX+1 < 16) && (mazeMap[mouseY][mouseX+1] < tempVar)){
		tempVar = mazeMap[mouseY][mouseX+1];
		tempDir = EAST;
	}
	if((mouseY+1 < 16) && (mazeMap[mouseY+1][mouseX] < tempVar)){
		tempVar = mazeMap[mouseY+1][mouseX];
		tempDir = SOUTH;
	}
	if((mouseX-1 >= 0) && (mazeMap[mouseY][mouseX-1] < tempVar)){
		tempVar = mazeMap[mouseY][mouseX-1];
		tempDir = WEST;
	}
	if((mouseY-1 >= 0) && (mazeMap[mouseY-1][mouseX] < tempVar)){
		tempVar = mazeMap[mouseY-1][mouseX];
		tempDir = NORTH;
	}	

	DisableInterrupts();

	TurnToDirection(tempDir);

	ConfigureInterrupts();

	MOVE();
}
	EndMaze();
}

}

void EndMaze(){
SaveMaze();

//switchPosition = SWITCH;
for(int i = 0;i < 2; i++){
	if(switchPosition != SWITCH)
		break;

	LED(5);
	DelayMs(1000);
	LED(0);
	DelayMs(1000);
}

TurnAround();
for(int bsVar = backMoveCount; bsVar >= 0; bsVar--){

if(backMove[bsVar] == 1)
	MOVE();
if(backMove[bsVar] == 2)
	TurnLeft();
if(backMove[bsVar] == 3)
	TurnRight();
if(backMove[bsVar] == 4)
	TurnAround();
}
	
switchPosition = SWITCH;
for(;;){
	if(switchPosition != SWITCH)
		break;

	LED(5);
	DelayMs(1000);
	LED(0);
	DelayMs(1000);
}


}


//Maze algorthim functions==================================
void CheckAdjacentCells(short y, short x){	//updates current cell value based upon lowest neighbour
short adjacentSquares[4]={300,300,300,300};
	short rightSquare;
	short leftSquare;
	short topSquare;
	short bottomSquare;
	short lowestSquare;
	short lowestFound = 0;
short holdVar = 0;

//-----------Get lowest value neighbor and add one to current cell------
	//Get adjacent squares
	//if statements check boundaries
	if(x + 1 < 16){
		adjacentSquares[0] = mazeMap[y][x+1]; 	//Right
		rightSquare = adjacentSquares[0];
	}
	if(x - 1 >= 0){
		adjacentSquares[1] = mazeMap[y][x-1];	//Left
		leftSquare = adjacentSquares[1];
	}
	if(y - 1 >= 0){
		adjacentSquares[2] = mazeMap[y-1][x];	//Top
		topSquare = adjacentSquares[2];
	}
	if(y + 1 < 16){
		adjacentSquares[3] = mazeMap[y+1][x];	//Bottom
		bottomSquare = adjacentSquares[3];
	}

	//Sort neighbors (lowest to highest value)
	for(int j = 0; j <= 2; j++)
		for(int i = j+1; i <= 3; i++)
			if(adjacentSquares[i]<adjacentSquares[j]){
				holdVar = adjacentSquares[j];
				adjacentSquares[j] = adjacentSquares[i];
				adjacentSquares[i] = holdVar;
			}

	//Check for walls between mouseCell and lowestValue
	for(int i = 0; i < 4; i++){
	if(lowestFound == 0){
		if(adjacentSquares[i] == topSquare)
			if(wallMap[y][x] % NORTH != 0){
				lowestSquare = topSquare;
				directionFlag = NORTH;
				lowestFound = 1;
				continue;
			}
		if(adjacentSquares[i] == rightSquare)
			if(wallMap[y][x] % EAST != 0){
				lowestSquare = rightSquare;
				directionFlag = EAST;
				lowestFound = 1;
				continue;
			}
		if(adjacentSquares[i] == bottomSquare)
			if(wallMap[y][x]% SOUTH != 0){
				lowestSquare = bottomSquare;
				directionFlag = SOUTH;
				lowestFound = 1;
				continue;
			}			
		if(adjacentSquares[i] == leftSquare)
			if(wallMap[y][x] % WEST != 0){
				lowestSquare = leftSquare;
				directionFlag = WEST;
				lowestFound = 1;
				continue;
			}
	}
}
//increment cellVal to 1 greater than this value
mazeMap[y][x] = lowestSquare + 1;
//----------------------------------------------------------------------
}


void CheckWalls(){
//Count for direction
//NEED TO WORRY ABOUT BOUNDARIES (&& statements take care of this)!!!
switch (mouseDirection){
//Mouse is facing north
case NORTH:
	if(F_SENSOR == 0){
		if (wallMap[mouseY][mouseX] % NORTH != 0)
			wallMap[mouseY][mouseX] *= NORTH;

		if((mouseY - 1 >= 0) && (wallMap[mouseY-1][mouseX] % SOUTH != 0))
			wallMap[mouseY-1][mouseX] *= SOUTH;

	}	
	if(R_SENSOR == 0){
		if (wallMap[mouseY][mouseX] % EAST != 0)
			wallMap[mouseY][mouseX] *= EAST;

		if((mouseX + 1 < 16) && (wallMap[mouseY][mouseX] % WEST != 0))
			wallMap[mouseY][mouseX+1] *= WEST;
		
	}	

	if(L_SENSOR == 0){
		if(wallMap[mouseY][mouseX] % WEST != 0)
			wallMap[mouseY][mouseX] *= WEST;

		if((mouseX - 1 >= 0) && (wallMap[mouseY][mouseX-1] % EAST != 0))
			wallMap[mouseY][mouseX-1] *= EAST;
	}
	break;

//Mouse is facing East
case EAST:
	if(F_SENSOR == 0){
		if(wallMap[mouseY][mouseX] % EAST != 0)
			wallMap[mouseY][mouseX] *= EAST;

		if((mouseX + 1 < 16) && (wallMap[mouseY][mouseX+1] % WEST != 0))
			wallMap[mouseY][mouseX+1] *= WEST;
	}

	if(R_SENSOR == 0){
		if(wallMap[mouseY][mouseX] % SOUTH != 0);
			wallMap[mouseY][mouseX] *= SOUTH;

		if((mouseY + 1 < 16) && (wallMap[mouseY+1][mouseX] % NORTH != 0))
			wallMap[mouseY+1][mouseX] *= NORTH;
	}

	if(L_SENSOR == 0){
		if(wallMap[mouseY][mouseX] % NORTH != 0)
			wallMap[mouseY][mouseX] *= NORTH;
		
		if((mouseY - 1 >= 0) && (wallMap[mouseY-1][mouseX] % SOUTH != 0))
			wallMap[mouseY-1][mouseX] *= SOUTH;
	}

	break;

//Mouse is facing South
case SOUTH:
	if(F_SENSOR == 0){
		if(wallMap[mouseY][mouseX] % SOUTH != 0)
			wallMap[mouseY][mouseX] *= SOUTH;
		
		if((mouseY + 1 < 16) && (wallMap[mouseY+1][mouseX] % NORTH != 0))
			wallMap[mouseY+1][mouseX] *= NORTH;
	}

	if(R_SENSOR == 0){
		if(wallMap[mouseY][mouseX] % WEST !=0 )
			wallMap[mouseY][mouseX] *= WEST;

		if((mouseX - 1 >= 0) && (wallMap[mouseY][mouseX-1] % EAST != 0))
			wallMap[mouseY][mouseX-1] *= EAST;
	}

	if(L_SENSOR == 0){
		if(wallMap[mouseY][mouseX] % EAST != 0)
			wallMap[mouseY][mouseX] *= EAST;
		
		if((mouseX + 1 < 16) && (wallMap[mouseY][mouseX+1] % WEST != 0))
			wallMap[mouseY][mouseX+1] *= WEST;
	}

	break;

//Mouse is facing West
case WEST:
	if(F_SENSOR == 0){
		if(wallMap[mouseY][mouseX] % WEST != 0)
			wallMap[mouseY][mouseX] *= WEST;

		if((mouseX - 1 >= 0) && (wallMap[mouseY][mouseX-1] % EAST != 0))
			wallMap[mouseY][mouseX-1] *= EAST;
	}

	if(R_SENSOR == 0){
		if(wallMap[mouseY][mouseX] % NORTH != 0)
			wallMap[mouseY][mouseX] *= NORTH;

		if((mouseY - 1 >= 0) && (wallMap[mouseY-1][mouseX] % SOUTH != 0))
			wallMap[mouseY-1][mouseX] *= SOUTH;
	}

	if(L_SENSOR == 0){
		if(wallMap[mouseY][mouseX] % SOUTH != 0);
			wallMap[mouseY][mouseX] *= SOUTH;
		
		if((mouseY + 1 < 16) && (wallMap[mouseY+1][mouseX] % NORTH != 0))
			wallMap[mouseY+1][mouseX] *= NORTH;
	}

	break;
}

}

//==================================Maze algorthim functions






//-----------------------Memory-----------------------
void ClearMaze(short clearRunTime, short clearEeprom){

//EEEPROM memory
if(clearEeprom == 1){
	for(int i = 0; i < 514; i++)
		eeprom_write(i, 0x00);
}

//Run-time memory
if(clearRunTime == 1){
	for(int i = 0; i < 16; i++)
		for(int j = 0; j < 16; j++)
			mazeMap[i][j] = 0;

	for(int i = 0; i < 16; i++)
		for(int j = 0; j < 16; j++)
			wallMap[i][j] = 1;
}

}


//EEPROM STUFF==============================================
void SaveMaze(){
//save mazeMap
for(int i = 0; i < 16; i++)
	for(int j = 0; j < 16; j++)
		eeprom_write((i*16)+j, mazeMap[i][j]);

//save wallMap
for(int i = 0; i < 16; i++)
	for(int j = 0; j < 16; j++)
		eeprom_write(i+j+256, wallMap[i][j]);

//save Starting square
eeprom_write(513, mazeStart);
}

void RestoreMaze(){
//read eeprom to mazeMap
for(int i = 0; i < 16; i++)
	for(int j = 0; j < 16; j++)
		mazeMap[i][j] = eeprom_read((i*16)+j);

//read eeprom to wallMap
for(int i = 0; i < 16; i++)
	for(int j = 0; j < 16; j++)
		wallMap[i][j] = eeprom_read(i+j+256);

//read Starting square
mazeStart = eeprom_read(513);

}
//==============================================EEPROM STUFF
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void TurnToDirection(int dir){
signed int dirCount;
int fixedDir;

backMoveCount++;

if(mouseDirection == NORTH)
	fixedDir = 0;
else if(mouseDirection == EAST)
	fixedDir = 1;
else if(mouseDirection == SOUTH)
	fixedDir = 2;
else if(mouseDirection == WEST)
	fixedDir = 3;

if(dir == NORTH)
	dirCount = 0;
else if(dir == EAST)
	dirCount = 1;
else if(dir == SOUTH)
	dirCount = 2;
else if(dir == WEST)
	dirCount = 3;

if(fixedDir - dirCount == -1){
	backMove[backMoveCount] = 2;
	TurnRight();
}
else if(fixedDir - dirCount == -2){
	backMove[backMoveCount] = 4;
	TurnAround();//TurnLeft();TurnLeft();}
}
else if(fixedDir - dirCount == -3){
	backMove[backMoveCount] = 3;
	TurnLeft();
}
else if(fixedDir - dirCount == 1){
	backMove[backMoveCount] = 3;
	TurnLeft();
}
else if(fixedDir - dirCount == 2){
	backMove[backMoveCount] = 4;
	TurnAround();//TurnLeft();TurnLeft();}
}
else if(fixedDir - dirCount == 3){
	backMove[backMoveCount] = 2;
	TurnRight();
}

mouseDirection = dir;
}







//MOVE FUNCTION
void MOVE(){
backMoveCount++;
backMove[backMoveCount] = 1;
DelayMs(100);
	int j;
	LATD = 0b00001010;

for(j = 0; j <11100; j++);

	for(j = 0; j < 10000; j++){		
	if(intFlag == 1){
			SteerLeft();
			intFlag = 0;
			LATD = 0b00001010;
		}
		else if(intFlag == 2){
			SteerRight();
			intFlag = 0;
			LATD = 0b00001010;
		}

	}
	for(j = 0; j < 16000; j++){
		if(intFlag == 1){
			SteerLeft();
			intFlag = 0;
			LATD = 0b00001010;
		}
		else if(intFlag == 2){
			SteerRight();
			intFlag = 0;
			LATD = 0b00001010;
		}
	}
	for(j = 0; j < 11000; j++){
		if(intFlag == 1){
			SteerLeft();
			intFlag = 0;
			LATD = 0b00001010;
		}
		else if(intFlag == 2){
			SteerRight();
			intFlag = 0;
			LATD = 0b00001010;
		}
	}
	for(j = 0; j < 1800; j++){
		if(intFlag == 1){
			SteerLeft();
			intFlag = 0;
			LATD = 0b00001010;
		}
		else if(intFlag == 2){
			SteerRight();
			intFlag = 0;
			LATD = 0b00001010;
		}
	}
	LATD = 0;

	//update mouse's position in maze
	switch(mouseDirection){
	case NORTH:
		mouseY -= 1;
		break;
	case EAST:
		mouseX += 1;
		break;
	case SOUTH:
		mouseY +=1;
		break;
	case WEST:
		mouseX -= 1;
		break;
	}
}