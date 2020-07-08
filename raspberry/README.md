# Raspberry Pi Config

## Basic Setup
### System
- Model: Raspberry Pi Model B Rev 2
- Camera: MakerHawk 5MP OV5647 with night vision
- Wifi Dongle: EDIMAX EW-7811UN
- OS: Raspberry Pi OS (32-bit) Lite (Release date: 2020-05-27)

### Wifi
create `/boot/wpa_supplicant.conf` with following content:

```
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=DE

network={
    ssid="«your_SSID»"
    psk="«your_PSK»"
    key_mgmt=WPA-PSK
}
```
### SSH Server
create empty file `/boot/ssh`

### Device Tree Overlays
add following lines to `/boot/config.txt`

```
dtoverlay=gpio-shutdown,gpio_pin=4
dtoverlay=gpio-poweroff,gpiopin=17,active_low=1
```

This will allow to shutdown the raspberry when GND and GPIO4 are shorted. When the pi is in a halted state GPIO17 will go from HIGH to LOW. Now it is safe to cut off the power supply.


## MotionEye
Instructions can be found also here: https://github.com/ccrisan/motioneye/wiki/Install-On-Raspbian

#### Motion (Monitors Video Signals From Cameras)
- `sudo apt-get install ffmpeg libmariadb3 libpq5 libmicrohttpd12`
- `cd /tmp && wget https://github.com/Motion-Project/motion/releases/download/release-4.3.1/pi_buster_motion_4.3.1-1_armhf.deb && sudo dpkg -i pi_buster_motion_4.3.1-1_armhf.deb`

#### MotionEye (Web Frontend for Motion)
- `sudo apt-get install python-pip python-dev libssl-dev libcurl4-openssl-dev libjpeg-dev libz-dev python-pillow`
- `sudo pip install motioneye`
- create configuration directory: `sudo mkdir /etc/motioneye && sudo cp /usr/local/share/motioneye/extra/motioneye.conf.sample /etc/motioneye/motioneye.conf` 
- create media directory: `sudo mkdir /var/lib/motioneye`
- run motioneye on startup: 
    - `sudo cp /usr/local/share/motioneye/extra/motioneye.systemd-unit-local /etc/systemd/system/motioneye.service`
    - `sudo systemctl daemon-reload`
    - `sudo systemctl enable motioneye.service`
    - `sudo systemctl start motioneye.service`
- access the frontend: `http://[your_ip]:8765/` (credentials: admin with empty pw)