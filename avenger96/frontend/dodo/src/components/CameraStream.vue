<template>
    <v-container>

        <v-row justify=center>
            <h1 class="mb-5">Webcam</h1>
        </v-row>

        <v-row justify=center>
            <v-btn large color="primary" v-if="!show_progress_circle && !show_stream" @click="start_rpi()">Start Camera</v-btn>

            <h3 v-if="show_progress_info_text" class="mr-5">Camera is booting up, please wait ... </h3>
            <v-progress-circular color="primary" indeterminate v-if="!show_stream && show_progress_circle"></v-progress-circular>
        </v-row>

        <v-row justify=center>
            <img src="http://192.168.2.117:5000/camera-stream" alt="Camera stream not available :(" v-if="show_stream" width="90%">
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
            
        }
    },
    

    methods: { 
        async start_rpi() {
            this.show_progress_circle = true;
            this.check_stream();
            await axios({
                method: 'post',
                url: 'http://192.168.2.117:5000/sensor-task',
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
            fetch('http://192.168.2.117:5000/camera-stream')
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