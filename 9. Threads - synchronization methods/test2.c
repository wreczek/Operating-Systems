#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#define test_errno(msg) do{if (errno) {perror(msg); exit(EXIT_FAILURE);}} while(0)


#define	N 10	/* liczba w¹tków */
#define K 100	/* liczba iteracji (z t¹ wartoœci¹ nale¿y eksperymentowaæ) */

//int blokada;

pthread_mutex_t blokada;

int licznik = 0;		// globalny licznik, powinien byæ chroniony blokad¹
int licznik2 = 0;
int licznik3 = 0;

void ms_sleep(unsigned ms) {
	struct timespec req;
	req.tv_sec  = (ms / 1000);
	req.tv_nsec = (ms % 1000 * 1000000);
	nanosleep(&req, NULL);
}
//------------------------------------------------------------------------

void* watek(void* numer) {
	int i;
	errno = pthread_mutex_lock(&blokada);
	printf("Numer w watku = %d\n", numer);
	errno = pthread_mutex_unlock(&blokada);
	for (i=0; i < K; i++) {
		errno = pthread_mutex_lock(&blokada);
		test_errno("pthread_mutex_lock");

		licznik = licznik + 1;
		licznik2++;
		ms_sleep(1);

		errno = pthread_mutex_unlock(&blokada);
		test_errno("pthread_mutex_unlock");
		ms_sleep(50);
	}

	return NULL;
}

int main() {
	pthread_t id[N];
	int i;

	printf("licznik = %d\n", licznik);
	printf("licznik2 = %d\n", licznik2);
	printf("licznik3 = %d\n", licznik3);

	errno = pthread_mutex_init(&blokada, NULL);
	test_errno("pthread_mutex_init");

	/* utworzenie w¹tku */
	for (i=0; i < N; i++) {
		errno = pthread_create(&id[i], NULL, watek, (void*)i);
		test_errno("pthread_create");
	}

	/* oczekiwanie na jego zakoñczenie */
	for (i=0; i < N; i++) {
		errno = pthread_join(id[i], NULL);
		test_errno("pthread_join");
	}

	printf("licznik = %d, licznik2 = %d, licznik3 = %d, expected value = %d %s\n",
		licznik,
		licznik2,
		licznik3,
		N*K,
		(licznik != N*K ? "Wrong answer!!!" : "")
	);

	return EXIT_SUCCESS;
}
