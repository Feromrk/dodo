<template>
    <v-container>

        <v-row justify=center>
            <h1 class="mb-5">Webcam</h1>
            <v-progress-circular class="ml-5 mt-2" color="primary" indeterminate v-if="!show_stream && show_progress_circle"></v-progress-circular>
            <v-icon x-large class="ml-1" v-if="show_stream" style="margin-top: -20px" color="light-green">mdi-power-plug</v-icon>
        </v-row>

        <v-row justify=center>
            <v-btn large color="primary" v-if="!show_progress_circle && !show_stream" @click="start_rpi()">Start Camera</v-btn>

            <span v-if="show_stream" class="mb-5" style="margin-top: -25px">(will be turned off automatically)</span>

            <h3 align=center v-if="show_progress_info_text">Camera is booting up,<br>
            please wait up to 4 minutes ...</h3>
            
            <v-tooltip v-model="show_tooltip" top max-width="80%" v-if="show_progress_info_text">
                <template v-slot:activator>
                    <v-btn icon @click="show_tooltip = !show_tooltip" class="mt-2 ml-2">
                        <v-icon large>mdi-information</v-icon>
                    </v-btn>
                </template>
                <span>To reduce power consumption, the controller located at the chicken house is off most of the time. It turns on every 3 minutes to check whether it should power on the webcam.</span>
            </v-tooltip>

        </v-row>

        <v-row justify=center>
            <img :src="camera_url" alt="Camera stream not available :(" v-if="show_stream" width="90%">
        </v-row>

    </v-container>
    
</template>

<script>
  import axios from 'axios'

  export default {
    name: 'CameraStream',

    mounted() {
        //console.log("");
    },

    data: function() {
        return {
            show_stream: false,
            show_progress_circle: false,
            show_progress_info_text: false,
            show_tooltip: false,
            camera_url: process.env.VUE_APP_FULL_BACKEND_URL + '/camera-stream'
        }
    },
    

    methods: { 
        async start_rpi() {
            this.show_progress_circle = true;
            this.check_stream();
            await axios({
                method: 'post',
                url: this.$backend_hostname + '/sensor-task',
                data: {
                    id: 2
                }
            })
            .then(resp => {
                console.log(resp);
            })
            .catch(err => {
                console.log(err);
            })
        },

        check_stream() {
            console.log("checking stream availability");
            fetch(this.camera_url)
            .then((resp) => {
                if(resp.ok) {
                    console.log("stream is there!");
                    this.show_progress_circle = false;
                    this.show_stream = true;
                    this.show_progress_info_text = false;
                } else {
                    console.log("camera stream not reachable, retrying");
                    
                    if(this.show_stream) {
                        this.show_stream = false;
                        this.show_progress_circle = false;
                        this.show_progress_info_text = false;
                        return;
                    } else {
                        this.show_progress_info_text = true;
                    }
                }

                setTimeout(this.check_stream, 15000);
            })
        }
    }

  }
</script>