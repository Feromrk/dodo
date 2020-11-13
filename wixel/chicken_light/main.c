#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <board.h>
#include <time.h>
//#include <stdio.h>

#define LIGHT_ON_TIME_MS (3600 * 1000 * 2.5) // 2.5 hours
#define LIGHT_SWITCH_PIN 12 //P1_2

uint32 start_time = 0;

void light_on()
{
    setDigitalOutput(LIGHT_SWITCH_PIN, HIGH);	
}

void light_off()
{
    setDigitalOutput(LIGHT_SWITCH_PIN, LOW);	
}

void sleep_forever()
{
    SLEEP = 0x07; // PM3, disable USB, power down other oscillators
    __asm nop __endasm;
    __asm nop __endasm;
    __asm nop __endasm;
    if (SLEEP & 3)
    {
        PCON |= 1;    // PCON.IDLE = 1 : Actually go to sleep.
    }
}

void loop()
{
    LED_YELLOW(1);
    
    if (getMs() - start_time >= LIGHT_ON_TIME_MS)
    {
        LED_YELLOW(0);
        light_off();
        sleep_forever();
    }
}

void main()
{
    systemInit();
    usbInit();
    
    light_on();
    start_time = getMs();

    while(1)
    {
        boardService();
        loop();
        usbShowStatusWithGreenLed();
        usbComService();
    }
}
