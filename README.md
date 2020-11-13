# dodo

This project is about monitoring my chicken house. I had a raspberry pi 1 and an avenger96 board lying around, so I came up with this idea.

## Overview
- ESP8266 NodeMCU sensor station located in the chicken house (solar powered)
  - equipped with 2 temperature sensors (monitoring inside and outside temps)
  - MOSFET for connecting/disconnecting the power supply of the raspberry pi
- Raspberry pi 1 located in the chicken house (solar powered)
  - poweroff via GPIO
  - motion (open source tool) with night vision camera for monitoring the chickens inside at night

- Avenger96 board located somewhere inside my flat
  - REST API backend for collecting/managing ESP8266 data
  - reverse proxy for converting http camera stream into https

- VueJS frontend 
    - displaying sensor data
    - displaying battery charge
    - booting up / turning off the raspberry pi camera (uses a lot of battery charge)
    - displaying a live stream of the raspberry pi camera
    - About page
    
## Additional (small) Project: Solar Light
A simple solar powered light bulb turns on when it is dark outside. I had the problem that it lasts too long, so that the chickens couldn't sleep.
That's why I decided to use a Wixel microcontroller that switches off the light after a few hours. 
Pretty straightforward setup:
- the solar light collector (which charges an integrated power bank during the day) turns on its voltage supply at night, which turns on the Wixel
- the Wixel pulls a GPIO pin to HIGH, which turns on the light switch
- the light switch is a simple mosfet transistor
- after a set time period (2.5 hours) the Wixel goes into a deep sleep state -> pull down resistor is connected to the GPIO pin, which turns off the light switch
- the next day - when the sun rises again - the solar panel turns off the voltage supply -> wixel goes from deep sleep to off -> light switch stays off
- the cycle repeats again