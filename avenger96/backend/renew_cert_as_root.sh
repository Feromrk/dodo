#!/bin/bash

/usr/sbin/apache2ctl graceful-stop
certbot renew
/usr/sbin/apache2ctl start