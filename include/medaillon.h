#ifndef MEDAILLON_H
# define MEDAILLON_H
# include <stdint.h>
# include <avr/io.h>
# include <avr/interrupt.h>
void led_send_data(uint8_t *pixels, uint16_t pixels_number);

#endif