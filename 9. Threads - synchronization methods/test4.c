#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#define test_errno(msg) do{if (errno) {perror(msg); exit(EXIT_FAILURE);}} while(0)

#define	N 10
#define K 100

pthread_mutex_t blokada1;
pthread_mutex_t blokada2;

int licznik = 0;

void ms_sleep(unsigned ms) {
	struct timespec req;
	req.tv_sec  = (ms / 1000);
	req.tv_nsec = (ms % 1000 * 1000000);
	nanosleep(&req, NULL);
}

void* watek1(void* numer) {
	errno = pthread_mutex_lock(&blokada2);
	printf("Numer w watku = %d\n", numer);
	usleep(5000);
    errno = pthread_mutex_unlock(&blokada2);


    errno = pthread_mutex_lock(&blokada1);
    errno = pthread_mutex_lock(&blokada2);
    test_errno("pthread_mutex_lock");

    licznik = licznik + 1;
    ms_sleep(1);

    errno = pthread_mutex_unlock(&blokada2);
    errno = pthread_mutex_unlock(&blokada1);
    test_errno("pthread_mutex_unlock");
    usleep(7000);
    printf("###KONIEC###");
	return NULL;
}

void* watek2(void* numer) {
	errno = pthread_mutex_lock(&blokada1);
	printf("Numer w watku = %d\n", numer);
	usleep(7000);
	errno = pthread_mutex_unlock(&blokada1);


    errno = pthread_mutex_lock(&blokada2);
    test_errno("pthread_mutex_lock");

    licznik = licznik + 1;
    ms_sleep(1);

    errno = pthread_mutex_unlock(&blokada2);
    test_errno("pthread_mutex_unlock");

	return NULL;
}

int main() {
	pthread_t id[2];
	int i;

	printf("licznik = %d\n", licznik);

	errno = pthread_mutex_init(&blokada1, NULL);
	test_errno("pthread_mutex_init");

	errno = pthread_mutex_init(&blokada2, NULL);
	test_errno("pthread_mutex_init");

    errno = pthread_create(&id[0], NULL, watek1, (void*)0);
    test_errno("pthread_create");

    errno = pthread_create(&id[1], NULL, watek2, (void*)1);
    test_errno("pthread_create");

    errno = pthread_join(id[0], NULL);
    test_errno("pthread_join");

    errno = pthread_join(id[1], NULL);
    test_errno("pthread_join");

	printf("licznik = %d, expected value = %d %s\n",
		licznik,
		N*K,
		(licznik != N*K ? "Wrong answer!!!" : "")
	);

	return EXIT_SUCCESS;
}
