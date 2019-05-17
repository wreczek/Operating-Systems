#include <math.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/times.h>
#include <sys/time.h>

#define buff_size 512

const int M = 255;
const char * delims = " \t\n\r";
typedef struct timeval timeval;

int thread_number       = 4;
char * way_of_division;     // block / interleaved
char * image_file_name  = "apple.pgm";
char * filter_file_name = "filter_c3_2.txt";
char * result_file_name = "apple_res.pgm";    // obraz wynikowy

typedef struct thread_info {
    int s;
    int e;
} thread_info;

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

struct timeval gettime(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return time;
}

struct timeval diff_time(struct timeval s_time, struct timeval e_time){
    struct timeval diff;
    diff.tv_sec = e_time.tv_sec - s_time.tv_sec;
    diff.tv_usec = e_time.tv_usec - s_time.tv_usec;

    if (e_time.tv_usec < s_time.tv_usec){
        --diff.tv_sec;
        diff.tv_usec = - diff.tv_usec;
    }
    return diff;
}

void save_picture(int w, int h, int ** J, FILE * fp){
    //char buff[1024];

    fprintf(fp, "P2\n");
    fprintf(fp, "%d %d\n", w, h);
    fprintf(fp, "%d\n", 255);

    for (int i = 0; i < h; ++i){
        int j;
        for (j = 0; j < w-1; ++j){
            fprintf(fp, "%d ", J[i][j]);
        }
        fprintf(fp, "%d\n", J[i][j]);
    }

    fclose(fp);
}

void save_results(timeval time, int c){ // czasy
    FILE *fp = fopen("Times.txt", "a");
    fprintf(fp, "%d threads with c = %d worked for %ld.%06ld\n", thread_number, c, time.tv_sec, time.tv_usec);
    fclose(fp);
}

int calculate_pixel_value(int x, int y, int w, int h, int c, int ** I, double ** K){
    double pv;
    for (int i = 0; i < c; ++i){
        int a = (int)round(fmax(0, x - ceil(c/2) + i));
        a = a < h ? a : h-1;
        for (int j = 0; j < c; ++j){
            int b = (int)round(fmax(0, y - ceil(c/2) + j));
            b = b < w ? b : w-1;
            pv += I[a][b] * K[i][j];
        }
    }
    pv = pv < 0 ? 0 : pv; // ??
    return (int) round(pv);
}


int main(int argc, char **argv) {
    //if (argc != 6) /* --> */ error("bad args number");

//    thread_number    = atoi(argv[1]);
//    way_of_division  = argv[2];
//    image_file_name  = argv[3];
//    filter_file_name = argv[4];
//    result_file_name = argv[5];

    char buff[buff_size];
    FILE * fp;

    /// ### ------ FILTER ------- ###
    if ((fp = fopen(filter_file_name, "r")) == NULL)   error("fopen f_file");

    fgets(buff, buff_size, fp); // c

    int c = (int) strtol(buff, NULL, 10);
    printf("c = %d", c);
    double ** K = calloc((size_t)c, sizeof(double*));
    for (int i = 0; i < c; ++i){
        K[i] = calloc((size_t)c, sizeof(double));
    }

    int i = 0, j = 0;
    while (fgets(buff, buff_size, fp) != 0){
        double k;
        char * num;
        char * content = strdup(buff);
        while ((num = strsep(&content, delims)) != 0){
            if (strcmp(num, "") == 0) continue;
            if (i == c) {
                ++i;
                j = 0;
            }
            sscanf(num, "%lf", &k);
            K[i][j] = k;
            if (i == c-1 && j == c-1) break;
            ++j;
        }
    }
    fclose(fp);
    /// ### ------ FILTER ------- ###
    /// |||||||||||||||||||||||||||||
    /// ### ------- INPUT ------- ###
    if ((fp = fopen(image_file_name, "r"))  == NULL)   error("fopen i_file");

    int w, h;

    fgets(buff, buff_size, fp); // P2
    fgets(buff, buff_size, fp); // dimensions

    char * dims;

    dims = strdup(buff);
    w = (int) strtol(strsep(&dims, delims), NULL, 10);
    h = (int) strtol(strsep(&dims, delims), NULL, 10);

    fgets(buff, buff_size, fp); // M

    int ** I = calloc((size_t)h, sizeof(int*));
    for (int i = 0; i < h; ++i) {
        I[i] = calloc((size_t)w, sizeof(int));
    }

    i = 0, j = 0;
    while (fgets(buff, buff_size, fp) != 0){ // proper content
        int k;
        char * content = strdup(buff);
        char * num;
        while ((num = strsep(&content, delims)) != 0){
            if (strcmp(num, "") == 0) continue;
            if (j == w){
                j = 0;
                ++i;
            }
            k = (int) strtol(num, NULL, 10);
            I[i][j] = k;
            if (i == h-1 && j == w-1) break;
            j++;
        }
    }
    fclose(fp);
    /// ### ------- INPUT ------- ###
    /// |||||||||||||||||||||||||||||
    /// ### ------ OUTPUT ------- ###
    int ** J = calloc((size_t)h, sizeof(int*));
    for (int i = 0; i < h; ++i){
        J[i] = calloc((size_t)w, sizeof(int));
    }

    if ((fp = fopen(result_file_name, "w")) == NULL)   error("fopen o_file");
//    printf("### --- DUPA --- ###\n");


    /// ### ------ OUTPUT ------- ###
    /// |||||||||||||||||||||||||||||

    /// ### ------- BLOCK ------- ###
    int thread_id;
    pthread_t *thread = calloc((size_t)thread_number, sizeof(pthread_t));
    struct thread_info **threads_info = malloc(thread_number * sizeof(thread_info *));

    struct timeval s_time = gettime();

    for (int i = 0; i < thread_number; i++) {
        threads_info[i]      = malloc(sizeof(thread_info));
        threads_info[i]->s   = (i * w / thread_number);
        threads_info[i]->e   = ((i + 1) * w / thread_number);
    }

    void *single_thread(void *infos) {
        struct thread_info *thread_infos = (thread_info *) infos;
        for (int y = thread_infos->s; y < thread_infos->e; ++y) {
            for (int x = 0; x < h; ++x){
                J[x][y] = calculate_pixel_value(x, y, w, h, c, I, K);
            }
        }
        return (void *) 0;
    }

    for (int i = 0; i < thread_number; ++i){
        if ((thread_id = pthread_create(&thread[i], NULL, single_thread, (void*)threads_info[i])) != 0)
            error("pthread_create");
    }

    int * rval_ptr = calloc((size_t)thread_number, sizeof(int));

    for (int i = 0; i < thread_number; ++i){
        pthread_join(thread[i], (void**)&rval_ptr); // dziala
        free(threads_info[i]);
    }
    //printf("%d", rval_ptr);
    free(threads_info);

    struct timeval e_time = gettime();
    struct timeval d_time = diff_time(s_time, e_time);

    /// ### ------- BLOCK ------- ###
    /// |||||||||||||||||||||||||||||
    /// ### ---- INTERLEAVED ---- ###

    save_picture(w, h, J, fp);
    save_results(d_time, c);
    //fclose(fp);

    for (int i = 0; i < h; ++i){
        for (int j = 0; j < w; ++j){
            printf("J[%d][%d] = %d\n", i, j, J[i][j]);
        }
    }

    // free the memory
    for (int i = 0; i < h; ++i)      free(I[i]);
    free(I);

    for (int i = 0; i < h; ++i)      free(J[i]);
    free(J);

//    for (int i = 0; i < c; ++i)      free(K[i]);
//    free(K);

    free(thread);

    return 0;
}
