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
#include <math.h>

#define MIN_HUMIDITY 0.0f
#define MAX_HUMIDITY 100.0f

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
    printf("\nCtrl+C zostało zignorowane. Użyj 'q' aby zakończyć program.\n");
}

int main(int argc, char *argv[])
{
    int shm_descriptor;
    garden_data *shared_data;
    sem_t *semaphore_descriptor;
    int SHM_SIZE = sizeof(garden_data);
    signal(SIGINT, handle_sigint);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

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

    printf("Podaj wilgotność powietrza w szklarni: ");

    while (1)
    {
        float air_humidity;
        int is_running;
        char buffer[10];

        sem_wait(semaphore_descriptor);
        is_running = shared_data->running;
        sem_post(semaphore_descriptor);

        if (!is_running)
        {
            printf("\nOtrzymano sygnał zamknięcia. Kończenie pracy...\n");
            break;
        }

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            buffer[strcspn(buffer, "\n")] = 0;

            if (buffer[0] == 'q' || buffer[0] == 'Q')
            {
                printf("\nZakonczono program przez polecenie 'q'.\n");
                break;
            }

            if (sscanf(buffer, "%f", &air_humidity) != 1)
            {
                printf("\nPodano niepoprawne dane.\n\n");
                printf("Podaj wilgotność powietrza w szklarni: ");
                continue;
            }

            float normalized_air_humidity = fmin(fmax(air_humidity, MIN_HUMIDITY), MAX_HUMIDITY);

            sem_wait(semaphore_descriptor);
            shared_data->sensor_values.air_humidity = normalized_air_humidity;
            sem_post(semaphore_descriptor);

            printf("Podaj wilgotność powietrza w szklarni: ");
        }
    }

    munmap(shared_data, SHM_SIZE);
    close(shm_descriptor);
    sem_close(semaphore_descriptor);

    printf("Program zakończony pomyślnie.\n");
    return 0;
}