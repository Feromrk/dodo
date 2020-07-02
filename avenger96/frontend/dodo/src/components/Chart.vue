<template>
    <v-container>

        <v-row justify=center>

            <v-col cols="11">
                <div id="wrapper">
                    <div id="chart-line2">
                        <apexchart type="line" height="230" :options="chartOptions" :series="series"></apexchart>
                    </div>
                <div id="chart-line">
                        <apexchart type="area" height="130" :options="chartOptionsLine" :series="seriesLine"></apexchart>
                    </div>
                </div>
            </v-col>

        </v-row>

    </v-container>
</template>

<script>
  import axios from 'axios'

  export default {
    name: 'Chart',

    async mounted() {
        await axios
            .get('http://192.168.2.117:5000/sensor-data')
            .then(response => {
                let arr = []
                for (let [timestamp, data] of Object.entries(response.data)) {
                    arr.push([parseInt(timestamp), parseFloat(data[0])])
                }                
                this.series = [{
                    data: arr
                }]
                this.seriesLine = [{
                    data: arr
                }]
                console.log(this.$vuetify.theme.themes.light.primary)
            })
            .catch(error => {
                console.log(error)
            })
    },

    data: function() {
        return {
            series: [{
                data: []
            }],
            chartOptions: {
                chart: {
                id: 'chart2',
                type: 'line',
                height: 430,
                toolbar: {
                    autoSelected: 'pan',
                    show: false
                }
                },
                colors: [this.$vuetify.theme.themes.light.secondary],
                stroke: {
                width: 3
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
                type: 'datetime'
                }
            },
            seriesLine: [{
                data: []
            }],
            chartOptionsLine: {
                chart: {
                id: 'chart1',
                height: 130,
                type: 'area',
                brush:{
                    target: 'chart2',
                    enabled: true
                },
                selection: {
                    enabled: true,
                    xaxis: {
                    min: Math.floor(new Date(new Date().setDate(new Date().getDate()-1)).getTime()/1000),
                    max: Math.floor(Date.now() / 1000) - 3000
                    }
                },
                },
                colors: [this.$vuetify.theme.themes.light.secondary],
                fill: {
                type: 'gradient',
                gradient: {
                    opacityFrom: 0.91,
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
                tickAmount: 2
                }
            },
        }
    },
  }
</script>