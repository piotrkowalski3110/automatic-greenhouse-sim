SRC_DIR = src
BUILD_DIR = build
CFLAGS = -lrt -pthread
MATH_FLAGS = -lrt -pthread -lm

all: controller devices_monitor air_humidity soil_moisture sunlight temperature shutdown

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

controller: $(BUILD_DIR)
	gcc $(SRC_DIR)/control/controller.c -o $(BUILD_DIR)/controller $(CFLAGS)

devices_monitor: $(BUILD_DIR)
	gcc $(SRC_DIR)/control/devices_monitor.c -o $(BUILD_DIR)/devices_monitor $(CFLAGS)

air_humidity: $(BUILD_DIR)
	gcc $(SRC_DIR)/sensors/air_humidity.c -o $(BUILD_DIR)/air_humidity $(MATH_FLAGS)

soil_moisture: $(BUILD_DIR)
	gcc $(SRC_DIR)/sensors/soil_moisture.c -o $(BUILD_DIR)/soil_moisture $(MATH_FLAGS)

sunlight: $(BUILD_DIR)
	gcc $(SRC_DIR)/sensors/sunlight.c -o $(BUILD_DIR)/sunlight $(MATH_FLAGS)

temperature: $(BUILD_DIR)
	gcc $(SRC_DIR)/sensors/temperature.c -o $(BUILD_DIR)/temperature $(MATH_FLAGS)

shutdown: $(BUILD_DIR)
	gcc $(SRC_DIR)/system/shutdown.c -o $(BUILD_DIR)/shutdown $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean controller devices_monitor air_humidity soil_moisture sunlight temperature shutdown