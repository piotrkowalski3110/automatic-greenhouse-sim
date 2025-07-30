#!/bin/bash
gcc controller.c -o control -lrt -pthread
gcc temperature.c -o temp -lrt -pthread -lm
gcc air_humidity.c -o air -lrt -pthread -lm
gcc soil_moisture.c -o soil -lrt -pthread -lm
gcc sunlight.c -o sun -lrt -pthread -lm
gcc shutdown.c -o shutdown -lrt -pthread
gcc devices_monitor.c -o devices -lrt -pthread