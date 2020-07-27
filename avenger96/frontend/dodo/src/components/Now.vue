<template>
    <v-container>

        <v-overlay z-index="4" color="white" absolute opacity="1" :value="!visible">
            <v-progress-circular color="primary" indeterminate></v-progress-circular>
        </v-overlay>

        <v-row justify=center>
            <div  v-if="this.minutes_ago===1">{{minutes_ago}} minute ago</div>
            <div  v-else>{{minutes_ago}} minutes ago</div>
        </v-row>

        <v-row justify="center" align="center">
            <v-col>
                <center>
                    <div id="current-value">
                        {{ temp_inside.toFixed(1) }}
                        <span>°C</span>
                    </div>
                    Inside
                </center>
            </v-col>

            <v-col>
                <center>
                     <div id="current-value">
                        {{ temp_outside.toFixed(1) }}
                        <span>°C</span>
                     </div>
                    Outside
                </center>
            </v-col>

            <v-col>
                <center>
                    <div id="current-value">
                        {{battery}}
                    </div>
                    Battery
                </center>
            </v-col>
            
        </v-row>

    </v-container>
</template>

<script>
import { EventBus } from "./eventbus.js"

export default {
    name: 'Now',

    data: function() {
        return {
            visible: false,
            minutes_ago: -1,
            temp_inside: -1,
            temp_outside: -1,
            battery: -1,
            data: {}
        }
    },

    mounted() {
        this.repeatEvery(this.add_minute, 60*1000);

        EventBus.$on("now-data", data => {

            let now = new Date();
            let timestamp = new Date(data.timestamp);
            this.minutes_ago = Math.floor((now - timestamp) / 1000 / 60);
            this.temp_inside = data.temp_in;
            this.temp_outside = data.temp_out;
            this.battery = data.battery;
            this.visible = true;
        });
    },

    methods: {
        add_minute() {
            this.minutes_ago++;
            //if(this.minutes_ago % 4 === 0) {
            if(this.minutes_ago > 3) {
                EventBus.$emit("should-update-data");
            }
        },
        repeatEvery( func, interval ) {
            let vm = this;

            function repeater() {
                vm.repeatEvery( func, interval);
                func();
            }
            var now = new Date();
            var delay = interval - now % interval;

            setTimeout(repeater, delay);
        }
    }
}
</script>

<style scoped>
#current-value {
   font-size: 35px;
   position: relative;
   font-weight: bold;
}

#current-value>span {
   font-size: 20px;
   line-height: 0px;
   vertical-align: super;
   opacity: 0.8;
   top: 21px;
   position: absolute;
   font-weight: bold;
}

</style>