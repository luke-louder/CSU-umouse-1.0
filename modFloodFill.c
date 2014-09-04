/* 	Modified Flood Fill - Luke Louder
	CSU uMouse 2015
	Optimization of flood fill
	Use a stack to update cells
	adding dead end recognition
*/

#define WIDTH 16	// Number of rows in the maze
#define HEIGHT 16	// Number of columns in the maze


void mazeSetup(short (&mazeToFill)[WIDTH * HEIGHT]){	// Flood fill a i x j array for the maze
	for(short row = 0; row < WIDTH; row++){
		for(short col = 0; col < HEIGHT; col++){
			if(row < 8){
				if(col < 8 ){
					mazeToFill[WIDTH * row + col] = 16 - 2 - col - row;
				}
				else{
					mazeToFill[WIDTH * row + col] = col - row;
				}
			}
			else{
				if(col < 8){
					mazeToFill[WIDTH * row + col] = row - col;
				}
				else{
					mazeToFill[WIDTH * row + col] = (col - 7) + (row - 7);
				}
			}
		}
	}
}

void mazePrint(short (&mazeToPrint)[WIDTH * HEIGHT]){
	for(short row = 0; row < 16; row++){
		for(short col = 0; col < 16; col++){
			printf(" %d ", &mazeToPrint[WIDTH * rol + col]);
		}
	}
}

int main(){
	short mazeMap[WIDTH * HEIGHT];
	mazeSetup(mazeMap);
	mazePrint(mazeMap);
	return 0;
}

