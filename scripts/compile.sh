#!/bin/bash

gcc ../src/control/controller.c -o ../build/control -lrt -pthread
gcc ../src/control/devices_monitor.c -o ../build/devices -lrt -pthread

gcc ../src/sensors/air_humidity.c -o ../build/air -lrt -pthread -lm
gcc ../src/sensors/soil_moisture.c -o ../build/soil -lrt -pthread -lm
gcc ../src/sensors/sunlight.c -o ../build/sun -lrt -pthread -lm
gcc ../src/sensors/temperature.c -o ../build/temp -lrt -pthread -lm

gcc ../src/system/shutdown.c -o ../build/shutdown -lrt -pthread