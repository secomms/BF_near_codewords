#!/bin/bash

# Avvia il notebook SageMath in background e filtra solo la riga con il link
sage -n jupyter --ip=0.0.0.0 --no-browser 2>&1 \
  | grep --line-buffered "http://" &
  
# Attende che compaia il link
while true; do
    URL=$(grep -m1 -o "http://[^ ]*")
    if [ ! -z "$URL" ]; then
        echo "🔗 Collegati qui: $URL"
        break
    fi
    sleep 1
done

# Mantiene vivo il container
tail -f /dev/null

