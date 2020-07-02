import Vue from 'vue';
import Vuetify from 'vuetify/lib';

Vue.use(Vuetify);

export default new Vuetify({
    theme: {
        themes: {
          light: {
            primary: '#EF5350',
            secondary: '#B71C1C',
            accent: '#FBC02D',
          },
        },
    },
});