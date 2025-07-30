#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

char SHM_NAME[] = "garden_shm";
char SEM_NAME[] = "garden_sem";

typedef struct
{
    volatile int running;
    struct
    {
        float temperature;
        float air_humidity;
        float soil_moisture;
        float sunlight;
    } sensor_values;
    struct
    {
        int heating_mat_state;
        int fan_state;
        int air_humidifier_state;
        int irrigator_state;
        int artificial_light_state;
    } garden_devices;
    struct
    {
        int temperature_led;
        int air_humidity_led;
        int soil_moisture_led;
        int sunlight_led;
    } led_indicators;

} garden_data;

void handle_sigint(int sig)
{
    // ignore ctrl+c
}

void initialize_shared_memory(garden_data *shared_data)
{
    shared_data->running = 1;

    shared_data->sensor_values.temperature = 0;
    shared_data->sensor_values.air_humidity = 0;
    shared_data->sensor_values.soil_moisture = 0;
    shared_data->sensor_values.sunlight = 0;

    shared_data->garden_devices.heating_mat_state = 0;
    shared_data->garden_devices.fan_state = 0;
    shared_data->garden_devices.air_humidifier_state = 0;
    shared_data->garden_devices.irrigator_state = 0;
    shared_data->garden_devices.artificial_light_state = 0;

    shared_data->led_indicators.temperature_led = 0;
    shared_data->led_indicators.air_humidity_led = 0;
    shared_data->led_indicators.soil_moisture_led = 0;
    shared_data->led_indicators.sunlight_led = 0;
}

int main(int argc, char *argv[])
{
    int existing_shm;
    sem_t *existing_sem;

    int shm_descriptor;
    garden_data *shared_data;
    garden_data garden_local;
    sem_t *semaphore_descriptor;
    int SHM_SIZE = sizeof(garden_data);
    signal(SIGINT, handle_sigint);

    // Check if semaphore or shm exist - incorrect closing
    if (((existing_shm = shm_open(SHM_NAME, O_RDWR, 0666)) != -1) || ((existing_sem = sem_open(SEM_NAME, O_RDWR)) != SEM_FAILED))
    {
        printf("Wykryto niepoprawnie zamknięte zasoby z poprzedniego uruchomienia.\n");
        if (existing_shm != 1)
        {
            shm_unlink(SHM_NAME);
            close(existing_shm);
            printf("Wyczyszczono pamięć współdzieloną!\n");
        }
        if (existing_sem != SEM_FAILED)
        {
            sem_unlink(SEM_NAME);
            sem_close(existing_sem);
            printf("Wyczyszczono semafor!\n");
        }
        sleep(3);
        system("clear");
    }

    if ((shm_descriptor = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666)) == -1)
    {
        perror("shm_open failure");
        _exit(-1);
    }

    if (ftruncate(shm_descriptor, SHM_SIZE) == -1)
    {
        perror("ftruncate failure");
        _exit(-1);
    }

    shared_data = mmap(NULL, SHM_SIZE,
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED, shm_descriptor, 0);

    if (shared_data == MAP_FAILED)
    {
        perror("mmap failure");
        _exit(-1);
    }

    if ((semaphore_descriptor = sem_open(SEM_NAME, O_CREAT, 0666, 1)) == SEM_FAILED)
    {
        perror("sem_open failure");
        _exit(-1);
    }

    initialize_shared_memory(shared_data);

    while (1)
    {
        sem_wait(semaphore_descriptor);
        garden_local.running = shared_data->running;
        sem_post(semaphore_descriptor);

        if (!garden_local.running)
        {
            printf("\nOtrzymano polecenie zamknięcia. Kończenie pracy...\n");
            break;
        }

        sem_wait(semaphore_descriptor);
        garden_local.sensor_values = shared_data->sensor_values;
        garden_local.garden_devices = shared_data->garden_devices;
        sem_post(semaphore_descriptor);

        printf("--- Stan czujników ---\n");
        printf("Temperatura: %.2f°C\n", garden_local.sensor_values.temperature);
        printf("Wilgotność powietrza: %.2f%%\n", garden_local.sensor_values.air_humidity);
        printf("Wilgotność ziemi: %.2f%%\n", garden_local.sensor_values.soil_moisture);
        printf("Nasłonecznienie: %.2f lux\n", garden_local.sensor_values.sunlight);
        printf("\n\n");

        garden_local.garden_devices.heating_mat_state = (garden_local.sensor_values.temperature < 10) ? 1 : 0;
        garden_local.garden_devices.fan_state = (garden_local.sensor_values.air_humidity > 80) ? 1 : 0;
        garden_local.garden_devices.air_humidifier_state = (garden_local.sensor_values.air_humidity < 40) ? 1 : 0;
        garden_local.garden_devices.irrigator_state = (garden_local.sensor_values.soil_moisture < 30) ? 1 : 0;
        garden_local.garden_devices.artificial_light_state = (garden_local.sensor_values.sunlight < 1500) ? 1 : 0;

        garden_local.led_indicators.temperature_led = (garden_local.sensor_values.temperature < 10) ? 2 : (garden_local.sensor_values.temperature <= 15) ? 1
                                                                                                      : (garden_local.sensor_values.temperature <= 25)   ? 0
                                                                                                      : (garden_local.sensor_values.temperature < 30)    ? 1
                                                                                                                                                         : 2;

        garden_local.led_indicators.air_humidity_led = (garden_local.sensor_values.air_humidity < 40) ? 2 : (garden_local.sensor_values.air_humidity <= 50) ? 1
                                                                                                        : (garden_local.sensor_values.air_humidity <= 70)   ? 0
                                                                                                        : (garden_local.sensor_values.air_humidity < 80)    ? 1
                                                                                                                                                            : 2;

        garden_local.led_indicators.soil_moisture_led = (garden_local.sensor_values.soil_moisture < 30) ? 2 : (garden_local.sensor_values.soil_moisture <= 40) ? 1
                                                                                                          : (garden_local.sensor_values.soil_moisture <= 80)   ? 0
                                                                                                          : (garden_local.sensor_values.soil_moisture < 90)    ? 1
                                                                                                                                                               : 2;

        garden_local.led_indicators.sunlight_led = (garden_local.sensor_values.sunlight < 1500) ? 2 : (garden_local.sensor_values.sunlight <= 3000) ? 1
                                                                                                  : (garden_local.sensor_values.sunlight <= 7000)   ? 0
                                                                                                  : (garden_local.sensor_values.sunlight < 8500)    ? 1
                                                                                                                                                    : 2;

        sem_wait(semaphore_descriptor);
        shared_data->garden_devices = garden_local.garden_devices;
        shared_data->led_indicators = garden_local.led_indicators;
        sem_post(semaphore_descriptor);
        sleep(1);
    }

    munmap(shared_data, SHM_SIZE);
    close(shm_descriptor);
    sem_close(semaphore_descriptor);
    sem_unlink(SEM_NAME);
    shm_unlink(SHM_NAME);
    printf("Poprawnie zakończono działanie programu!\n");
    return 0;
}