#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#define BTN 3
#define timer_init() (TIMSK |= (1 << OCIE0A))
#define BTN_HOLD_MS 1000    // Press button for 1 second

//LED on PB3 (pin2)
//BUT on PB4 (pin3)


enum Device_Status
{
    POWER_OFF,
    RUNNING
};
enum Btn_Status
{
    BTN_UP,
    BTN_DOWN,
    BTN_IGNORE
};
void setup()
{
    sei();                  // Enable interrupts
    PORTB |= (1 << BTN);    // Enable PULL_UP resistor
    GIMSK |= (1 << PCIE);   // Enable Pin Change Interrupts
    PCMSK |= (1 << BTN);    // Use PCINTn as interrupt pin (Button I/O pin)
    TCCR0A |= (1 << WGM01); // Set CTC mode on Timer 1
    TIMSK |= (1 << OCIE0A); // Enable the Timer/Counter0 Compare Match A interrupt
    TCCR0B |= (1 << CS01);  // Set prescaler to 8
    OCR0A = 125;            // Set the output compare reg so tops at 1 ms
}
void power_off()
{
    cli();                               // Disable interrupts before next commands
    wdt_disable();                       // Disable watch dog timer to save power
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set sleep mode power down
    sleep_enable();
    sleep_bod_disable(); // Disable brown-out detector
    sei();               // Enable interrupts
    sleep_cpu();
    sleep_disable();
}
volatile unsigned int timer;  // milliseconds counter 
Btn_Status btn_status;        // Status of the button
int main()
{
    setup();
    Device_Status status = RUNNING; // Set start ON or OFF when power is connected
    btn_status = BTN_UP;
    DDRB |= (1 << DDB4); // Set pin 0 as output
    for (;;)
    {
        if (btn_status == BTN_DOWN)
        {
            if (timer > BTN_HOLD_MS) // Check if button has been pressed enough
            {
                if (status == RUNNING)
                    status = POWER_OFF;
                else
                {
                    status = RUNNING;
                    // setup of the device here if needed;
                }
                btn_status = BTN_IGNORE; // If status already changed don't swap it again
            }
        }
        else
        {
            if (status) // Is status RUNNING?
            {
                /* main code here */
                PORTB |= (1 << PB4); // Pin 0 ON
                /* -------------- */
            }
            else
            {
                PORTB &= ~(1 << PB4); // Pin 0 OFF
                power_off();
            }
        }
    }
}

ISR(PCINT0_vect)
{
    if (!((PINB >> BTN) & 0x01)) // Check if button is down
    {
        btn_status = BTN_DOWN;
        timer_init();
        timer = 0;
    }
    else
        btn_status = BTN_UP;
}
ISR(TIM0_COMPA_vect)
{
    timer++;
}