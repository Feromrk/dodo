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