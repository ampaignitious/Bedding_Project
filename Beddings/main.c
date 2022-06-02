#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define F_CPU 1000000UL
#include <util/delay.h>
char input[20];

char* size[]={"small size", "medium size","large size"};
int chosen_size = -1;
int chosen_time =-1;


void enable(){
	PORTG |= (1<<5); //en - give a 1
	_delay_ms(10);
	PORTG &= ~(1<<5); //en - give a 0
	_delay_ms(10);
}

void start_lcd(){
	PORTG &= ~(1<<4); // r/w - writing
	PORTG &= ~(1<<3); //rs - command mode
	
	PORTH = 0x38; // set 8 data lines 0011 1111
	enable();
	
	PORTH = 0x0f; // blink 0000 1111
	enable();
	
	PORTH = 0x10; // display 0001 1100
	enable();
}

void print(char word[20],int row){
	PORTG &= ~(1<<3); //rs - command mode
	enable();
	int positions[] = {0x80,0xc0,0x94,0xd4};
	PORTH = positions[row]; // set print out 1001 0010
	enable();
	
	//switch to data mode
	PORTG |= (1<<3);
	enable();
	for(int i=0; i<strlen(word); i++){
		PORTH = word[i];
		enable();
	}
}

void clear(){
	PORTG &= ~(1<<3); //rs - command mode
	enable();
	PORTH = 0x01;
	enable();
}

char* choose(){
	strcpy(input,"");
	DDRJ = 0xf8;
	char selections[4][3] = {{'1','2','3'},{'4','5','6'},{'7','8','9'},{'*','0','#'}};
	
	/* Replace with your application code */
	while (1) {
		char choice = '*';
		int choice_set = 0;
		for (int i=3; i<=6; i++){
			PORTJ = ~(1<<i);
			if((PINJ&(1<<2))==0)
			choice = selections[choice_set][0];
			if((PINJ&(1<<1))==0)
			choice = selections[choice_set][1];
			if((PINJ&(1<<0))==0)
			choice = selections[choice_set][2];
			choice_set++;
		}
		
		if (choice=='#')
			break;
		else if (choice=='*')
			continue;
		else{
			strncat(input, &choice, 1);
			_delay_ms(500);
		}
	}
	return input;
}

void config(){
	print("WELCOME TO", 0);
	print("COMFORTABLE BEDDING",1);
	print("SYSTEM",2);
	_delay_ms(1000);
	clear();
	start_lcd();
	
	print("SET CONFIGURATION",0);
	print("ENTER MATTRESS_TYPE",1);
	//print("2.TIME",2);
	//print("3.TOTAL_NO.",3);
	_delay_ms(1000);
	
	clear();
	start_lcd();
	print("1.SMALL",0);
	print("2.MEDIUM",1);
	print("3.LARGE",2);
	char* z =choose();
	chosen_size = atoi(z);
	clear();
	print(size[chosen_size-1], 0);
	print("ENTER TIME FOR", 1);
	print("SELECTED", 2);
	print("MATTRESS: ",3);
	 char *t = choose();
	 chosen_time = atoi(t);
	 clear();
	 start_lcd();
	 
	 char str[6];
	 sprintf(str, "%d", chosen_time);
	 
	 //print("Time entered: ",0);
	 print(str, 1);
	 //print(" seconds",1);
	//if(p==2){
		//clear();
		//print("ENTER TIME: ",0);
	//}
	//if(p==3){
		//print("ENTER TOTAL MATRESS:", 0);
	//}
}


int main(void){
	//char word[] = "COMFORTABLE BEDDINGS";
    /* Replace with your application code */
	DDRC = 0xff;
	DDRD = 0x00;
	DDRG = 0xff;
	DDRH = 0xff;
	DDRK = 0xff;
	DDRJ = 0x00;
	
	start_lcd();
	config();
	
	
    while (1) 
    {
    }
}

