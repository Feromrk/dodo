<template>
  <v-app>
    <v-app-bar
      app
      color="primary"
      dark
    >
      <div class="d-flex align-center">
        <v-img
          alt="Vuetify Logo"
          class="shrink mr-2"
          contain
          :src="require('./assets/logo.svg')"
          transition="scale-transition"
          width="50"
        />

        <div class="text-h3">
          dodo
        </div>
      </div>

      <v-spacer></v-spacer>

      <v-btn
        href="https://github.com/Feromrk/dodo"
        target="_blank"
        text
      >
        <span class="mr-2">Source Code</span>
        <v-icon>mdi-open-in-new</v-icon>
      </v-btn>
    </v-app-bar>

    <v-content>
      <Chart/>
    </v-content>
  </v-app>
</template>

<script>
import Chart from './components/Chart';

export default {
  name: 'App',

  components: {
    Chart,
  },

  data: () => ({
    //
  }),
};
</script>
