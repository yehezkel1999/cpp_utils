#!/bin/bash
#
# Installs the given library (shared object file) to /usr/lib.
# Note: needs root permissions
# 

lib=$1

sudo rm -f /usr/lib/$lib  # delete old library

sudo cp $lib /usr/lib
sudo chmod 0755 /usr/lib/$lib

sudo ldconfig