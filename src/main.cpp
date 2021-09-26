#include <Arduino.h>
#include "medaillon.h"

//https://www.electronics-lab.com/project/attiny85-push-button-power-switching-software-solution/

uint8_t buffer[64 * 3];
uint8_t colors_buffer[32];
uint8_t index = 0;
uint8_t tmp;
uint8_t current_frame = 0;
int delayval = 100;

void setup() {
  DDRB = 1 << PIN4;
  for (uint8_t i = 0; i < 64; i++)
  {
    colors_buffer[i] = 0;
  }
	DDRB &= !(0b00000100);
	// PCICR |= 0b00000101;
	PCMSK |= (1 << PCINT3);


}


ISR(PCINT0_vect)
{
  current_frame = current_frame == 1 ? 0 : 1;
}

uint8_t colors[16] = {
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

uint8_t get_color()
{
  if (index % 2 == 0)
  {
   return ((colors_buffer[index / 2] & 0xF0) >> 4);
  }
  else
  {
   return (colors_buffer[index / 2] & 0x0F);
  }
}

void update_color()
{
  tmp = get_color();
  tmp = tmp >= 15 ? 0 : tmp + 1;

  if (index % 2 == 0)
  {
    colors_buffer[index / 2] = (tmp << 4) & (colors_buffer[index / 2] & 0x0F);
  }
  else
  {
    colors_buffer[index / 2] = (tmp) & (colors_buffer[index / 2] & 0xF0);
  }
}


void loop() {

  {
    for (uint8_t i = 0; i < 64; i++)
    {
        index = i;
        uint8_t color = get_color();
        buffer[i * 3] = (color & 0x00FF00) >> 8;
        buffer[i * 3 + 1] = (color & 0xFF0000) >> 16;
        buffer[i * 3 + 2] = color & 0x0000FF;
    }
  for (uint8_t i = 0; i < 64; i++)
  {

  }



    led_send_data(buffer, 64);
    delay(10);
  }
}