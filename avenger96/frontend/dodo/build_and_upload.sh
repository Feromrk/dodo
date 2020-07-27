#!/bin/bash

SSH_LOGIN="root@192.168.2.117"

if [[ $(npm run build) ]]; then
    echo "deleting remote files"
    ssh "$SSH_LOGIN" "rm -r /var/www/html/*"

    echo "uploading ./dist"
    scp -r dist/* "$SSH_LOGIN:/var/www/html/"
fi