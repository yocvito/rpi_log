#!/bin/bash

mkdir logs

gcc -o rpilogs rpi_log_severalDevices.c

chmod 700 ./*
