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

int main(int argc, char *argv[])
{
    int shm_descriptor;
    garden_data *shared_data;
    garden_data garden_local;
    sem_t *semaphore_descriptor;
    int SHM_SIZE = sizeof(garden_data);
    signal(SIGINT, handle_sigint);

    if ((shm_descriptor = shm_open(SHM_NAME, O_RDWR, 0666)) == -1)
    {
        perror("shm_open failure");
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

    if ((semaphore_descriptor = sem_open(SEM_NAME, 0)) == SEM_FAILED)
    {
        perror("sem_open failure");
        _exit(-1);
    }

    while (1)
    {
        int is_running;

        sem_wait(semaphore_descriptor);
        is_running = shared_data->running;
        sem_post(semaphore_descriptor);

        if (!is_running)
        {
            printf("\nOtrzymano sygnał zamknięcia. Kończenie pracy...\n");
            break;
        }

        sem_wait(semaphore_descriptor);
        garden_local.garden_devices = shared_data->garden_devices;
        garden_local.led_indicators = shared_data->led_indicators;
        sem_post(semaphore_descriptor);

        printf("--- Urządzenia ---\n");
        printf("Mata grzewcza: %s\n", garden_local.garden_devices.heating_mat_state ? "Włączona" : "Wyłączona");
        printf("Wentylator: %s\n", garden_local.garden_devices.fan_state ? "Włączony" : "Wyłączony");
        printf("Nawilżacz powietrza: %s\n", garden_local.garden_devices.air_humidifier_state ? "Włączony" : "Wyłączony");
        printf("System nawadniania: %s\n", garden_local.garden_devices.irrigator_state ? "Włączony" : "Wyłączony");
        printf("Sztuczne oświetlenie: %s\n", garden_local.garden_devices.artificial_light_state ? "Włączone" : "Wyłączone");

        printf("\n--- Diody ostrzegawcze ---\n");
        printf("Temperature LED: %s\n", garden_local.led_indicators.temperature_led == 0 ? "zielona" : garden_local.led_indicators.temperature_led == 1 ? "pomaranczowa" : "czerwona");
        printf("Humidity LED: %s\n", garden_local.led_indicators.air_humidity_led == 0 ? "zielona" : garden_local.led_indicators.air_humidity_led == 1 ? "pomaranczowa" : "czerwona");
        printf("Moisture LED: %s\n", garden_local.led_indicators.soil_moisture_led == 0 ? "zielona" : garden_local.led_indicators.soil_moisture_led == 1 ? "pomaranczowa" : "czerwona");
        printf("Sunlight LED: %s\n", garden_local.led_indicators.sunlight_led == 0 ? "zielona" : garden_local.led_indicators.sunlight_led == 1 ? "pomaranczowa" : "czerwona");
        printf("\n\n");
        sleep(1);
    }

    munmap(shared_data, SHM_SIZE);
    close(shm_descriptor);
    sem_close(semaphore_descriptor);
    printf("Poprawnie zakończono działanie programu!\n");
    return 0;
}