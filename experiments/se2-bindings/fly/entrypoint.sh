#!/bin/bash
# https://docs.docker.com/config/containers/multi-service_container/
  
# turn on bash's job control
set -m

# Create a swapfile
# https://community.fly.io/t/swap-memory/2749/3
echo 'Enabling swap...'
fallocate -l 512M /swapfile
chmod 0600 /swapfile 
mkswap /swapfile 
#echo 10 > /proc/sys/vm/swappiness
swapon /swapfile
sleep 5

# Start nginx, background it
echo 'Starting Nginx...'
nginx -g "daemon off;" &
sleep 2
  
# Start e2core & background it
echo 'Starting e2core...'
#e2core mod start --http-port 8080 /py.wasm &
SAT_HTTP_PORT=8000 e2core mod start /py.wasm &
sleep 2

# bring nginx back into the foreground and leave it there
echo 'Resuming nginx'
fg %1
