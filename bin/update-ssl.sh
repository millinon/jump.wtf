#!/usr/bin/env bash

# this needs to be run as root and is probably not reliable, I just don't want to have to figure it out every time

cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/letsencrypt"

./letsencrypt-auto certonly --renew-by-default --rsa-key-size 4096 --agree-tos -d jump.wtf -d www.jump.wtf

# get certs, keys for www.jump.wtf, jump.wtf
# when more subdomains are whitelisted, static. and cdn. too?
# it would be nice to push the CDN cert to AWS, but maybe that's asking too much
