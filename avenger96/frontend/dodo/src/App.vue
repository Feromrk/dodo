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
          width="45"
        />

        <div class="text-h4">
          dodo
        </div>
      </div>

      <v-spacer></v-spacer>

      <v-btn
        href="https://github.com/Feromrk/dodo"
        target="_blank"
        text
      >
        <span class="mr-2">Github</span>
        <v-icon>mdi-open-in-new</v-icon>
      </v-btn>
    </v-app-bar>

    <v-content>

      <v-container>

        <v-row justify=center>
            <v-col cols="12" lg="10">
                <v-card>
                  <Now/>
                </v-card>
            </v-col>
        </v-row>


        <v-row justify=center>
            <v-col cols="12" lg="10">
                <v-card>
                  <TemperaturePlot/>
                </v-card>
            </v-col>
        </v-row>

        <v-row justify=center>
          <v-col cols="12" lg="10">
              <v-card>
                <CameraStream/>
              </v-card>
          </v-col>
        </v-row>

        <v-row justify=center>
          <v-col cols="12" lg="10">
              <v-card>
                <About/>
              </v-card>
          </v-col>
        </v-row>


      </v-container>

    </v-content>
  </v-app>
</template>

<script>
import TemperaturePlot from './components/TemperaturePlot';
import CameraStream from './components/CameraStream'
import Now from './components/Now'
import About from './components/About'

export default {
  name: 'App',

  components: {
    TemperaturePlot,
    CameraStream,
    Now,
    About
  },

  data: () => ({
    //
  }),
};
</script>
