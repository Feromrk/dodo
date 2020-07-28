#!/bin/bash

METADATA='
<title>Project dodo - Monitoring website for a smart hen house</title>
<meta name="description" content="Project dodo is about monitoring a smart hen-coop. The coop is equipped with a microcontroller that collects sensor data and accepts tasks. Everything is powered by solar.">
<meta property="og:title" content="Project dodo - Monitoring website for a smart hen house">
<meta property="og:site_name" content="dodo">
<meta property="og:type" content="website">
<meta property="og:image" itemprop="image" content="https://visitdodo.web.app/img/logo.f83ac6ea.svg">
<meta property="og:image:secure" itemprop="image" content="https://visitdodo.web.app/img/logo.f83ac6ea.svg">
<meta property="og:image:secure_url" itemprop="image" content="https://visitdodo.web.app/img/logo.f83ac6ea.svg">
<meta property="og:image:type" content="image/svg">
<meta property="og:image:width" content="150">
<meta property="og:image:height" content="150">
<meta property="og:url" content="https://visitdodo.web.app">
<meta name="robots" content="index,follow">
'

#remove all new lines
METADATA=$(echo $METADATA | sed -e ':a;N;$!ba;s/\n//g')

#remove all <meta> tags
#sed -i 's/<meta[^>]*>//g' dist/index.html

#add custom meta tags
sed -i "s|<head>|<head>$METADATA|" dist/index.html