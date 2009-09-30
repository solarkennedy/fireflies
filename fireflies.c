/*
Fireflies v.1
Tiny85
7/9/09
Kyle Anderson
*/

#include<avr/io.h>
#include<avr/interrupt.h>
#include<stdlib.h>
#include <stdint.h>
#include <avr/wdt.h>
#include <stdbool.h> 
#include <avr/sleep.h>

#define NumberOfFlys 12

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

typedef unsigned char u08;
int temp;
void setrandom(void);
int makedecision(int fly);
void randomizepersonality(void);
void randomplay(void);
void alloff(void);
void allon(void);
int analogRead(void);
int lightness_counter = 0;
void testpattern(void);

void delay(int var)		//delay of 10ms for var=1
{
	unsigned char il, jl, kl;
	for (il=0; il<var; il++) 
	for (jl=0; jl<32;jl++)
	for (kl=0; kl<249; kl++) 
	asm("NOP");
}
		 
volatile u08 j=1;
volatile u08 l;
volatile u08 anode[NumberOfFlys]={0,0,0,1,1,1,3,3,3,4,4,4};
volatile u08 cathode[NumberOfFlys]={1,3,4,0,3,4,0,1,4,0,1,3};
volatile int flystatus[NumberOfFlys]={0,0,0,0,0,0,0,0,0,0,0,0};
volatile int currentstate[NumberOfFlys]={0,0,0,0,0,0,0,0,0,0,0,0};
volatile int brightness[NumberOfFlys]={0,0,0,0,0,0,0,0,0,0,0,0};
volatile long activity = -1000;
long lightvalue=0;
volatile long tick=0;
int main(void);
volatile u08 pwm[12];
u08 call(u08 n);
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void) \
     __attribute__((naked)) \
     __attribute__((section(".init3")));
void get_mcusr(void)
{
    mcusr_mirror = MCUSR;
    MCUSR = 0;
    wdt_disable();
}
void check_if_sleepy(void);
int main()
{
	get_mcusr(); //Why did we wake up?


        ADCSRA=0b10000000;                      //ADC Enable,ADC Start Conversion,division factor 128,Auto trigger enable
        sbi(MCUCR, PUD);
    	// PB0, PB1, PB3, PB4 are output
	// PB2 is the sensor
	//setup
	TCCR0A = 0x00;  //Timer/Counter Control Register A (normal operation 0)
	TCCR0B = 0x05;  //Timer/Counter Control Register B (2 = clkI/O/8 (From prescaler))
	TIMSK = 0x02;   //Timer/Counter Interrupt Mask Register  (2 = Timer/Counter1 Overflow Interrupt Enable)
	DDRB = 0x1b;    //Port B Data Direction Register
			//1e = DDB4 DDB3 DDB2 DDB1 are outputs?
	PORTB = 0x1b;   //Port B Data Register
			//1e = DDB4 DDB3 DDB2 DDB1 are inputs?
			//DD PO IO     Pullup  Comment
			//1  1  Output No      Output High (Source) (page 57)
	

	sei();  //Set Interrupst enabled
	

//	testpattern();
	wdt_enable(WDTO_8S);

	for(;;)
	{
	check_if_sleepy();
	for(int j = 0; j < NumberOfFlys; j++)
	      {

	        if (flystatus[j] == 1)
       		 {
        	   if (brightness[j] < 500) brightness[j]+=10;
       		 }
     	       else //wantedstate=0
       		 { 
    		   if (brightness[j] > 0) brightness[j]-=10;
                }
       
       
     	      if (brightness[j] > (rand() % 512))
      		 {
        	    currentstate[j]=1;
     		  }
      		     else
      		 {
   	         currentstate[j]=0;
      		 }
		DDRB = 0;
		PORTB = 1<<anode[j]|~currentstate[j]<<cathode[j];
		DDRB = 1<<anode[j]|1<<cathode[j];
        	}
	
	 wdt_reset();
	}//End Loop
}//End Main


//Our Timer Interrupt
ISR(TIM0_OVF_vect)
{
	tick++;
	if (tick % 5== 0)
	{
			if ( analogRead() < 300 ) //This may need to be adjusted for your particular sensor
			{
			lightness_counter=0;
                        //activity = activity + 100;
                        activity = activity + ( rand() % 10 );
                        if (activity >= 100)
                        activity=( 100 - rand () % 1000);
			}
			else
			{
			if (lightness_counter < 100)
			lightness_counter++;
                        activity -= 100;
             	        if (activity > -1000)
                       	activity=-1000;
			}
			randomplay();
	}
	
}

void alloff(){
for (int fly = 0; fly < NumberOfFlys; fly++)
  {
	 flystatus[fly]=0;
  }
}
void allon(){
for (int fly = 0; fly < NumberOfFlys; fly++)
  {
         flystatus[fly]=1;
  }
}



void randomplay(){
long decide = 0;
for (int fly = 0; fly < NumberOfFlys; fly++)
  {
  //decide = ((rand() % 1200) - ( rand() % abs(1000-activity)) + activity );
  decide = ((rand() % 1000 ) + activity );
//  decide = 900;
 // int decide = random(20) + personality[fly];
	if (decide < 800 )
 flystatus[fly]=0;
	if (decide > 500 )
 flystatus[fly]=1;
}

}


int analogRead(void)
{
        uint8_t low, high;
        ADMUX = 0b00000001;
        sbi(ADCSRA, ADSC);
        while (bit_is_set(ADCSRA, ADSC));
        low = ADCL;
        high = ADCH;
        return (high << 8) | low;
}



void check_if_sleepy(){
bool readyforbed = true;
for(int j = 0; j < NumberOfFlys; j++)
     {
	if ( flystatus[j] == 1 )
	readyforbed=false;
     }
if ( lightness_counter < 10 )
	readyforbed=false;
cli();
if ( readyforbed==true )
{ 
     DDRB = 0;
//     PORTB = 1<<anode[0]|0<<cathode[0];
 //    DDRB = 1<<anode[0]|1<<cathode[0];

     set_sleep_mode(SLEEP_MODE_PWR_DOWN);
     sleep_mode();
     sleep_enable();
     sei();
     sleep_cpu();
     sleep_disable();
}
sei();
}
