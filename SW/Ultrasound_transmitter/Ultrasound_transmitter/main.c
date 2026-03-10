#define F_CPU 10000000UL

#include <xc.h>
#include <util/delay.h>

#define DIO PIN2_bm //portB
#define CLK PIN3_bm //portB
#define WO0 PIN0_bm //portB

// Buttons
#define PLAYB PIN1_bm	 //PORTA
#define NXTB PIN2_bm	 //PORTA
#define PREVB PIN3_bm	 //PORTA
//
#define LED PIN1_bm // portB

uint8_t FREQ = 0;	// KHZ to transmit

 uint8_t b_next = 0;
 uint8_t b_prev = 0;
 uint8_t b_play = 0;
 
 uint8_t enable = 0;

const uint8_t digitMap[10] = {
	
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F  // 9
};

void TM_sendByte(uint8_t data){
	for(int i = 0; i<8; i++){
		
		PORTB.OUTCLR = CLK;
		
		if (data & (1<<i)) // if value at index (lsb first)
		{
			PORTB.OUTSET = DIO;
		}
		else {
			PORTB.OUTCLR = DIO;
		}
		
		PORTB.OUTSET = CLK;
	}
	
	// WAIT FOR ACK
	PORTB.OUTCLR = CLK;
	PORTB.DIRCLR = DIO; // To input
	PORTB.OUTSET = CLK;
	_delay_ms(1);
	PORTB.OUTCLR = CLK;
	PORTB.DIRSET = DIO; // Back to Output
}

void TM_start(void){
	PORTB.OUTSET = CLK | DIO;
	PORTB.OUTCLR = DIO;
	PORTB.OUTCLR = CLK;
}

void TM_stop(void){
	PORTB.OUTCLR = CLK;
	PORTB.OUTCLR = DIO;
	PORTB.OUTSET = CLK;
	PORTB.OUTSET = DIO;
}
void TM_displayInit(void){
	TM_start();
	TM_sendByte(0x40);  // auto increment
	TM_stop();

	TM_start();
	TM_sendByte(0x88 | 0x07); // display ON, max brightness
	TM_stop();
}

void TM_printNumber(uint16_t num)
{
	uint8_t digits[4];

	if (num > 9999) num = 9999;

	digits[0] = digitMap[(num / 1000) % 10];
	digits[1] = digitMap[(num / 100)  % 10];
	digits[2] = digitMap[(num / 10)   % 10];
	digits[3] = digitMap[num % 10];

	TM_start();
	TM_sendByte(0xC0);      // Start form 0
	for (uint8_t i = 0; i < 4; i++)
	TM_sendByte(digits[i]);
	TM_stop();
	
}

void setup(void){
	
	CPU_CCP = CCP_IOREG_gc; // Write protection off
	CLKCTRL.MCLKCTRLB = CLKCTRL_PEN_bm | CLKCTRL_PDIV_2X_gc; // 20 MHz clk, 2x Prescale = 10 MHZ

	PORTB.DIRSET = DIO; // to output
	PORTB.DIRSET = CLK; // to output
	PORTB.DIRSET = WO0; // to output
	PORTB.DIRSET = LED; // to output
	
	PORTA.DIRCLR = PLAYB; // to input
	PORTA.PIN1CTRL = PORT_PULLUPEN_bm; // pullup on
	PORTA.DIRCLR = NXTB; // to input
	PORTA.PIN2CTRL = PORT_PULLUPEN_bm; // pullup on
	PORTA.DIRCLR = PREVB; // to input
	PORTA.PIN3CTRL = PORT_PULLUPEN_bm; // pullup on
	
	TCA0.SINGLE.PER = 1000; // number to count to
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc; // CLKDIV 16
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // counter enable
	
	TM_displayInit();
}

void set_frequency(uint8_t fq){
	TCA0.SINGLE.PER = 40 - fq;
}
 
 void read_buttons(void){
	 
	 b_next = 0;
	 b_prev = 0;
	 b_play = 0;

	 uint8_t buttonreg = PORTA.IN;
	 if (!(buttonreg && NXTB))
	 {
		 b_next = 1;
	 }
	 if (!(buttonreg && PREVB))
	 {
		 b_prev = 1;
	 }
	 if (!(buttonreg && PLAYB))
	 {
		 b_play = 1;
	 }
 }
 
 void select_frequency(void){
	 if (b_next)
	 {
		 FREQ++;
	 }
	 if (b_prev)
	 {
		 if (FREQ != 0)
		 {
			FREQ--; 
		 }
	 }
	 
	 set_frequency(FREQ);
 }
 
 void sound_disable(void){
	 PORTB.OUTCLR = LED;
	 enable = 0;
	 TCA0.SINGLE.CTRLA &~ TCA_SINGLE_ENABLE_bm ; // counter disable
 }
 void sound_enable(){
	 PORTB.OUTSET = LED;
	 enable = 1;
	 TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // counter enable
 }
  
int main(void)
{
	setup();
    while(1)
    {
		
        read_buttons();
		select_frequency();
		if (b_play)
		{
			if (enable == 0)
			{
				enable = 1;
			}
			else {
				enable = 0;
			}
		}
		
		_delay_ms(200);
    }
}