/*
 * sudokuSolver.cpp
 *
 *  Created on: 2017-10-24
 *      Author: arjun
 */

#include <iostream>
using namespace std;
void printBoard(int sudoku[9][9]);
void printBoard3(int board[9][9][10]);
int values(int sudoku[][9], int x, int y, int number);
int fill(int sudoku[][9], int x, int y);
int possibilities(int board[9][9][10]);
void printBoard(int sudoku[9][9]){
	for (int i = 0; i < 9; i ++){
		if (i % 3 == 0){
			printf("********************** \n");
		}
		for (int j = 0; j < 9; j++){
			if (j == 2 || j == 5) printf("%d * ", sudoku[i][j]); // print * b/w ys
			else printf("%d ", sudoku[i][j]); // print normal
		}
		printf("\n");
	}
	printf("**********************\n");
}
void printBoard3(int board[9][9][10]){
	// print fat line ahead
	printf("**********************\n");

	for (int i = 0; i < 9; i ++){
		if (i == 3 || i == 6){
			printf("********************** \n");
		} // prints fat line b/w boxes
		for (int j = 0; j < 9; j++){
			if (j == 2 || j == 5) printf("%d * ", board[i][j][0]); // print * b/w columns
			else if (j == 8) printf("%d\n", board[i][j][0]); // print * b/w columns
			else printf("%d ", board[i][j][0]); // print normal
		}
	}
	printf("**********************\n"); // print fat line at bottom
}
int values(int sudoku[][9], int x, int y, int number){
	int gridx = (x / 3) * 3;
	int gridy = (y / 3) * 3;

	for (int i = 0; i <9; ++i){
		if (sudoku[x][i] == number) return 0; //col
		if (sudoku[i][y] == number) return 0; //row
	}
	for (int i = gridx; i < gridx + 3; i++){ //box
		for (int j = gridy; j < (gridy + 3); j++){
			if (sudoku[i][j] == number) return 0;
		}
	}
	return 1;
}

int fill(int sudoku[][9], int x, int y){
	if (y >= 9){
		y = 0;
		x++;
		if (x >= 9) return 1;
	}
	if (sudoku[x][y] != 0) return fill(sudoku, x, y + 1);
	else {
		for (int i = 1; i < 10; i++){ // LOOP THROUGH ENTRIES
			if (values(sudoku, x, y, i)){ // IF values ENTRY
				sudoku[x][y] = i; // VALUE = I + 1
				if (fill(sudoku, x, y + 1)) return 1;
				else sudoku[x][y] = 0;
			}
		}
		return 0;
	}
}
int possibilities(int board[9][9][10]){
	int count=0;
	for (int i=0; i<9;i++){
		for (int j=0; j<9;j++){
			for (int k=1; k<10;k++){
				if (board[i][j][0]!=0){
					board[i][j][k]=0;
				}else{
					board[i][j][k]=k;//set the possibilities of each block to integers from 1-9
				}
				for (int l=0; l<9; l++){
					if (board[i][j][0]==0 && board[i][l][0]!=0){//checks if there is a value in the row
						board[i][j][board[i][l][0]]=0;
					}
				}
				for (int l=0; l<9; l++){
					if (board[i][j][0]==0 && board[l][j][0]!=0){//checks if there is a value in the column
						board[i][j][board[l][j][0]]=0;
					}
				}
				for (int l=(i/3)*3;l<(i/3)*3+3;l++){
					for (int k=(j/3)*3;k<(j/3)*3+3;k++){
						if(board[i][j][0]==0&& board[l][k][0]!=0){//checks the three x three square for any values
							board[i][j][board[l][k][0]]=0;
						}
					}
				}
			}
		}
	}
	for (int i=0; i<9;i++){
		for (int j=0; j<9;j++){
			for (int k=1; k<10;k++){
				if (board[i][j][k]!=0){
					count++;//increases the number of possibilities
				}
			}
			if (count==0 && board[i][j][0]==0){
				printf("%d, %d\n", i,j);
				return 0;
			}
			if (count==1){//if the number possibilities is equal to one, then the square must become that value
				for (int l=1; l<10; l++){
					if (board[i][j][l]!=0){
						board[i][j][0]=l;
						return 1;
					}
				}
			}
		}
	}
	for (int i=0; i<9; i++){
		for (int j=0; j<9; j++){
			for (int m=1; m<10; m++){
				count=0;
				for (int k=(i/3)*3; k<((i/3)*3+3); k++){
					for (int l=(j/3)*3; l<((j/3)*3+3); l++){
						if(board[i][j][m]==board[k][l][m] && board[i][j][m]!=0){//checks if other blocks in the 3x3 has the possibility of
							// being a number. If only one has the possibility, it is
							//logically the only one able to become that number
							count++;
						}
					}
				}
				if (count==1 && board[i][j][m]==m){
					board[i][j][0]=m;
					return 1;
				}
			}
		}
	}
	return 0;
}
int main(void) {
	int simple;
	int board[9][9][10];
	for (int i = 0; i < 9; i++){
		for (int j = 0; j < 9; j++){
			board[i][j][0] = 0;
		}
	}

	board[0][0][0] = 1;
	board[0][1][0] = 4;
	board[0][2][0] = 1;
	board[0][3][0] = 9;
	board[0][4][0] = 0;
	board[0][5][0] = 0;
	board[0][6][0] = 0;
	board[0][7][0] = 0;
	board[0][8][0] = 0;
	board[1][0][0] = 0;
	board[1][1][0] = 2;
	board[1][2][0] = 0;
	board[1][3][0] = 0;
	board[1][4][0] = 0;
	board[1][5][0] = 0;
	board[1][6][0] = 4;
	board[1][7][0] = 1;
	board[1][8][0] = 0;
	board[2][0][0] = 5;
	board[2][1][0] = 0;
	board[2][2][0] = 0;
	board[2][3][0] = 6;
	board[2][4][0] = 0;
	board[2][5][0] = 0;
	board[2][6][0] = 0;
	board[2][7][0] = 9;
	board[2][8][0] = 0;
	board[3][0][0] = 0;
	board[3][1][0] = 0;
	board[3][2][0] = 7;
	board[3][3][0] = 5;
	board[3][4][0] = 0;
	board[3][5][0] = 0;
	board[3][6][0] = 0;
	board[3][7][0] = 0;
	board[3][8][0] = 0;
	board[4][0][0] = 9;
	board[4][1][0] = 0;
	board[4][2][0] = 0;
	board[4][3][0] = 0;
	board[4][4][0] = 8;
	board[4][5][0] = 0;
	board[4][6][0] = 0;
	board[4][7][0] = 0;
	board[4][8][0] = 3;
	board[5][0][0] = 0;
	board[5][1][0] = 0;
	board[5][2][0] = 0;
	board[5][3][0] = 0;
	board[5][4][0] = 0;
	board[5][5][0] = 6;
	board[5][6][0] = 8;
	board[5][7][0] = 0;
	board[5][8][0] = 0;
	board[6][0][0] = 0;
	board[6][1][0] = 9;
	board[6][2][0] = 0;
	board[6][3][0] = 0;
	board[6][4][0] = 0;
	board[6][5][0] = 2;
	board[6][6][0] = 0;
	board[6][7][0] = 0;
	board[6][8][0] = 4;
	board[7][0][0] = 0;
	board[7][1][0] = 8;
	board[7][2][0] = 1;
	board[7][3][0] = 0;
	board[7][4][0] = 0;
	board[7][5][0] = 0;
	board[7][6][0] = 0;
	board[7][7][0] = 3;
	board[7][8][0] = 0;
	board[8][0][0] = 0;
	board[8][1][0] = 0;
	board[8][2][0] = 0;
	board[8][3][0] = 0;
	board[8][4][0] = 0;
	board[8][5][0] = 3;
	board[8][6][0] = 0;
	board[8][7][0] = 2;
	board[8][8][0] = 9;
	printBoard3(board);
	do{
		simple = possibilities(board);
	}while(simple!=0);
	int sudoku[9][9];
	for (int i=0;i<9; i++){
		for (int j=0 ;j<9; j++){
			sudoku[i][j]=board[i][j][0];
		}
	}
	if(fill(sudoku, 0, 0)){
		printBoard(sudoku);
	} else printf("\n\nSolution doesn't exist. \n\n");
	return 0;
}
