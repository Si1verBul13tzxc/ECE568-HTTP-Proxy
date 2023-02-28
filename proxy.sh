#!/bin/bash
make clean
make
./proxy > proxy.log 2>&1 & 
while true; do continue; done