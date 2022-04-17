#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#define errExit(x)                      \
    write(STDERR_FILENO, x, strlen(x)); \
    exit(EXIT_FAILURE);

int main(int argc, char **argv)
{
    srand(time(NULL));

    sem_t *sem;

    sem_unlink("/mysem");
    sem = sem_open("/mysem", O_CREAT, S_IRUSR | S_IWUSR, 10);

    if (sem == SEM_FAILED)
    {
        errExit("Aqcuiring a semaphore has failed!");
    }

    int fd = open("cstest", O_CREAT | O_RDWR | O_APPEND, S_IWUSR | S_IRUSR);

    if (fd == -1)
    {
        errExit("Failed to open cstest file!");
    }

    shm_unlink("/mymemshared"); // remove if exsits

    int shm_fd = shm_open("/mymemshared", O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);

    if (shm_fd == -1)
    {
        errExit("Failed to get a shared memory object!");
    }

    if (ftruncate(shm_fd, sizeof(unsigned int)) == -1)
    {
        errExit("Failed to truncate shared memory!");
    }

    unsigned int *memory = mmap(NULL, sizeof(unsigned int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (memory == MAP_FAILED)
    {
        errExit("Failed to map a shared memory object!");
    }

    close(shm_fd);

    char log_file_name[10];

    sprintf(log_file_name, "logfile.%d", atoi(argv[1]));

    int log_fd = open(log_file_name, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);

    if (log_fd == -1)
    {
        errExit("Failed to open logfile file!");
    }

    unsigned int queue;

    char log_data[30] = "Entering critical section!\n";

    time_t ptime;

    for (int i = 0; i < 5; i++)
    {
        write(log_fd, log_data, strlen(log_data));
        sem_wait(sem);

        sleep(rand() % 6);

        char writeData[100];

        queue = *memory;

        time(&ptime);
        char time_buffer[30];
        strcpy(time_buffer, ctime(&ptime));
        time_buffer[strlen(time_buffer) - 1] = '\0';
        sprintf(writeData, "%s Queue %d File modified by process number %s\n", time_buffer, queue, argv[1]);

        write(fd, writeData, strlen(writeData));

        queue++;
        *memory = queue;

        sleep(rand() % 6);

        sem_post(sem);
    }

    close(fd);
    sem_close(sem);
    close(log_fd);

    return 0;
}