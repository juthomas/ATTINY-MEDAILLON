#include <Arduino.h>
#include "medaillon.h"

//https://www.electronics-lab.com/project/attiny85-push-button-power-switching-software-solution/

uint8_t buffer[64 * 3];
uint8_t colors_buffer[4][32];
uint8_t tmp;
uint8_t current_frame = 0;
int delayval = 100;

void setup() {
	DDRB = 1 << PIN4;
	for (uint8_t u = 0; u < 4; u++)
	{
		for (uint8_t i = 0; i < 32; i++)
		{
			colors_buffer[u][i] =  (u << 4) + u;
		}
	}
	// DDRB &= !(0b00000100);
	// // PCICR |= 0b00000101;
	// PCMSK |= (1 << PCINT3);


}


// ISR(PCINT0_vect)
// {
// 	current_frame = current_frame == 1 ? 0 : 1;
// }

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
	uint8_t	g = ((color & 0x00FF00) >> 8) * 0.1;
	uint8_t	r = ((color & 0xFF0000) >> 16) * 0.1;
	uint8_t	b = (color & 0x0000FF) * 0.1;
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

void update_color(uint8_t index, uint8_t frame)
{
	tmp = get_color(index, frame);
	tmp = tmp >= 15 ? 0 : tmp + 1;

	if (index % 2 == 0)
	{
		colors_buffer[frame][index / 2] = (tmp << 4) & (colors_buffer[frame][index / 2] & 0x0F);
	}
	else
	{
		colors_buffer[frame][index / 2] = (tmp) & (colors_buffer[frame][index / 2] & 0xF0);
	}
}


void loop() {
		static uint8_t frame = 0;

		// for (uint8_t i = 0; i < 32; i++)
		// {
		// 	colors_buffer[i] = (current_color << 4) + current_color;
		// }
		frame = frame >= 4 ? 0 : frame + 1;
		for (uint8_t i = 0; i < 64; i++)
		{
				// index = i;
				uint32_t color = get_color(i, frame);
				buffer[i * 3] = (color & 0x00FF00) >> 8;
				buffer[i * 3 + 1] = (color & 0xFF0000) >> 16;
				buffer[i * 3 + 2] = color & 0x0000FF;
		}
		// for (uint8_t i = 0; i < (64 * 3); i++)
		// {
		// 	buffer[i] = 10;
		// }


		led_send_data(buffer, 64);
		delay(1000);
}