#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <board.h>
#include <time.h>

#define LIGHT_ON_TIME_MS 9000000 // 2.5 hours
#define LIGHT_SWITCH_PIN 12 // P1_2

uint32 start_time = 0;

void light_on()
{
    LED_YELLOW(1);
    setDigitalOutput(LIGHT_SWITCH_PIN, HIGH);	
}

void light_off()
{
    LED_YELLOW(0);
    setDigitalOutput(LIGHT_SWITCH_PIN, LOW);	
}

void sleep_forever()
{
    while(1)
    {
        SLEEP = 0x07; // PM3, disable USB, power down other oscillators
        __asm nop __endasm;
        __asm nop __endasm;
        __asm nop __endasm;
        if (SLEEP & 3)
        {
            PCON |= 1;    // PCON.IDLE = 1 : Actually go to sleep.
        }
        
        // normally never reaches this point
        // retry if sleep failed
        delayMs(1000);
    }
}

void main_loop()
{
    if (getMs() - start_time >= LIGHT_ON_TIME_MS)
    {
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
        main_loop();
        usbShowStatusWithGreenLed();
        usbComService();
    }
}
