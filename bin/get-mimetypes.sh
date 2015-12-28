#!/bin/sh

# Apparently PHP doesn't have a built-in way to get the MIME type of an extension? This is horrible, but it will handle most extensions.

printf "<?hh\nclass mimes {static \$mime_types;static function init(): void {self::\$mime_types = [\n" > www/mimes.hh

awk '!/^#/{ for(i=2; i <= NF; i++) { print "\"" $i "\" => \"" $1 "\","; } }' /etc/mime.types >> www/mimes.hh

printf "\n];}} mimes::init();" >> www/mimes.hh
