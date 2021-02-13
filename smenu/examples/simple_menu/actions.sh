#!/usr/bin/env bash

(( $# != 1 )) && exit 0

ACTION=""git clone https://github.com/plasmoduck/modwm" && "cd modwm" && "doas make install""

case $ACTION in
  *) $ACTION
     sleep 1
     ;;
esac

exit 0
