<template> 
    <div class="pb-5 pl-5 pr-5">
        <center>
            <h1 class="pt-2">About</h1>
            
            This project is about monitoring the temperatures of a hen-coop and provoding remote camera support. Since everything is solar powered, a low power microcontroller (ESP8266) was used as the main controller.

            <h2 class="mt-3">Hen-coop</h2>
            The system is powered by a 5 watt solar panel:
            <v-img alt='MCU' src='@/assets/solar.jpg' max-width="100%" class="mt-1 mb-5"></v-img>

            Taking a look inside, there are some more components involved:
            <v-img alt='MCU' src='@/assets/mcu.jpg' max-width="100%" class="mt-1 mb-5"></v-img>
            <ul style="width:100%; display: inline-block; text-align:left;" class="mt-2">
                <li>Battery: The 12V from the solar panel are regulated to 5V and are used to charge a lipo battery (10000 mAh) extracted from a standard power bank.</li>
                <li>ESP8266: The heart of the system. Collects sensor data, can turn on/off the raspberry pi and communicates with the backend. To safe power it is in a deep sleep state most of the time and turns on every 3 minutes to collect fresh sensor data and dispatch tasks received from the backend.</li>
                <li>Raspberry Pi: Only used for prodiving a camera stream to the backend when turned on.</li>
                <li>Night vision camera connected to the raspberry pi.</li>
            </ul>

            <h2 class="mt-3">Frontend</h2>
            <v-img alt='MCU' src='@/assets/vvf.png' max-width="60%" class="mt-1 mb-5"></v-img>
            The frontend is written in Vue.js with Vuetify and hosted on Firebase.

            <h2 class="mt-3">Backend</h2>
            <v-img alt='MCU' src='@/assets/avenger96.jpg' max-width="80%" class="mt-1 mb-5"></v-img>
            <div style="width:100%">
            The backend runs on a debian linux powered by an Avenger96 board (dual core ARM Cortex-A7 650 MHz + Cortex-M4 209 MHz) and is written in Python with Flask. It provides a small REST-API for the frontend and also serves as a reverse proxy for the camera stream (the raspberry pi provides a http stream which is converted to https by the backend).
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