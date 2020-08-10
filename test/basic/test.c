#include <pthread.h>

int main (int argc, char *argv[]) {
	int i;
	int perr;
	pthread_mutex_t m;

	perr = pthread_mutex_init (&m, NULL);
	if (perr) { printf ("init err\n"); }

	perr = pthread_mutex_lock (&m);
	if (perr) { printf ("lock err\n"); }

	perr = pthread_mutex_unlock (&m);
	if (perr) { printf ("unlock err\n"); }

	perr = pthread_mutex_destroy (&m);
	if (perr) { printf ("destroy err\n"); }

	return 0;
}