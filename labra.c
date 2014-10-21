#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h> //for srand()

#include "lcd.h"

#define LCD_START_LINE1  0x00
#define LCD_START_LINE2  0x40

void init(void) {

     /* estetään kaikki keskeytykset */
  cli();

        /* kaiutin pinnit ulostuloksi */
        DDRE  |=  (1 << PE4) | (1 << PE5);
        /* pinni PE4 nollataan */
        PORTE &= ~(1 << PE4);
        /* pinni PE5 asetetaan */
        PORTE |=  (1 << PE5);   
        
        /* ajastin nollautuu, kun sen ja OCR1A rekisterin arvot ovat samat */
        TCCR1A &= ~( (1 << WGM11) | (1 << WGM10) );
        TCCR1B |=    (1 << WGM12);
        TCCR1B &=   ~(1 << WGM13);

        /* salli keskeytys, jos ajastimen ja OCR1A rekisterin arvot ovat samat */
        TIMSK |= (1 << OCIE1A);

        /* asetetaan OCR1A rekisterin arvoksi 0x3e (~250hz) */
        OCR1AH = 0x00;//0x00
        OCR1AL = 0x14;//0x14

        /* käynnistä ajastin ja käytä kellotaajuutena (16 000 000 / 1024) Hz */
        TCCR1B |= (1 << CS12) | (1 << CS10);

  /* näppäin pinnit sisääntuloksi */
  DDRA &= ~(1 << PA0);
  DDRA &= ~(1 << PA2);
  DDRA &= ~(1 << PA4);


  /* rele/led pinni ulostuloksi */
  DDRA |= (1 << PA6);

  /* lcd-näytön alustaminen */
  lcd_init();
  lcd_write_ctrl(LCD_ON);
  lcd_write_ctrl(LCD_CLEAR);

  //ALUSTA STUFF

  DDRA &= ~(1 << PC0);

}

void ledivalo();

void lcd_gotoxy (unsigned char x, unsigned char y);

int checkButtonPress(int buttonNumber);

int calculateCursorLocationVertical(int cursorLocationY, int currentButton);
int calculateCursorLocationHorizontal(int cursorLocationX, int currentButton);

char calculateRowNumber(int number, char numberChar);

int main(void) 
{

  init();
   
	int inMenu = 1;
	int inGame = 0;
	int MainMenuTextWritten = 0;
	int InGameTextWritten = 0;
	char topRow[] = "BATTLESHIP!     ";
	char bottomRow[] = "Press Down Btn  ";
	char youLose[] = "YOU LOSE! AGAIN?  ";
	char youWin[] = "YOU WIN! GRATS! ";
	int Rows[10][10] = {{0,0,0,0,0,0,0,0,0,0},
					    {0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0}};
	
	char topRowNumber = '1';
	char bottomRowNumber = '2';
	int topRowNumberInt = 1;
	int bottomRowNumberInt = 2;
	int counter = 0;
	int counter2 = 0;
	int counter3 = 0;
	int counter4 = 0;
	int currentButton = 0;
	int cursorLocationY = 0;
	int cursorLocationX = 0;
	int currentTopRow = 0;
	int currentRow = 0;
	int randomHolderYAxis;
	int randomHolderXAxis;
	int shipSize[] = {5,4,3,3,2,1};
	int numberOfShipsPlaced = 0;
	int spotTakenFlag = 0;
	int verticalOrHorizontal = 1;
	int ShotsRemaining = 10;
	int Hits = 0;
	int shipAddedSuccessfullyFlag = 0;
	int debugMode = 0;

while(1)
{
while(inMenu == 1)
{
	
	//DRAW THE MAIN MENU
	if(MainMenuTextWritten == 0)
	{
		for(counter = 0; counter < 17; counter++)
		{
			lcd_write_data(topRow[counter]);
		}
		lcd_gotoxy(0,1);
		for(counter = 0; counter < 17; counter++)
		{
			lcd_write_data(bottomRow[counter]);
		}
		debugMode = 0;
		MainMenuTextWritten = 1;
	}

	//debug mode when pressing up
   		if(PINA & (1 << PA0))
		{
			//Do nothing
		}
		else
		{
			debugMode = 1;
		}

	//when pressing down button, start game
	if(PINA & (1 << PA4))//PA2
   {
   		//loop counter generation
		if(counter != 100)
		{
			counter++;
		}
		else
		{
			counter = 0;
		}
		if(counter2 != 100)
		{
			counter2++;
		}
		else
		{
			counter2 = 50;
		}
		
   }
   else
   {
   		inMenu = 0;
		inGame = 1;
		InGameTextWritten = 0;
		lcd_write_ctrl(LCD_CLEAR);
		ShotsRemaining = 10;
		numberOfShipsPlaced = 0;
		Hits = 0;
		OCR1AL = 0x14;
		currentButton = 6;

		//generate random ships
		srand( (counter + counter2));
	
		//attempt to place ships in random sections until all ships are placed
		while(numberOfShipsPlaced < 6)
		{
			//insert new random variables
			verticalOrHorizontal = rand() % 2;
			shipAddedSuccessfullyFlag = 0;
			randomHolderYAxis = rand() % 10;
			randomHolderXAxis = rand() % 10;

			if(verticalOrHorizontal == 0)
			{
				//loop through columns
				for(counter = randomHolderYAxis; counter < 10; counter++)
				{
					if(shipAddedSuccessfullyFlag == 1)
					{
						break;
					}
					//loop through rows
					for(counter2 = randomHolderXAxis; counter2 < 10; counter2++)
					{
						if(shipAddedSuccessfullyFlag == 1)
						{
							break;
						}
						spotTakenFlag = 0;
						//check to see if spots are taken
						for(counter3 = counter2; counter3 < (counter2 + shipSize[numberOfShipsPlaced]); counter3++)
						{
							if(counter2 + shipSize[numberOfShipsPlaced] < 11)
							{
								if(Rows[counter][counter3] != 0)
								{
									spotTakenFlag = 1;
									break;
								}
							}
							else
							{
								spotTakenFlag = 1;
								break;
							}
						}
						//check to see if spots above and below are taken
						for(counter3 = counter2; counter3 < (counter2 + shipSize[numberOfShipsPlaced]); counter3++)
						{
							if(counter2 + shipSize[numberOfShipsPlaced] < 11)
							{
								if(counter != 0)
								{
									if(Rows[counter - 1][counter3] != 0)
									{
										spotTakenFlag = 1;
										break;
									}
								}
								if(counter != 9)
								{
									if(Rows[counter + 1][counter3] != 0)
									{
										spotTakenFlag = 1;
										break;
									}
								}
							}
							else
							{
								spotTakenFlag = 1;
								break;
							}
						}
						//check to see if spots left and right are taken
						if(counter2 != 0)
						{
							if(Rows[counter][counter2 - 1] != 0)
							{
								spotTakenFlag = 1;
							}
						}
						if(counter2 != 9)
						{
							if(Rows[counter][counter2 + 1] != 0)
							{
								spotTakenFlag = 1;
							}
						}
						//if all spots are free, draw ship in spots
						if(spotTakenFlag == 0)
						{
							for(counter3 = counter2; counter3 < (counter2 + shipSize[numberOfShipsPlaced]); counter3++)
							{
								Rows[counter][counter3] = 1;
							}
							numberOfShipsPlaced++;
							shipAddedSuccessfullyFlag = 1;
						}
					}
				}				
			}
			else if(verticalOrHorizontal == 1)
			{
				//loop through rows
				for(counter2 = randomHolderXAxis; counter2 < 10; counter2++)
				{
					if(shipAddedSuccessfullyFlag == 1)
					{
						break;
					}
					//loop through columns
					for(counter = randomHolderYAxis; counter < 10; counter++)
					{
						if(shipAddedSuccessfullyFlag == 1)
						{
							break;
						}
						spotTakenFlag = 0;
						//check to see if spots are taken
						for(counter3 = counter; counter3 < (counter + shipSize[numberOfShipsPlaced]); counter3++)
						{
							if((counter + shipSize[numberOfShipsPlaced]) < 10)
							{
								if(Rows[counter3][counter2] != 0)
								{
									spotTakenFlag = 1;
									break;
								}
							}
							else
							{
								spotTakenFlag = 1;
								break;
							}
						}
						//check to see if spots left and right are taken
						for(counter3 = counter; counter3 < (counter + shipSize[numberOfShipsPlaced]); counter3++)
						{
							if(counter + shipSize[numberOfShipsPlaced] < 10)
							{
								if(counter2 != 0)
								{
									if(Rows[counter3][counter2 - 1] != 0)
									{
										spotTakenFlag = 1;
										break;
									}
								}
								if(counter2 != 9)
								{
									if(Rows[counter3][counter2 + 1] != 0)
									{
										spotTakenFlag = 1;
										break;
									}
								}
							}
						}
						//check to see if spots above and below are taken
						if(counter != 0)
						{
							if(Rows[counter - 1][counter2] != 0)
							{
								spotTakenFlag = 1;
							}
						}
						if(counter != 9)
						{
							if(Rows[counter + 1][counter2] != 0)
							{
								spotTakenFlag = 1;
							}
						}
						//if spots are free, draw ship on spots
						if(spotTakenFlag == 0)
						{
							for(counter3 = counter; counter3 < (counter + shipSize[numberOfShipsPlaced]); counter3++)
							{
								Rows[counter3][counter2] = 1;
							}
							numberOfShipsPlaced++;
							shipAddedSuccessfullyFlag = 1;
						}
					}
				}
			}
			
		}

   }
}

//Ingame Logic
while(inGame == 1)
{
	//Function checks which button is pressed
	if(currentButton == 0)
	{
		
		currentButton = checkButtonPress(currentButton);

		//Shoot when pressing middle button
		if(currentButton == 2)
		{
			if(cursorLocationY == 0)
			{
					//if selection is a hit, change marker to hit
					if(Rows[currentTopRow][cursorLocationX] == 1)
					{
						Rows[currentTopRow][cursorLocationX] = 2;
						Hits++;
						ledivalo();
					}
					//else, change to a miss
					else if(Rows[currentTopRow][cursorLocationX] == 0)
					{
						Rows[currentTopRow][cursorLocationX] = 3;
						ShotsRemaining--;
					}
			}
			else if(cursorLocationY == 1)
			{
					//if selection is a hit, change marker to hit
					if(Rows[currentTopRow + 1][cursorLocationX] == 1)
					{
						Rows[currentTopRow + 1][cursorLocationX] = 2;
						Hits++;
						ledivalo();
					}
					//else, change to a miss
					else if(Rows[currentTopRow + 1][cursorLocationX] == 0)
					{
						Rows[currentTopRow + 1][cursorLocationX] = 3;
						ShotsRemaining--;
					}
			}
			sei();
		}
		else
		{
			cli();
		}

		//Function changes cursor location according to input
		cursorLocationY = calculateCursorLocationVertical(cursorLocationY, currentButton);
		cursorLocationX = calculateCursorLocationHorizontal(cursorLocationX, currentButton);

		//Change row number upon moving vertically
		if(currentButton == 4) //Moving downwards
		{
			
			if(currentRow == 0)
			{
				currentRow++;
			}
			else if(currentTopRow < 8)
			{
				currentTopRow++;
				topRowNumberInt++;
				topRowNumber = calculateRowNumber(topRowNumberInt, topRowNumber);
				bottomRowNumberInt++;
				bottomRowNumber = calculateRowNumber(bottomRowNumberInt, bottomRowNumber);
			}
			
		
		}
		if(currentButton == 5) //Moving upwards
		{
			if(currentRow == 1)
			{
				currentRow--;
			}
			else if(currentTopRow > 0)
			{
				currentTopRow--;
				topRowNumberInt--;
				topRowNumber = calculateRowNumber(topRowNumberInt, topRowNumber);
				bottomRowNumberInt--;
				bottomRowNumber = calculateRowNumber(bottomRowNumberInt, bottomRowNumber);
			}
			
		}
	}







	//DRAW THE GAME SCREEN
	if(currentButton != 0 || InGameTextWritten == 0)//||
	{
		lcd_write_ctrl(LCD_CLEAR);

			//Draw row number and colon
			lcd_write_data(topRowNumber);
			lcd_write_data(':');		

			//draw first row contents
			for(counter = 0; counter < 10; counter++)
			{
				if(cursorLocationY == 0)
				{
					if(counter == cursorLocationX)
					{
						lcd_write_data('_');
						continue;
					}
				}
					if(Rows[currentTopRow][counter] == 0)
					{
						lcd_write_data(' ');
					}
					else if(Rows[currentTopRow][counter] == 1)
					{
						if(debugMode == 1)
						{
							lcd_write_data('X');
						}
						else
						{
							lcd_write_data(' ');
						}
					}
					else if(Rows[currentTopRow][counter] == 2)
					{
						lcd_write_data('H');
					}
					else if(Rows[currentTopRow][counter] == 3)
					{
						lcd_write_data('M');
					}
				
			}			

			lcd_gotoxy(0,1);

			//draw second row number and colon
			if(bottomRowNumberInt < 10)
			{ 
				lcd_write_data(bottomRowNumber);
				lcd_write_data(':');
			}
			else
			{
				lcd_write_data('1');
				lcd_write_data('0');
			}

			//draw second row contents
			for(counter = 0; counter < 10; counter++)
			{
				if(cursorLocationY == 1)
				{
					if(counter == cursorLocationX)
					{
						lcd_write_data('_');
						continue;
					}
				}
					if(Rows[currentTopRow + 1][counter] == 0)
					{
						lcd_write_data(' ');
					}
					else if(Rows[currentTopRow + 1][counter] == 1)
					{
						if(debugMode == 1)
						{
							lcd_write_data('X');
						}
						else
						{
							lcd_write_data(' ');
						}
					}
					else if(Rows[currentTopRow + 1][counter] == 2)
					{
						lcd_write_data('H');
					}
					else if(Rows[currentTopRow + 1][counter] == 3)
					{
						lcd_write_data('M');
					}
			}

		InGameTextWritten = 1;
		}
		//If no buttons are pressed, allow for further button checks
		if((PINA & (1 << PA0)) && (PINA & (1 << PA1)) && (PINA & (1 << PA2)) &&
	   (PINA & (1 << PA3)) && (PINA & (1 << PA4)))
		{
			currentButton = 0;
			ledivalo();
		}
	

	
	//if player is out of shots, reset the game
	if(ShotsRemaining < 1)
	{
		inGame = 0;
		inMenu = 1;
		InGameTextWritten = 1;
		MainMenuTextWritten = 0;
		for(counter4 = 0; counter4 < 10; counter4++)
		{
			for(counter3 = 0; counter3 < 10; counter3++)
			{
				Rows[counter4][counter3] = 0;
			}
		}
		for(counter = 0; counter < 16; counter++)
		{
			topRow[counter] = youLose[counter];
		}
		lcd_gotoxy(0,0);
		OCR1AL = 0x15;
		break;
	}
	//if player Hits, calculate to see if player wins (if there are no more ships)
	else if(Hits > 1)
	{
		counter2 = 0;

		for(counter4 = 0; counter4 < 10; counter4++)
		{
			for(counter3 = 0; counter3 < 10; counter3++)
			{
				if(Rows[counter4][counter3] == 1)
				{
					counter2++;
				}
			}
		}

		}
		if(counter2 == 0)
		{
			inGame = 0;
			inMenu = 1;
			InGameTextWritten = 1;
			MainMenuTextWritten = 0;
			for(counter4 = 0; counter4 < 10; counter4++)
			{
				for(counter3 = 0; counter3 < 10; counter3++)
				{
					Rows[counter4][counter3] = 0;
				}
			}
			for(counter = 0; counter < 16; counter++)
			{
				topRow[counter] = youWin[counter];
			}
			lcd_gotoxy(0,0);
			OCR1AL = 0x12;
			break;
		}
	}
}

}

//if pressing middle button, activate LED light
void ledivalo()
{
   /* painetaanko nappia? */ 
     if (PINA & (1 << PA2))
   {
      /* ei: aukaise rele */ 
      PORTA &= ~(1 << PA6); 
   }          
     else
   {
      /* kyll�: kytke rele */
      PORTA |=  (1 << PA6);
   }
}

//function for switching rows
void lcd_gotoxy (unsigned char x, unsigned char y)
    {
      if ( y==0 ) 
	  {
            lcd_write_ctrl(LCD_DDRAM | (LCD_START_LINE1+x));
     }
	 else
	 {
            lcd_write_ctrl(LCD_DDRAM | (LCD_START_LINE2+x));
    	}
    }

//function for checking which button is pressed
int checkButtonPress(int buttonNumber)
{

	buttonNumber = 0;

	//check Middle Button
	if(PINA & (1 << PA2)){
	}
	else
	{
		buttonNumber = 2;

	}

	//Check Left Button
	if(PINA & (1 << PA1)){
	}
	else
	{
		buttonNumber = 1;
	}

	//Check Right Button
	if(PINA & (1 << PA3)){
	}
	else
	{
		buttonNumber = 3;
	}

	//Check Down Button
	if(PINA & (1 << PA4)){
	}
	else
	{
		buttonNumber = 4;
		//sei();
	}

	//Check Up Button
	if(PINA & (1 << PA0)){
	}
	else
	{
		buttonNumber = 5;
	}

	return buttonNumber;
}

//function for calculating location of cursor vertically
int calculateCursorLocationVertical(int cursorLocationY, int currentButton)
{
	//Move cursor Down if pressing Down
	if(currentButton == 4)
	{
		if(cursorLocationY == 0)
		{
			cursorLocationY = cursorLocationY + 1;
		}
	}
	//Move cursor Up if pressing Up
	if(currentButton == 5)
	{
		if(cursorLocationY == 1)
		{
			cursorLocationY = cursorLocationY - 1;
		}
	}

	return cursorLocationY;
}

//function for calculating location of cursor horizontally
int calculateCursorLocationHorizontal(int cursorLocationX, int currentButton)
{

	//Move cursor Right if pressing Right
	if(currentButton == 3)
	{
		if(cursorLocationX < 9)
		{
			cursorLocationX = cursorLocationX + 1;
		}
	}
	//Move cursor Left if pressing Left
	if(currentButton == 1)
	{
		if(cursorLocationX > 0)
		{
			cursorLocationX = cursorLocationX - 1;
		}
	}

	return cursorLocationX;
}

//function that changes printed row char based on int value
char calculateRowNumber(int number, char numberChar)
{
	if(number == 10)
	{
		numberChar = '0';
	}
	else if(number == 1)
	{
		numberChar = '1';
	}
	else if(number == 2)
	{
		numberChar = '2';
	}
	else if(number == 3)
	{
		numberChar = '3';
	}
	else if(number == 4)
	{
		numberChar = '4';
	}
	else if(number == 5)
	{
		numberChar = '5';
	}
	else if(number == 6)
	{
		numberChar = '6';
	}
	else if(number == 7)
	{
		numberChar = '7';
	}
	else if(number == 8)
	{
		numberChar = '8';
	}
	else if(number == 9)
	{
		numberChar = '9';
	}
			
	return numberChar;
}

ISR(TIMER1_COMPA_vect) {

 // vaihdetaan kaiutin pinnien tilat XOR operaatiolla 
  PORTE ^= (1 << PE4) | (1 << PE5); 
}

