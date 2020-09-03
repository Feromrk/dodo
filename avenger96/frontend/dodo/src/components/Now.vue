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
                    <div class="tempValue">
                        {{ temp_inside.toFixed(1) }}
                        <span>°C</span>
                    </div>
                    Inside
                </center>
            </v-col>

            <v-col>
                <center>
                     <div class="tempValue">
                        {{ temp_outside.toFixed(1) }}
                        <span>°C</span>
                     </div>
                    Outside
                </center>
            </v-col>

            <v-col>
                <center>
                    <div class="battery">
                        <div class="batteryLevel" :class="{warn: battery_medium, alert: battery_low}" :style="{'height': battery + '%'}"></div>
                        <div class="batteryValue">{{battery}}<span>%</span></div>
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
            battery_medium: false,
            battery_low: false,
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

            if(this.battery < 60 && this.battery >= 25) {
                this.battery_medium = true
                this.battery_low = false
            } else if(this.battery < 25) {
                this.battery_medium = false
                this.battery_low = true
            } else {
                this.battery_medium = false
                this.battery_low = false
            }
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

<style scoped lang="scss">
.tempValue {
   font-size: 35px;
   position: relative;
   font-weight: bold;
}

.tempValue>span {
   font-size: 20px;
   line-height: 0px;
   vertical-align: super;
   opacity: 0.8;
   top: 21px;
   position: absolute;
   font-weight: bold;
}

.battery {
  border: 3px solid #333;
  width: 18px;
  height: 28px;
  padding: 2px;
  border-radius: 4px;
  position: relative;
  margin: 15px 0;
  margin-right: 50px;
  
  &:before {
    content: '';
    height: 3px;
    width: 9px;
    background: #333;
    display: block;
    position: absolute;
    top: -6px;
    border-radius: 4px 4px 0 0;
  }

  &:after {
    content: '';
    display: block;
    position: absolute;
    top: -1px;
    left: -1px;
    right: -1px;
    bottom: -1px;
    border: 1px solid #fff;
    border-radius: 2px;
  }
}

.batteryValue {
    position: absolute;
    left: 21px;
    top: -11px;
    font-weight: bold;
    font-size: 30px;
}

.batteryValue>span {
   font-size: 20px;
   line-height: 0px;
   vertical-align: super;
   opacity: 0.8;
   top: 18px;
   position: absolute;
   font-weight: bold;
}

.batteryLevel {
  background: #30b455;  
  position: absolute;
  bottom: 0px;
  left: 0;
  right: 0;
  
  &.warn {
    background-color: #EFAF13;
  }
    
  &.alert {
    background-color: #e81309;
  }  
}

</style>