# dodo

This project is about monitoring my chicken house (one chicken is named dodo =)). I had a raspberry pi 1 and an avenger96 board lying around, so I came up with this idea.

## Roadmap
- ESP8266 NodeMCU sensor station located in the chicken house
  - equipped with 2 temperature sensors (monitoring inside and outside temps)
  - MOSFET for connecting/disconnecting the power supply of the raspberry pi
- Raspberry pi 1 located in the chicken house
  - poweroff via GPIO
  - motioneyeOS with night vision camera for monitoring the inside
- Avenger96 board located somewhere inside my flat
  - REST API backend for collecting/managing ESP8266 data
  - VueJS frontend 
    - displaying sensor data
    - displaying battery charge
    - booting up / turning off the raspberry pi camera (uses a lot of battery charge)
    - displaying a live stream of the raspberry pi camera
 - maybe solar powered
