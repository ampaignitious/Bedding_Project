#include <avr/io.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#define F_CPU 1000000UL
#include <util/delay.h>

char input[20];
char eeprom_data[80];
char* size[]={"small size", "medium size","large size"};
int totals[]={0,0,0};
long overall_totals[]={0,0,0};
int times[]={0,0,0};
char numeric_str[10];

void EEPROM_write(int start_addr, int stop_addr, char* ucData)
{
	for(int i=strlen(ucData)-1; stop_addr>=start_addr; i--){
		// Wait for completion of previous write
		while(EECR & (1<<EEPE));
		
		// Set up address and Data Registers
		EEAR = stop_addr;
		
		if(i>=0){
			EEDR = ucData[i];
		}
		else{
			EEDR = '0';
		}
		
		// Write logical one to EEMPE
		EECR |= (1<<EEMPE);
		
		// Start eeprom write by setting EEPE
		EECR |= (1<<EEPE);
		stop_addr--;
	}
}

char* EEPROM_read(unsigned int start_addr, unsigned int stop_addr){
	strcpy(eeprom_data,"");
	for(;start_addr<=stop_addr; start_addr++){
		// Wait for completion of previous write
		while(EECR & (1<<EEPE));
		
		// Set up address register
		EEAR = start_addr;
		
		// Start eeprom read by writing EERE
		EECR |= (1<<EERE);
		
		// Return data from Data Register
		strncat(eeprom_data, &EEDR, 1);
	}
	return eeprom_data;
}

char* num_to_str(unsigned long num){
	sprintf(numeric_str, "%lu", num);
	return numeric_str;
}

void enable(){
	PORTG |= (1<<5); //en - give a 1
	_delay_ms(10);
	PORTG &= ~(1<<5); //en - give a 0
	_delay_ms(10);
}

void start_lcd(){
	PORTC |=(1<<2);
	PORTG &= ~(1<<4); // r/w - writing
	PORTG &= ~(1<<3); //rs - command mode
	
	PORTH = 0x3f; // set 8 data lines 0011 1111
	enable();
	
	PORTH = 0x0f; // blink 0000 1111
	enable();
	
	PORTH = 0x10; // display 0001 1100
	enable();
}

void stop_lcd(){
	PORTG &= ~(1<<3); //rs - command mode
	PORTH = 0x08; // of
	enable();
}

void print1(char* messages[], int rows[],int num){
	stop_lcd();
	start_lcd();
	
	for(int i=0; i<=num; i++){
		if (rows[i]>0){
			PORTG &= ~(1<<3); //rs - command mode
			enable();
			int positions[] = {0x80,0xbf,0x93,0xd3};
			PORTH = positions[rows[i]]; // set row
			enable();
		}
		
		//switch to data mode
		PORTG |= (1<<3);
		enable();
		
		if (rows[i]==0){
			PORTH = ' ';
			enable();
		}
		
		for(int y=0; y<strlen(messages[i]); y++){
			PORTH = messages[i][y];
			enable();
		}
	}
}

void print(char word[20],int row){
	PORTG &= ~(1<<3); //rs - command mode
	enable();
	if (row>0){
		int positions[] = {0x80,0xbf,0x93,0xd3};
		PORTH = positions[row]; // set print out 1001 0010
		enable();
	}
	
	//switch to data mode
	PORTG |= (1<<3);
	enable();
	
	if (row==0){
		PORTH = ' ';
		enable();
	}
	
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
	start_lcd();
	return input;
}

void load_config(){
	totals[0] = atoi(EEPROM_read(1,10));
	totals[1] = atoi(EEPROM_read(11,20));
	totals[2] = atoi(EEPROM_read(21,30));
	
	times[2] = atoi(EEPROM_read(31,40));
	times[1] = times[2]/2;
	times[0] = times[2]/4;
}

void config(){
	PORTC =0x00000010;
	char t[20];
	print("WELCOME TO", 0);
	print("COMFORTABLE BEDDINGS",1);
	print("SYSTEM",2);
	_delay_ms(1000);
	clear();
	start_lcd();
	
	print("SET CONFIGURATIONS",0);
	_delay_ms(1000);
	
	clear();
	print("ENTER TIME", 0);
	print("FOR LARGE", 1);
	print("MATTRESS: ",2);
	strcpy(t,choose());
	times[2] = atoi(t);
	times[1] = times[2]/2;
	times[0] = times[2]/4;
	EEPROM_write(31,40,t);
	strcat(t," seconds");
	clear();
	print("TIME SELECTED:", 0);
	print(t, 1);
	_delay_ms(1000);
	clear();
	
	print("ENTER NUMBER", 0);
	print("OF MATTRESSES", 1);
	print("TO MANUFACTURE",2);
	print("FOR EACH SIZE",3);
	_delay_ms(1500);
	clear();
	print("SMALL SIZE:", 0);
	strcpy(t,choose());
	EEPROM_write(1,10,t);
	totals[0] = atoi(t);
	strcat(t," mattresses");
	print(t, 1);
	_delay_ms(1500);
	clear();
	
	print("MEDIUM SIZE:", 0);
	strcpy(t,choose());
	totals[1] = atoi(t);
	EEPROM_write(11,20,t);
	strcat(t," mattresses");
	print(t, 1);
	_delay_ms(1500);
	clear();
	
	print("LARGE SIZE:", 0);
	strcpy(t,choose());
	totals[2] = atoi(t);
	EEPROM_write(21,30,t);
	strcat(t," mattresses");
	print(t, 1);
	_delay_ms(1500);
	clear();
}

void manufacture(int z){
	PORTC =0x00000100;
	char message[60];
	for (; z<3; z++){
		EEPROM_write(0,0,num_to_str(z+1));
		int overall_start_loc = (z*10)+41;
		int rem_start_loc = (z*10)+1;
		long curr_sum = atoi(EEPROM_read(overall_start_loc,overall_start_loc+9));
		for(int i = totals[z]; i>0;){
			start_lcd();
			sprintf(message,"MANUFACTURING...%d",i);
			strcat(message,size[z]);
			strcat(message," remaining");
			
			char* l[] = {message};
			int m[] = {1};
			print1(l,m,1);
			
			for (int a=0; a<times[z]; a++){
				_delay_ms(1000);
			}
			EEPROM_write(rem_start_loc,rem_start_loc+9,num_to_str(--i));
			EEPROM_write(overall_start_loc,overall_start_loc+9,num_to_str(++curr_sum));
			if(i==0){
				clear();
				strcpy(message,size[z]);
				strcat(message," complete");
				print(message,1);
				_delay_ms(500);
			}
		}
		clear();
		print("Packing has started",1);
	}
	EEPROM_write(0,0,"0");
}

void init(){
	char* y = EEPROM_read(0,0);
	if (!isdigit(y[0])){
		for(int i=0; i<10; i++){
			EEPROM_write(10*i,9+(10*i),"0");
		}
	}
	else{
		short state = atoi(y);
		if (state>0){
			load_config();
			manufacture(state-1);
		}
	}
	
}

int main(void){	
	DDRC = 0xff;
	DDRD = 0x00;
	DDRG = 0xff;
	DDRH = 0xff;
	DDRK = 0xff;
	DDRJ = 0x00;
	
	
	start_lcd();
	//initializing the eeprom from addr 0 to 100 with default zero values
	init();
	
	config();
	manufacture(0);
	
	//EEPROM_write(0,0,"0");
	//char* y = EEPROM_read(0,0);
	//print(y, 1);
	
	while (1);
}

