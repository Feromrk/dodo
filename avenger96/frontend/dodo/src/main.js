import Vue from 'vue'
import App from './App.vue'
import vuetify from './plugins/vuetify';
import VueApexCharts from 'vue-apexcharts'

Vue.config.productionTip = false
Vue.prototype.$hostname = 'https://visitdodo.ddns.net:5000'

Vue.use(VueApexCharts)
Vue.component('apexchart', VueApexCharts)

new Vue({
  vuetify,
  render: h => h(App)
}).$mount('#app')
