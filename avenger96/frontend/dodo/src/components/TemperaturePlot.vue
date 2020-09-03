<template> 
    <div id="wrapper">
        <v-overlay z-index="4" color="white" absolute opacity="1" :value="!visible">
            <v-progress-circular color="primary" indeterminate></v-progress-circular>
        </v-overlay>

        <center><h1 class="pt-2">History</h1></center>

        <div id="temperature_plot-main">
            <apexchart type="line" height="230" :options="chartOptions" :series="series"></apexchart>
        </div>
        <div id="temperature_plot-footer">
            <apexchart type="area" height="130" :options="chartOptionsLine" :series="seriesLine"></apexchart>
        </div>
    </div>
</template>

<script>
  import axios from 'axios'
  import { EventBus } from "./eventbus.js";

  export default {
    name: 'TemperaturePlot',

    mounted() {
        this.get_data();
        EventBus.$on("should-update-data", this.get_data);
    },

    updated() {
        this.visible = true;
    },

    methods: {
        async get_data() {
            await axios
            .get(process.env.VUE_APP_FULL_BACKEND_URL + '/sensor-data')
            .then(response => {
                let temps_in = []
                let temps_out = []
                let rpi_states = []

                for (let [timestamp, data] of Object.entries(response.data)) {
                    let ts = parseInt(timestamp+"000")
                    temps_in.push([ts, parseFloat(data[0])])
                    temps_out.push([ts, parseFloat(data[1])])
                    rpi_states.push([ts, parseInt(data[2])])
                }

                //data for Now.vue
                let latest_timestamp = temps_in[temps_in.length-1][0]
                
                let latest_timestamp_rpi_off = latest_timestamp, i=1;
                while(parseInt(response.data[String(latest_timestamp_rpi_off).slice(0, -3)][2])) {
                    latest_timestamp_rpi_off = temps_in[temps_in.length-++i][0]
                }

                EventBus.$emit("now-data", {
                    temp_in : temps_in[temps_in.length-1][1],
                    temp_out : temps_out[temps_in.length-1][1],
                    battery : parseInt(response.data[String(latest_timestamp_rpi_off).slice(0, -3)][3]),
                    timestamp : latest_timestamp,
                });

                this.series = [
                    {
                        name: "Inside",
                        data: temps_in
                    },
                    {
                        name: "Outside",
                        data: temps_out
                    },
                    {
                        name: "Camera",
                        data: rpi_states,
                        type: 'area',
                    }
                ]
                this.seriesLine = [this.series[0], this.series[1]]
                //console.log('temps inside', temps_in)
                //console.log('temps outside', temps_out)
                //console.log('rpi states', rpi_states)

                this.chartOptionsLine = {
                    ...this.chartOptionsLine,
                    chart: {
                        ...this.chartOptionsLine.chart,
                        selection: {
                            ...this.chartOptionsLine.chart.selection,
                            xaxis: {
                                min: temps_in[temps_in.length-60][0],
                                max: temps_in[temps_in.length-1][0]
                            }
                        }
                    }
                };

            })
            .catch(error => {
                console.log(error)
            })
        }
    },

    data: function() {
        return {
            visible: false,
            series: [{
                data: []
            }],
            chartOptions: {
                // title: {
                //     text: 'Temperaturen im Hühnerhaus',
                //     align: 'center',
                //     floating: true,
                //     style: {
                //         fontSize:  '18px',
                //     },
                // },
                // theme: {
                //     palette: 'palette7'
                // },
                colors: ['#D7263D', '#1B998B', this.$vuetify.theme.themes.light.accent],
                tooltip: {
                    x: {
                        show: false,
                        format: 'HH:mm'
                    },
                    y: {
                        formatter: function(val, { _, seriesIndex}) { // eslint-disable-line no-unused-vars
                            if(seriesIndex === 2) {
                                return val === 0 ? 'off' : 'on'
                            } else {
                                return String(val.toFixed(1)) + ' °C'
                            }
                        }
                    }
                },
                chart: {
                    id: 'main',
                    animations: {
                        enabled: false
                    },
                    type: 'line',
                    height: 550,
                    toolbar: {
                        autoSelected: 'pan',
                        show: false,
                        tools: {
                            download: false,
                            selection: true,
                            zoom: true,
                            zoomin: true,
                            zoomout: true,
                            pan: true,
                            reset: true ,
                            customIcons: []
                        },
                    },
                },
                //colors: [this.$vuetify.theme.themes.light.primary, this.$vuetify.theme.themes.light.secondary, this.$vuetify.theme.themes.light.accent],
                stroke: {
                    width: [3, 3, 1],
                    curve: ['straight', 'straight', 'stepline'],
                    dashArray: [0, 0, 2]
                },
                dataLabels: {
                    enabled: false
                },
                fill: {
                    type: 'solid',
                    opacity: [1, 1, 0.1]
                },
                markers: {
                    size: 0
                },
                xaxis: {
                    type: 'datetime',
                    labels: {
                        datetimeUTC: false
                    }
                },
                yaxis: [
                    {
                        seriesName: 'Inside',
                        title: {
                            text: '°C',
                            style: {
                                fontSize: '16px'
                            }
                        },
                        labels: {
                            formatter: function(val) {
                                return Math.round(val);
                            }
                        }
                    },
                    {
                        seriesName: 'Outside',
                        show: false
                    },
                    {
                        opposite: true,
                        seriesName: 'Camera',
                        // title: {
                        //     text: 'Camera On/Off',
                        //     style: {
                        //         fontSize: '16px',
                        //         color: '#DE7D00'
                        //     }
                        // },
                        tickAmount: 1,
                        labels: {
                            formatter: function(val) {
                                return val < 1 ? 'off' : 'on'
                            },
                            style: {
                                colors: this.$vuetify.theme.themes.light.accent,
                                fontSize: '14px',
                                fontWeight: 500,
                            },
                            
                        }
                    }
                ],
                legend: {
                    fontSize: '15px',
                    position: 'bottom'
                },
                // annotations: {
                //     yaxis: [
                //         {
                //             y: 30,
                //             borderColor: this.$vuetify.theme.themes.light.accent,
                //             strokeDashArray: 4,
                //             label: {
                //                 borderColor: this.$vuetify.theme.themes.light.accent,
                //                 style: {
                //                     color: '#fff',
                //                     background: this.$vuetify.theme.themes.light.accent,
                //                     fontSize: '17px'
                //                 },
                //                 text: '🥵 30°C'
                //             }
                //         },
                //         {
                //             y: -15,
                //             borderColor: this.$vuetify.theme.themes.light.accent,
                //             label: {
                //                 borderColor: this.$vuetify.theme.themes.light.accent,
                //                 style: {
                //                     color: '#fff',
                //                     background: this.$vuetify.theme.themes.light.accent
                //                 },
                //                 text: '🥶'
                //             }
                //         },
                //     ]
                // }
            },
            seriesLine: [{
                data: []
            }],
            chartOptionsLine: {
                theme: {
                    palette: 'palette7'
                },
                chart: {
                    id: 'footer',
                    height: 130,
                    type: 'area',
                    brush:{
                        target: 'main',
                        enabled: true
                    },
                    selection: {
                        enabled: true,
                        // xaxis: {
                        //     min: Math.floor(new Date(new Date().setDate(new Date().getDate()-1)).getTime()),
                        //     max: Math.floor(Date.now()) - 3000
                        // }
                    },
                },
                //colors: [this.$vuetify.theme.themes.light.primary, this.$vuetify.theme.themes.light.secondary, this.$vuetify.theme.themes.light.accent],
                fill: {
                    type: 'gradient',
                    gradient: {
                        opacityFrom: 0.8,
                        opacityTo: 0.1,
                    }
                },
                xaxis: {
                    type: 'datetime',
                    tooltip: {
                        enabled: false
                    }
                },
                yaxis: {
                    tickAmount: 2,
                    title: {
                        text: '°C',
                        style: {
                            fontSize: '16px'
                        }
                    },
                    labels: {
                        formatter: function(val) {
                            return Math.round(val);
                        }
                    }
                },
                legend: {
                    show: false,
                }
            },
        }
    },
  }
</script>