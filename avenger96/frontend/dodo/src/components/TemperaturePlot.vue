<template> 
    <div id="wrapper">
        <v-overlay color="white" absolute opacity="1" :value="!visible">
            <v-progress-circular color="primary" indeterminate></v-progress-circular>
        </v-overlay>

        <center><h1>Temperatures</h1></center>

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
            .get('http://192.168.2.117:5000/sensor-data')
            .then(response => {
                let temps_in = []
                let temps_out = []

                for (let [timestamp, data] of Object.entries(response.data)) {
                    temps_in.push([parseInt(timestamp+"000"), parseFloat(data[0])])
                    temps_out.push([parseInt(timestamp+"000"), parseFloat(data[1])])
                }

                //data for Now.vue
                let latest_timestamp = temps_in[temps_in.length-1][0]
                EventBus.$emit("now-data", {
                    temp_in : temps_in[temps_in.length-1][1],
                    temp_out : temps_out[temps_in.length-1][1],
                    battery : parseInt(response.data[String(latest_timestamp).slice(0, -3)][3]),
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
                    }
                ]
                this.seriesLine = this.series
                //console.log('temps inside', temps_in)
                //console.log('temps outside', temps_out)

                this.chartOptionsLine = {
                    ...this.chartOptionsLine,
                    chart: {
                        ...this.chartOptionsLine.chart,
                        selection: {
                            ...this.chartOptionsLine.chart.selection,
                            xaxis: {
                                min: temps_in[temps_in.length-100][0],
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
                theme: {
                    palette: 'palette7'
                },
                tooltip: {
                    x: {
                        show: false
                    },
                    y: {
                        formatter: function(val) {
                            return String(val.toFixed(1)) + ' °C'
                        }
                    }
                },
                chart: {
                    id: 'main',
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
                    width: 3,
                    curve: 'straight'
                },
                dataLabels: {
                    enabled: false
                },
                fill: {
                    opacity: 1,
                },
                markers: {
                    size: 0
                },
                xaxis: {
                    type: 'datetime',
                },
                yaxis: {
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
                    fontSize: '15px',
                    position: 'bottom'
                },
                annotations: {
                    yaxis: [
                        {
                            y: 30,
                            borderColor: this.$vuetify.theme.themes.light.accent,
                            strokeDashArray: 4,
                            label: {
                                borderColor: this.$vuetify.theme.themes.light.accent,
                                style: {
                                    color: '#fff',
                                    background: this.$vuetify.theme.themes.light.accent,
                                    fontSize: '17px'
                                },
                                text: '🥵 30°C'
                            }
                        },
                        {
                            y: -15,
                            borderColor: this.$vuetify.theme.themes.light.accent,
                            label: {
                                borderColor: this.$vuetify.theme.themes.light.accent,
                                style: {
                                    color: '#fff',
                                    background: this.$vuetify.theme.themes.light.accent
                                },
                                text: '🥶'
                            }
                        },
                    ]
                }
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
                        xaxis: {
                            min: Math.floor(new Date(new Date().setDate(new Date().getDate()-1)).getTime()),
                            max: Math.floor(Date.now()) - 3000
                        }
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