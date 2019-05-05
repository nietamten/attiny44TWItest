#define F_CPU 8000000

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  
#include <util/delay.h>    
 
#include "USI_TWI_Slave.h"

volatile uint16_t reg1 = 0xaaaa;
volatile uint16_t reg2 = 0x3421;
volatile uint16_t reg3 = 0x3421;
volatile uint8_t reg4 = 0x3421;
volatile uint32_t reg5 = 0x34213443;

uint32_t buff = 0x1234;
uint8_t cnt = 0;
volatile uint8_t addr = 0;

void* regs[] = 			{	&reg1,	&ADC,	&reg2,	&OCR0A,	&reg5	};
uint8_t regSizes[] =	{	2,		2,		2,		1,		4	}; //max 4

//volatile int x = 0;

void usiRequest() 
{
	for(uint8_t i=regSizes[addr];i>0;i--)
	{
		uint32_t v = *((uint32_t*) (regs[addr]));
		usiTwiTransmitByte(v>>((i-1)*8));
	}
	addr = 0;
}

void initPWM()
{
	DDRB = (1<<PB2); //pwm out
	
	TCCR0A |= (1 << WGM00) | (1 << WGM01) | (1 << COM0A1)| (1 << COM0A0);
	TCCR0B |= (1<<CS00); //1
	//TCCR0B |= (1<<CS00) | (1<<CS02); //1024
}

void SetPWMoutput(int duty)
{
	OCR0A = duty; 
}

ISR(ADC_vect)
{
}

void initADC()
{
	ADMUX  =    (1<<REFS1) | (1 <<MUX3) | (1 << MUX0) ;

	ADCSRA |= (1 << ADEN); // Enable ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler to 128
	ADCSRB |= (1 << BIN);   //differential/binary mode
	ADCSRA |= (1 << ADIE);  //interrupt eable
	ADCSRA |= (1 << ADATE);  //autotrigger conversion (free running mode)
	ADCSRA |= (1 << ADSC); // Start the conversion
}

int main(void)
{
	initADC();
	
	initPWM();
	SetPWMoutput(245);
	
	DDRA = (1<<PA2);
	PORTA |= (1<<PA2);

	sei();

	usi_onRequestPtr = usiRequest;

	usiTwiSlaveInit(0x02);

	wdt_enable(WDTO_4S);

	while(1)
	{
		wdt_reset();

		if(usiTwiAmountDataInReceiveBuffer())
		{
			if(!addr)
			{
				addr = usiTwiReceiveByte();
				cnt = 0;
				buff = 0;
			}
			else
			{
				buff |= (uint32_t)usiTwiReceiveByte()<<((3-cnt)*8);
				
				cnt++;
				if(cnt>(regSizes[addr]-1))
				{
					buff = buff>>((4-cnt)*8);
					uint32_t *vp = (uint32_t*)regs[addr];
					memcpy(vp,&buff,regSizes[addr]);
					addr = 0;
				}
			}
		}
	}
}

