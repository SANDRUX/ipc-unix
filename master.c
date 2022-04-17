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
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>

static unsigned int timer_status = 0;
static int log_fd;

void timer_has_expired(union sigval timer_data)
{
    // *(int *)(timer_data.sival_ptr) = 1;
    // printf("alarm, signal rised!, %d", timer_status);
    // fflush(stdout);
    system("killall slave");
    exit(EXIT_FAILURE);
}

void sigint_handler(int sig_num)
{
    char *message = "Siginit recieved!";
    write(log_fd, message, strlen(message));
    printf(message);
    fflush(stdout);
}

#define errExit(x)                          \
    write(STDERR_FILENO, x, strlen(x) + 1); \
    exit(EXIT_FAILURE);

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        errExit("Usage: ./master -t ss n \n-t [specify time and number of proccesses]");
    }

    int proc_count;
    int time_proc;

    if (atoi(argv[3]) > 20)
    {
        const char *warning = "Number of procceses can be equal or lass than 20, setting to 20 now\n";
        write(STDOUT_FILENO, warning, strlen(warning) + 1);

        proc_count = 20;
    }

    time_proc = atoi(argv[2]);
    proc_count = atoi(argv[3]);

    int fd = open("cstest", O_CREAT | O_WRONLY | O_APPEND, S_IWUSR | S_IRUSR);

    if (fd == -1)
    {
        errExit("Failed to open cstest file!");
    }

    log_fd = open("logfile", O_CREAT | O_WRONLY | O_APPEND, S_IWUSR | S_IRUSR);

    if (log_fd == -1)
    {
        errExit("Failed to open cstest file!");
    }

    // char *args[] = {"12"};

    // printf("GOT HERE ");
    char command[10];

    pid_t pids[proc_count];
    pid_t temp;

    struct sigevent timer_signal_event;
    timer_t timer;

    struct itimerspec timer_period;

    timer_signal_event.sigev_notify = SIGEV_THREAD;
    timer_signal_event.sigev_notify_function = timer_has_expired;

    timer_signal_event.sigev_value.sival_ptr = (void *)&timer_status; // as will this (both in a structure)
    timer_signal_event.sigev_notify_attributes = NULL;
    timer_create(CLOCK_MONOTONIC, &timer_signal_event, &timer);

    timer_period.it_value.tv_sec = time_proc;
    timer_period.it_value.tv_nsec = 0;
    timer_period.it_interval.tv_sec = 0;
    timer_period.it_interval.tv_nsec = 0;

    timer_settime(timer, 0, &timer_period, NULL);

    char *proc_var[3];
    proc_var[0] = "./slave";
    // sprintf(proc_var[0], "./slave");
    proc_var[1] = malloc(sizeof(char) * 16);
    proc_var[2] = NULL;

    signal(SIGINT, sigint_handler);

    for (int i = 0; i < proc_count; i++)
    {
        switch ((temp = fork()))
        {
        case -1:
            errExit("Failed to fork a child process");
            break;

        case 0:
            sprintf(proc_var[1], "%d", i);
            execv("./slave", (char **)&proc_var);
            // sprintf(command, "./slave %d", i);

            // sprintf(command, "./slave %d", i);

            // execve("./slave", &command, NULL);
            // system(command); // a variant of exec
            _exit(EXIT_SUCCESS);

        default:
            // printf("PROCESS CREATED\n");
            fflush(stdout);
            pids[i] = temp;
            break;
        }
    }

    for (int i = 0; i < proc_count; i++)
    {
        // printf("timer status: %d\n", timer_status);
        // fflush(stdout);
        // if (timer_status)
        // {
        //     char command[30];
        //     sprintf(command, "kill -9 %d", pids[i]);

        //     system(command);

        //     exit(EXIT_FAILURE);
        // }

        wait(NULL);
    }

    close(fd);

    free(proc_var[1]);

    timer_delete(timer);

    close(log_fd);

    return 0;
}