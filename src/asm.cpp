#include "medaillon.h"
void led_send_data(uint8_t *pixels, uint16_t pixels_number)
{
	uint8_t mask = 1 << PIN4;
	volatile uint16_t
	i   = pixels_number * 3; // Loop counter
	volatile uint8_t
	*ptr = pixels,   // Pointer to next byte
	b   = *ptr++,   // Current byte value
	hi,             // PORT w/output bit set high
	lo;             // PORT w/output bit set low
	volatile uint8_t next, bit;

	volatile uint8_t *port = &PORTB;

	hi   = *port |  mask;
	lo   = *port & ~mask;
	next = lo;
	bit  = 8;
	cli();
	asm volatile(
	 "head20:"                   "\n\t" // Clk  Pseudocode    (T =  0)
	  "st   %a[port],  %[hi]"    "\n\t" // 2    PORT = hi     (T =  2)
	  "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
	   "mov  %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T =  4)
	  "dec  %[bit]"              "\n\t" // 1    bit--         (T =  5)
	  "st   %a[port],  %[next]"  "\n\t" // 2    PORT = next   (T =  7)
	  "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T =  8)
	  "breq nextbyte20"          "\n\t" // 1-2  if(bit == 0) (from dec above)
	  "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 10)
	  "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 12)
	  "nop"                      "\n\t" // 1    nop           (T = 13)
	  "st   %a[port],  %[lo]"    "\n\t" // 2    PORT = lo     (T = 15)
	  "nop"                      "\n\t" // 1    nop           (T = 16)
	  "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 18)
	  "rjmp head20"              "\n\t" // 2    -> head20 (next bit out)
	 "nextbyte20:"               "\n\t" //                    (T = 10)
	  "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 11)
	  "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 13)
	  "st   %a[port], %[lo]"     "\n\t" // 2    PORT = lo     (T = 15)
	  "nop"                      "\n\t" // 1    nop           (T = 16)
	  "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 18)
	   "brne head20"             "\n"   // 2    if(i != 0) -> (next byte)
	  : [port]  "+e" (port),
		[byte]  "+r" (b),
		[bit]   "+r" (bit),
		[next]  "+r" (next),
		[count] "+w" (i)
	  : [ptr]    "e" (ptr),
		[hi]     "r" (hi),
		[lo]     "r" (lo));
		sei();
}