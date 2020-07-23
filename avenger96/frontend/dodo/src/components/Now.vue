<template>
    <v-container>

        <v-row justify=center>
            <div  v-if="this.minutes_ago===1">{{minutes_ago}} minute ago</div>
            <div  v-else>{{minutes_ago}} minutes ago</div>
        </v-row>

        <v-row justify="center" align="center">
            <v-col>
                <center>
                    <div id="current-value">
                        {{ temp_inside }}
                        <span>°C</span>
                    </div>
                    Inside
                </center>
            </v-col>

            <v-col>
                <center>
                     <div id="current-value">
                        {{temp_outside}}
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
            minutes_ago: -1,
            temp_inside: -1,
            temp_outside: -1,
            battery: -1,
        }
    },

    mounted() {
        EventBus.$on("now-data", data => {
            console.log("hello from now.vue", data);
            //console.log

            let now = new Date();
            let timestamp = new Date(data.timestamp);
            this.minutes_ago = Math.round((now - timestamp) / 1000 / 60);

            this.temp_inside = data.temp_in;
            this.temp_outside = data.temp_out;
            this.battery = data.battery;
        });
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