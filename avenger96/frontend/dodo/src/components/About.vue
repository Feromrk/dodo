<template> 
    <div class="pb-5 pl-5 pr-5">
        <center>
            <h1 class="pt-2">About</h1>
            
            This project is about monitoring the temperatures of a hen-coop and provoding remote camera support. Since everything is solar powered, a low power microcontroller was used.

            <h2 class="mt-3">Hen-coop</h2>
            The system is powered by a 5 watt solar panel:
            <v-img alt='MCU' src='@/assets/solar.jpg' max-width="100%" class="mt-1 mb-5"></v-img>

            Taking a look inside, there are some more components involved:
            <v-img alt='MCU' src='@/assets/mcu.jpg' max-width="100%" class="mt-1 mb-5"></v-img>
            <ul style="width:100%; display: inline-block; text-align:left;" class="mt-2">
                <li>Battery: The 12 volts from the solar panel are regulated to 5 volts and are used to charge a lipo battery (10000 mAh), which was extracted from a standard power bank. A Lipo cell operates at around 3.0V - 4.2V, so there is a step up converter needed on the other side for prodiving a stable 5V power source (for the ESP and Raspberry). This may sound not very efficient (which is probably right), but it is sufficient for this use case here.</li>
                <li>ESP8266 NodeMCU: The low power microcontroller. Collects sensor data, can turn on/off the raspberry pi with the help of a MOSFET and communicates with the backend. To safe power it is in a deep sleep state most of the time and turns on every 3 minutes to collect fresh sensor data and dispatch tasks received from the backend.</li>
                <li>16 bit ADC: The ESP has a builtin 10 bit ADC, which is used to measure the voltage of the lipo battery (estimation of the state of charge). An additional NTC temperature sensor is connected to the 16bit ADC for measuring the temperature inside the hen-coop. The outside temperature is measured with a digital DHT22 sensor.</li>
                <li>Raspberry Pi: Only used for prodiving a camera stream to the backend when turned on. Has a minimal Raspbian installation and uses Motion to deliver an http camera stream.</li>
                <li>Night vision camera: Mostly used for checking whether all the chickens are inside and safe at night (we have some martens running around).</li>
            </ul>

            <h2 class="mt-3">Frontend</h2>
            <v-img alt='MCU' src='@/assets/vvf.png' max-width="60%" class="mt-1 mb-5"></v-img>
            The frontend is written in Vue.js with Vuetify and hosted on Firebase.

            <h2 class="mt-3">Backend</h2>
            <v-img alt='MCU' src='@/assets/avenger96.jpg' max-width="80%" class="mt-1 mb-5"></v-img>
            <div style="width:100%">
            The backend runs on a debian linux powered by an Avenger96 board (dual core ARM Cortex-A7 650 MHz + Cortex-M4 209 MHz) and is written in Python with Flask. It provides a small REST-API for the frontend and also serves as a reverse proxy for the camera stream (the raspberry pi provides an http stream which is converted to https by the backend).
            </div>


            
        </center>
    </div>
</template>

<script>

export default {
    name: 'About',

    data: function() {
        return {
            
        }
    },
}

</script>

<style scoped>


</style>