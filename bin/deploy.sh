#!/usr/bin/env bash

cd `git rev-parse --show-toplevel`

./bin/minify.js ./www/htdocs/js/main.js ./www/htdocs/css/main.css ./www/htdocs/js/clip.js
./bin/get-mimetypes.sh

for file in aws.json aws_config.hh jump_config.hh; do

    if [ ! -f "./www/config/${file}" ] then
        cp "./www/config/${file}.example" "./www/config/${file}"
    fi
done
