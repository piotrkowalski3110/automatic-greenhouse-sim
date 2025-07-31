#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int main(int argc, char *argv[])
{
    int shm_descriptor;
    garden_data *shared_data;
    sem_t *semaphore_descriptor;
    int SHM_SIZE = sizeof(garden_data);

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

    sem_wait(semaphore_descriptor);
    shared_data->running = 0;
    sem_post(semaphore_descriptor);

    munmap(shared_data, SHM_SIZE);
    close(shm_descriptor);
    sem_close(semaphore_descriptor);

    printf("Sygnał wyłączenia został wysłany\n");
    return 0;
}