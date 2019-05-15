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

int thread_number;
char * way_of_division;     // block / interleaved
char * image_file_name;
char * filter_file_name = "filter_c3.txt";
char * result_file_name;    // obraz wynikowy

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

struct timeval gettime(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return time;
}

void save_picture(int w, int h, int ** J, FILE * fp){
    char buff[1024];

    fprintf(fp, "P2\n");
    fprintf(fp, "%d %d\n", w, h);
    fprintf(fp, "%d\n", 255);

    // ....

    fclose(fp);
}

void save_results(timeval time, ...){ // czasy
    FILE *fp = fopen("Times.txt", "a");

    // ....

    fclose(fp);
}

int calculate_pixel_value(int x, int y, int w, int h, int c, int ** I, double ** K){
    double pv;
    for (int i = 0; i < c; ++i){
        int a = (int)round(fmax(0, x - ceil(c/2) + i));
        a < h ? a : h-1;
        for (int j = 0; j < c; ++j){
            int b = (int)round(fmax(0, y - ceil(c/2) + j));
            b = b < w ? b : w-1;
            pv += I[a][b] * K[i][j];
        }
    }
    //pv = pv < 0 ? 0 : pv;
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

    // przetworzymy plik z filtrami
    printf("File name: %s\n", filter_file_name);
    if ((fp = fopen(filter_file_name, "r")) == NULL)   error("fopen f_file");

    fgets(buff, buff_size, fp); // c

    int c = (int) strtol(buff, NULL, 10);

    double ** K = calloc((size_t)c, sizeof(double*));
    for (int i = 0; i < c; ++i){
        K[i] = calloc((size_t)c, sizeof(double));
    }
    int i = 0, j = 0;
    while (fgets(buff, buff_size, fp) != 0){
        char * num;
        char * content = strdup(buff);
        while ((num = strsep(&content, delims)) != 0){
            printf("num = %s\n", num);
            double k;
            sscanf(num, "%lf", &k);
            fflush(stdout);
            fflush(stdin);
            if (i == c) {
                ++j;
                i = 0;
            }

            K[i++][j] = k;
        }
//    printf("### --- DUPA --- ###\n");
    }

    for (int i = 0; i < c; ++i){
        for (int j = 0; j < c; ++j){
            printf("%f ", K[i][j]);
        }
        printf("\n");
    }

    fclose(fp);
    /*
    // przetworzymy plik z obrazem wejsciowym
    if ((i_file = fopen(image_file_name, "r"))  == NULL)   error("fopen i_file");

    int w, h;

    fgets(buff, buff_size, f_file); // P2
    fgets(buff, buff_size, f_file); // dimensions

    printf(">>%s<<\n", buff);

    char * dims;

    dims = strdup(buff);
    w = (int) strtol(strsep(&dims, delims), NULL, 10);
    h = (int) strtol(strsep(&dims, delims), NULL, 10);

    fgets(buff, buff_size, f_file); // M
    fgets(buff, buff_size, f_file); // proper content

    double ** K = calloc((size_t)c, sizeof(double*));
    for (int i = 0; i < c; ++i){
        K[i] = calloc((size_t)c, sizeof(double));
    }

    fclose(i_file);

    // operacje na pliku wyjsciowym
    if ((o_file = fopen(result_file_name, "w")) == NULL)   error("fopen o_file");

    fclose(o_file);

    /// BLOCK || INTERLEAVED

    int thread_id;
    pthread_t *thread = calloc((size_t)thread_number, sizeof(pthread_t));

    printf("tid = %d\n", pthread_self());
    if ((thread_id = pthread_create(thread, NULL, NULL, NULL)) != 0) error("pthread_create");
    */
    return 0;
}
