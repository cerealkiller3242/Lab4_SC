#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

//El codigo se inicia con un
//gcc -pthread -std=c11 -o EJ2_LAB4_SC EJ2_LAB4_SC.c
//./EJ2_LAB4_SC

//El codigo lo detenmos con ctrl+c

static atomic_int print_numbers_flag = 0;
static atomic_int running = 1;

void handle_sigint(int sig) {
    (void)sig;
    atomic_store(&running, 0);
}

void *timer_thread(void *arg) {
    (void)arg;
    //En esta parte el codigo comienza a correr hasta que se reciba un ctrl+c
    //Se usa el comando sleep para la temporización
    while (atomic_load(&running)) {
        sleep(5); // esperar los 5 s para entrar a la interrupción
        if (!atomic_load(&running)) break;
        atomic_store(&print_numbers_flag, 1);
        // flag de impresión de números
        printf("interrupcion INICIADA (flag=1)\n");
        fflush(stdout);
        sleep(2); // esperar 2 s hasta finalizar "interrupción"
        atomic_store(&print_numbers_flag, 0);
        // flag de impresión de letras
        printf("interrupcion FINALIZADA (flag=0)\n");
        fflush(stdout);
    }
    return NULL;
}

//hilo que imprime letras o números según el flag
void *printer_thread(void *arg) {
    (void)arg;
    const char letters[] = "abcdefghijklmnopqrstuvwxyz";
    int idxA = 0;
    int idxN = 0;
    struct timespec ts = {0, 100000000L}; // 0.1 s

    while (atomic_load(&running)) {
        // dependiendo del flag se imprime un número o una letra
        // se usa un modulo para ciclar entre los caracteres y se repitan
        if (atomic_load(&print_numbers_flag)) {
            printf("%d\n", idxN);
            idxN = (idxN + 1) % 10;
        } else {
            printf("%c\n", letters[idxA]);
            idxA = (idxA + 1) % 26;
        }
        fflush(stdout);
        nanosleep(&ts, NULL);
    }
    return NULL;
}

int main(void) {
    struct sigaction sa = {0};
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    pthread_t t_timer, t_printer;
    if (pthread_create(&t_timer, NULL, timer_thread, NULL) != 0) {
        perror("pthread_create timer");
        return 1;
    }
    if (pthread_create(&t_printer, NULL, printer_thread, NULL) != 0) {
        perror("pthread_create printer");
        atomic_store(&running, 0);
        pthread_join(t_timer, NULL);
        return 1;
    }

    //cuando se detenga el programa se limpia el timer y el printer
    while (atomic_load(&running)) pause();

    atomic_store(&running, 0);
    pthread_join(t_timer, NULL);
    pthread_join(t_printer, NULL);
    printf("ej2: programa usuario terminado\n");
    return 0;
}
