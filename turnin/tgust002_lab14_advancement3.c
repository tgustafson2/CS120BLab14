/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #14  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <time.h>
#include <stdlib.h>
#include <timer.h>
#include <scheduler.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum O_States{OSM_Start,OSM_Output};
enum A_States{ASM_Start,ASM_S0};
enum I_States{ISM_Start,ISM_Wait,ISM_Left,ISM_Right};
enum B_States{BSM_Start,BSM_Move,BSM_Reset};
enum T_States{TSM_Start,TSM_S0,TSM_S1,TSM_S2,TSM_S3};
enum P_States{PSM_Start,PSM_AI,PSM_Player};
enum H_States{HSM_Start,HSM_Wait,HSM_Left,HSM_Right};
unsigned short mid=519;
task task1,task2,task3,task4,task5,task6,task7,task8;
unsigned char pattern[3];
unsigned char row[3];
unsigned char aiPattern,pPattern,opponent;
unsigned short bx, by;
unsigned char reset;
unsigned char p1,p2;
int H_Tick(int state){
	switch (state){
		case HSM_Start:
			pPattern=0x38;
			state=HSM_Wait;
			break;
		case HSM_Wait:
			if(((~PINB)&0x06)==0x02){
				if((pPattern&0x01)!=0x01){
					pPattern=pPattern>>1;
				}
				//pPattern=0x01;
				HSM_Left;
			}
			else if(((~PINB)&0x06)==0x04){
				if((pPattern&0x80)!=0x80){
					pPattern=pPattern<<1;
				}
				//pPattern=0x80;
				state=HSM_Right;
			}
			else if(((~PINB)&0x06)==0x00){
				state=HSM_Wait;
			}
			break;
		case HSM_Left:
                        if(((~PINB)&0x06)==0x04){
                                if((pPattern&0x80)!=0x80){
                                        pPattern=pPattern<<1;
                                }
                                state=HSM_Right;
                        }
                        else if(((~PINB)&0x06)==0x00){
                                state=HSM_Wait;
                        }
                        break;
		case HSM_Right:
			if(((~PINB)&0x06)==0x02){
                                if((pPattern&0x01)!=0x01){
                                        pPattern=pPattern>>1;
                                }
                                HSM_Left;
                        }
                        else if(((~PINB)&0x06)==0x00){
                                state=HSM_Wait;
                        }
                        break;
		default:
			pPattern=0x38;
			state=HSM_Wait;
	}
	return state;
}

int P_Tick(int state){
	switch(state){
		case PSM_Start:
			opponent=0x00;
			state=PSM_AI;
			break;
		case PSM_AI:
			if((~PINB)&0x01){
				state=PSM_Player;
				opponent=0x01;
			}
			break;
		case PSM_Player:
			if(!((~PINB)&0x01)){
				state=PSM_AI;
				opponent=0x00;
			}
			break;
		default:
			opponent=0x00;
			state=PSM_AI;
	}
	return state;
}
int A_Tick(int state){
	unsigned char temp1;
	switch(state){
		case ASM_Start:
			aiPattern=0x38;
			state=ASM_S0;
			break;
		case ASM_S0:
			temp1=rand()%10;
			if(temp1<7){
				if(pattern[1]>aiPattern&&(aiPattern&0x80)!=0x80){
					aiPattern=aiPattern<<1;
					}
				else if(pattern[1]<aiPattern&&(aiPattern&0x01)!=0x01){
					aiPattern=aiPattern>>1;
				}
			}
			else if(temp1==7){
				if(pattern[1]>aiPattern&&(aiPattern&0x01)!=0x01){
                                        aiPattern=aiPattern>>1;
                                        }
                                else if(pattern[1]<aiPattern&&(aiPattern&0x80)!=0x80){
                                        aiPattern=aiPattern<<1;
                                }
			}
			break;
		default:
			aiPattern=0x38;
			state=ASM_S0;
	}
	return state;
}

int BX_Tick(int state){
	switch(state){
		case BSM_Start:
			bx=5;
			state=BSM_Move;
		case BSM_Move:
			if(reset)bx=(char)rand()%10;
			if(bx>4){
				if((pattern[1]&0x01)!=0x01){
				pattern[1]=pattern[1]>>1;}
				else{
					pattern[1]=pattern[1]<<1;
					bx=4;
				}
			}
			else{
				if((pattern[1]&0x80)!=0x80){
					pattern[1]=pattern[1]<<1;}
				else{
					pattern[1]=pattern[1]>>1;
					bx=5;
				}
			}
			break;
		default:
			bx=5;
			state=BSM_Move;
	}
	return state;
}

int BY_Tick(int state){
	switch(state){
		case BSM_Start:
			pattern[1]=0x10;
			row[1]=0xfb;
			by=2;
			p1=0;
			p2=0;
			state=BSM_Move;
			break;
		case BSM_Move:
			if(by<3){
				row[1]=row[1]<<1|0x01;
			}
			else{
				row[1]=row[1]>>1|0x10;
			}
			if((row[1]&0x01)==0x00||(row[1]&0x10)==0x00){
				if((row[1]&0x01)==0x00&&(pattern[1]&pattern[0])==pattern[1]){
					by=2;
					row[1]=row[1]<<2|0x03;
					if(((pattern[1]>>1)&pattern[0])==(pattern[1]>>1)&&((pattern[1]<<1)&pattern[0])==(pattern[1]<<1)){
						task4.period+=15;
						task5.period+=15;
					}
					else{
						task4.period-=40;
						task5.period-=40;
					}
				}
				else if((row[1]&0x10)==0x00&&(pattern[1]&pattern[2])==pattern[1]){
					by=3;
					row[1]=row[1]>>2|0x18;
					if(((pattern[1]>>1)&pattern[2])==(pattern[1]>>1)&&((pattern[1]<<1)&pattern[2])==(pattern[1]<<1)){
                                                task4.period+=15;
                                                task5.period+=15;
                                        }
					else{
						task4.period-=40;
						task5.period-=40;
					}
				}
				else{
				reset=0x01;
				if((row[1]&0x01)==0x01)p1++;
				else if((row[1]&0x10)==0x10)p2++;
				if(p1>2||p2>2){
					p1=0;
					p2=0;}
				task4.period=750;
				task5.period=750;
				state=BSM_Reset;
				row[1]=0xfb;
				pattern[1]=0x10;}
			}
			break;
		case BSM_Reset:
			state=BSM_Move;
			reset=0x00;
			break;
		default:
			row[1]=0xfb;
			pattern[1]=0x08;
			state=BSM_Move;
	}
	return state;
}
int I_Tick(int state){
	unsigned short my_input=ADC;
	switch(state){
		case ISM_Start:
			pattern[2]=0x38;
			state=ISM_Wait;
			break;
		case ISM_Wait:
			if(my_input<mid-10){
				state=ISM_Left;
				if((pattern[2]&0x80)!=0x80){
					pattern[2]=pattern[2]<<1;
				}
			}
			else if(my_input>mid+10){
				state=ISM_Right;
				if((pattern[2]&0x01)!=0x01)pattern[2]=pattern[2]>>1;
			}
			break;
		case ISM_Left:
			if(my_input>=mid-10&&my_input<=mid+10){
				state=ISM_Wait;
			}
			else if(my_input>mid+10){
                                state=ISM_Right;
                                if((pattern[2]&0x01)!=0x01)pattern[2]=pattern[2]>>1;
                        }
			else{
				if((pattern[2]&0x80)!=0x80){
                                        pattern[2]=pattern[2]<<1;
                                }
			}
			break;
		case ISM_Right:
			if(my_input>=mid-10&&my_input<=mid+10){
                                state=ISM_Wait;
                        }
			else if(my_input<mid-10){
                                state=ISM_Left;
                                if((pattern[2]&0x80)!=0x80){
                                        pattern[2]=pattern[2]<<1;
                                }
                        }
			else{
                                if((pattern[2]&0x01)!=0x01)pattern[2]=pattern[2]>>1;
                        }
			break;
		default:
			state=ISM_Wait;
			pattern[2]=0x38;
	}
	return state;
}

int T_Tick(int state){
	unsigned short temp=ADC;
	switch(state){
		case TSM_Start:
			state=TSM_S0;
			break;
		case TSM_S0:
			if(temp<=(mid-(170*3))||temp>=(mid+(160*3))){
				task2.period=100;
				state=TSM_S1;
			}
			else if(temp<=(mid-(170*2))||temp>=(mid+(160*2))){
                                task2.period=250;
                                state=TSM_S2;
                        }
			else if(temp<=(mid-(170))||temp>=(mid+(160))){
                                task2.period=500;
                                state=TSM_S3;
                        }
			else{
				task2.period=1000;
				state=TSM_S0;
			}
			break;
		case TSM_S1:
			if(temp<=(mid-(170*3))||temp>=(mid+(160*3))){
                                task2.period=100;
                                state=TSM_S1;
                        }
                        else if(temp<=(mid-(170*2))||temp>=(mid+(160*2))){
                                task2.period=250;
                                state=TSM_S2;
                        }
                        else if(temp<=(mid-(170))||temp>=(mid+(160))){
                                task2.period=500;
                                state=TSM_S3;
                        }
			else{
                                task2.period=1000;
                                state=TSM_S0;
                        }
                        break;
		case TSM_S2:
			if(temp<=(mid-(170*3))||temp>=(mid+(160*3))){
                                task2.period=100;
                                state=TSM_S1;
                        }
                        else if(temp<=(mid-(170*2))||temp>=(mid+(160*2))){
                                task2.period=250;
                                state=TSM_S2;
                        }
                        else if(temp<=(mid-(170))||temp>=(mid+(160))){
                                task2.period=500;
                                state=TSM_S3;
                        }
                        else{
                                task2.period=1000;
                                state=TSM_S0;
                        }
                        break;
		case TSM_S3:
			if(temp<=(mid-(170*3))||temp>=(mid+(160*3))){
                                task2.period=100;
                                state=TSM_S1;
                        }
                        else if(temp<=(mid-(170*2))||temp>=(mid+(160*2))){
                                task2.period=250;
                                state=TSM_S2;
                        }
                        else if(temp<=(mid-(170))||temp>=(mid+(160))){
                                task2.period=500;
                                state=TSM_S3;
                        }
                        else{
                                task2.period=1000;
                                state=TSM_S0;
                        }
                        break;
		default:
			task2.period=1000;
			state=TSM_S0;
	}
	return state;
}

int O_Tick(int state){
	static unsigned char j;
	switch(state){
		case OSM_Start:
			state=OSM_Output;
			j=0;
			break;
		case OSM_Output:
			if(opponent==0x00){
			pattern[0]=aiPattern;}
			else pattern[0]=pPattern;
			transmit_data(pattern[j],2);
			transmit_data(row[j],1);
			j++;
			if(j==3)j=0;
			PORTD=p1|p2<<2;
			break;
		default:
			state=OSM_Output;
	}
	return state;
}




void A2D_init(){
	ADCSRA|=(1<<ADEN)|(1<<ADSC)|(1<<ADATE);
	//ADEN: Enables analog-to-digital conversion
	//ADSC: Starts analog-to-digital conversion
	//ADATE: Enables auto triggering, allowing for constant analog to digital conversions
}

void transmit_data(unsigned char data, unsigned char s){
	int i;
	if (s==0x01){
	for (i=0;i<8;i++){
		//Sets SRCLR to 1 allowing data to be set
		//Also clears SRCLk in preparation of sending data
		PORTC=0x08;
		//set SET=nex bit of data to be sent
		PORTC|=((data>>i)&0x01);
		//set SRCLK=1. Rising edge shifts next bit of data into the shift register
		PORTC|=0x02;
	}
	//set RCLK=1. Rising edge copies data from "Shift" register to "Storage register
	PORTC|=0x04;
	//clears all lines in perpartion of a new transmission
	PORTC|=0x00;}
	else if(s==0x02){
	for(i=0;i<8;i++){
                //Sets SRCLR to 1 allowing data to be set
                //Also clears SRCLk in preparation of sending data
                PORTC=0x20;
                //set SET=nex bit of data to be sent
                PORTC|=((data>>i)&0x01);
                //set SRCLK=1. Rising edge shifts next bit of data into the shift register
                PORTC|=0x02;
        }
        //set RCLK=1. Rising edge copies data from "Shift" register to "Storage" register
        PORTC|=0x10;
        //clears all lines in perpartion of a new transmission
        PORTC|=0x00;}

}

int main(void) {
    /* Insert DDR and PORT initializations */
    srand((int)time(0));
    DDRC=0xFF; PORTC=0x00;
    DDRB=0x00; PORTB=0xFF;
    DDRD=0xFF; PORTD=0x00;
    //DDRA=0x00; PORTA=0xFF;
    //static task task1,task2;
    task *tasks[]={&task1,&task2,&task3,&task4,&task5,&task6,&task7,&task8};
    const unsigned short numTasks=sizeof(tasks)/sizeof(task*);
    /* Insert your solution below */
    task1.state=OSM_Start;
    task1.period=1;
    task1.elapsedTime=task1.period;
    task1.TickFct=&O_Tick;
    task2.state=ISM_Start;
    task2.period=1000;
    task2.elapsedTime=task2.period;
    task2.TickFct=&I_Tick;
    task3.state=TSM_Start;
    task3.period=1;
    task3.elapsedTime=task3.period;
    task3.TickFct=&T_Tick;
    task4.state=BSM_Start;
    task4.period=750;
    task4.elapsedTime=task4.period;
    task4.TickFct=&BY_Tick;
    task5.state=BSM_Start;
    task5.period=750;
    task5.elapsedTime=task5.period;
    task5.TickFct=&BX_Tick;
    task6.state=ASM_Start;
    task6.period=500;
    task6.elapsedTime=task6.period;
    task6.TickFct=&A_Tick;
    task7.state=PSM_Start;
    task7.period=1;
    task7.elapsedTime=task7.period;
    task7.TickFct=&P_Tick;
    task8.state=HSM_Start;
    task8.period=100;
    task8.elapsedTime=task8.period;
    task8.TickFct=&H_Tick;
    TimerSet(1);
    TimerOn();
    A2D_init();
    row[0]=0xfe;
    row[1]=0xfB;
    row[2]=0xef;
    aiPattern=0x38;
    pattern[1]=0x08;
    unsigned char i;
    while (1) {
	    for (i=0;i<numTasks;i++){
		    if(tasks[i]->elapsedTime>=tasks[i]->period){
		    	tasks[i]->state=tasks[i]->TickFct(tasks[i]->state);
		    	tasks[i]->elapsedTime=0;
		    }
		    tasks[i]->elapsedTime+=1;
	    }
	    while(!TimerFlag);
	    TimerFlag=0;

    }
    return 1;
}
