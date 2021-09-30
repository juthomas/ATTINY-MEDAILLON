#include <Arduino.h>
#include "medaillon.h"
#include <util/delay.h>


//https://www.electronics-lab.com/project/attiny85-push-button-power-switching-software-solution/

uint8_t buffer[64 * 3] = {0};
uint8_t colors_buffer[4][32];
uint8_t tmp;
float luminosity = 0.05;
uint8_t current_frame = 0;
volatile uint8_t x = 0;
volatile uint8_t y = 0;
int delayval = 100;
volatile uint8_t buttonFlag = 0;
volatile uint8_t buttonPressed = false;

ISR(TIMER1_COMPA_vect)     //Interrupt vector for Timer0
{
	if(!buttonFlag)
		buttonFlag = (~(PINB >> 1)) & 0x0F;
	else if (buttonPressed && !(~(PINB >> 1) & 0x0F))
	{
		buttonPressed = false;
		buttonFlag = 0;
	}
}

void timer_setup()
{
	TCCR1 = 0; // Stop timer
  	TCNT1 = 0; // Zero timer
  	GTCCR = _BV(PSR1); // Reset prescaler
  	OCR1A = 243; // T = prescaler / 1MHz = 0.004096s; OCR1A = (1s/T) - 1 = 243
  	OCR1C = 243; // Set to same value to reset timer1 to 0 after a compare match
  	TIMSK = _BV(OCIE1A); // Interrupt on compare match with OCR1A
  
  // Start timer in CTC mode; prescaler = 4096; 
  TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS12) | _BV(CS10);
  sei();
}

void external_interrupt()
{
  DDRB |= (1<<PB1)|(1<<PB0);     // set PB2 as output(LED)
  sei();     //enabling global interrupt
  //GIMSK |= (1<<INT0);     // enabling the INT0 (external interrupt) 
  //MCUCR |= (1<<ISC01);    // Configuring as falling edge 
}

void pin_change_interrupt()
{
	//sei();
  //DDRB|=(1<<PB0);
  GIMSK|= (1<<PCIE);
  PCMSK|= (1<<PCINT1) |(1 << PCINT2) | (1<<PCINT3) |(1 << PCINT4);;
}

void setup() {
	DDRB = 1 << PB0;
	for (uint8_t u = 0; u < 4; u++)
	{
		for (uint8_t i = 0; i < 32; i++)
		{
			colors_buffer[u][i] =  (u << 4) + u;
		}
	}
	timer_setup();
	led_send_data(buffer, 64);

}
uint32_t colors[] = {
	0x000000,
	0xf44336,
	0xe81e63,
	0x9c27b0,
	0x673ab7,
	0x3f51b5,
	0x2196f3,
	0x00bcd4,
	0x009688,
	0x4caf50,
	0x8bc34a,
	0xcddc39,
	0xffeb3b,
	0xffc107,
	0xff9800,
	0xffffff
};

uint32_t get_reduced_color(uint32_t color)
{
	uint8_t	g = ((color & 0x00FF00) >> 8) * luminosity;
	uint8_t	r = ((color & 0xFF0000) >> 16) * luminosity;
	uint8_t	b = (color & 0x0000FF) * luminosity;
	return (((uint32_t)g << 8) + ((uint32_t)r << 16) + (uint32_t)b);
}

uint32_t get_reduced_invert_color(uint32_t color)
{
	uint8_t	g = (0xFF - ((color & 0x00FF00) >> 8)) * luminosity;
	uint8_t	r = (0xFF - ((color & 0xFF0000) >> 16)) * luminosity;
	uint8_t	b = (0xFF - (color & 0x0000FF)) * luminosity;
	return (((uint32_t)g << 8) + ((uint32_t)r << 16) + (uint32_t)b);
}


uint32_t get_color(uint8_t index, uint8_t frame)
{
	if (index % 2 == 0)
	{
	 return (get_reduced_color(colors[(colors_buffer[frame][index / 2] & 0xF0) >> 4]));
	}
	else
	{
	 return (get_reduced_color(colors[colors_buffer[frame][index / 2] & 0x0F]));
	}
}

uint32_t get_color_inverted(uint8_t index, uint8_t frame)
{
	if (index % 2 == 0)
	{
	 return (get_reduced_invert_color(colors[(colors_buffer[frame][index / 2] & 0xF0) >> 4]));
	}
	else
	{
	 return (get_reduced_invert_color(colors[colors_buffer[frame][index / 2] & 0x0F]));
	}
}

void update_color(uint8_t index, uint8_t frame)
{
	tmp = index % 2 == 0 ? (colors_buffer[frame][index / 2] & 0xF0) >> 4 : (colors_buffer[frame][index / 2] & 0xF);
	tmp = tmp >= 15 ? 0 : tmp + 1;

	if (index % 2 == 0)
	{
		colors_buffer[frame][index / 2] = (tmp << 4) | ((colors_buffer[frame][index / 2] & 0x0F));
	}
	else
	{
		colors_buffer[frame][index / 2] = (tmp) | ((colors_buffer[frame][index / 2] & 0xF0));
	}
}

void  update_buffer(uint8_t frame, uint8_t current_pixel, uint8_t flag)
{
		uint32_t color;
		for(uint8_t i = 0; i < 64; i++)
		{
			if (i != current_pixel || (flag % 42))
				color = get_color(i, frame);
			else
				color = get_color_inverted(i, frame);
			buffer[i * 3] = (color & 0x00FF00) >> 8;
			buffer[i * 3 + 1] = (color & 0xFF0000) >> 16;
			buffer[i * 3 + 2] = color & 0x0000FF;
		}
		led_send_data(buffer, 64);
}


void loop() {
		static uint8_t frame = 0;
		uint8_t flag = 0;
		uint8_t current_pixel = 0;
		// frame = frame >= 4 ? 0 : frame + 1;
			while(1)
			{
				current_pixel = (x % 8) + (y % 8) * 8;
				flag++;
				if(!buttonPressed && (buttonFlag & 0x08))
				{
					buttonPressed = true;
					x++;
				}
				if(!buttonPressed && (buttonFlag & 0x04))
				{
					buttonPressed = true;
					y++;
				}
				if(!buttonPressed && (buttonFlag & 0x02))
				{
					buttonPressed = true;
					update_color(current_pixel, frame);
				}
				if(!buttonPressed && (buttonFlag & 0x01))
				{
					buttonPressed = true;
					frame = (frame + 1) % 4;
				}
				update_buffer(frame, current_pixel, flag);
				_delay_ms(21);
			}
		delay(1000);
}