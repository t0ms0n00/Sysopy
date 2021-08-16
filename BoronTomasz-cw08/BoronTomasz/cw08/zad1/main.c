#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

int th_num;
int height;
int width;
int pixel_range = 255;
int** input_matrix;
int** result_matrix;

void read_input(char* filename){
    FILE* input_file = fopen(filename, "r");
    if(input_file == NULL){
        perror("No such file\n");
        exit(2);
    }

    char buff[5];

    fscanf(input_file, "%s", buff);
    fscanf(input_file, "%d %d", &width, &height);
    fscanf(input_file, "%d", &pixel_range);     /// to omit value
    pixel_range = 255;

    input_matrix = calloc(height, sizeof(int*));
    for(int i = 0; i < height; i++){
        input_matrix[i] = calloc(width, sizeof(int));
    }

    result_matrix = calloc(height, sizeof(int*));
    for(int i = 0; i < height; i++){
        result_matrix[i] = calloc(width, sizeof(int));
    }

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            fscanf(input_file, "%d", &input_matrix[i][j]);
        }
    }

    fclose(input_file);
}

void write_output(char* filename){

    FILE* output_f = fopen(filename, "w");

    if(output_f == NULL){
        perror("Cannot open file\n");
        exit(3);
    }

    fprintf(output_f, "%s\n", "P2");
    fprintf(output_f, "%d %d\n", width, height);
    fprintf(output_f, "%d\n", pixel_range);
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            fprintf(output_f, "%d ", result_matrix[i][j]);
        }
        fprintf(output_f, "\n");
    }

    fclose(output_f);
}

void processing_numbers_mode(int modulo){
    printf("Thread %d start his job\n", modulo);
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            if(input_matrix[i][j] % th_num == modulo){
                result_matrix[i][j] = pixel_range - input_matrix[i][j];
            }
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);
    int result = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000.0;
    double time_val = 1.0 * result/1000000.0;
    pthread_exit(&time_val);
}

void processing_block_mode(int id){
    int multiplier = width/th_num;
    if(width % th_num != 0) multiplier++;
    int left, right;
    left = (id-1)*multiplier;
    right = id*multiplier - 1;
    if (right >= width) right = width-1;
    printf("Thread %d between lines %d %d start his job\n", id, left, right);
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    for(int i = 0; i < height; i++){
        for(int j = left; j <= right; j++){
            result_matrix[i][j] = pixel_range - input_matrix[i][j];
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);
    int result = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000.0;
    double time_val = 1.0 * result/1000000.0;
    pthread_exit(&time_val);
}

int main(int argc, char** argv){
    if(argc != 5){
        perror("Bad number of arguments\n");
        exit(1);
    }
    th_num = atoi(argv[1]);
    char method[10];
    strcpy(method, argv[2]);
    char input_filename[50];
    strcpy(input_filename, argv[3]);
    char output_filename[50];
    strcpy(output_filename, argv[4]);

    read_input(input_filename);

    struct timespec main_start, main_end;
    clock_gettime(CLOCK_REALTIME, &main_start);

    pthread_t* threads_set = calloc(th_num, sizeof(pthread_t));

    if(strcmp(method, "numbers") == 0){
        for(int i = 0; i < th_num; i++){
            pthread_create(&threads_set[i], NULL, (void*) processing_numbers_mode, i);
        }
    }
    else if(strcmp(method, "block") == 0){
        for(int i = 0; i < th_num; i++){
            pthread_create(&threads_set[i], NULL, (void*) processing_block_mode, i+1);
        }
    }
    else{
        perror("Bad mode: [numbers/block]\n");
        exit(4);
    }

    for(int i = 0; i < th_num; i++){
        double result;
        pthread_join(threads_set[i], (void*) &result);
        printf("Thread %d worked for %lf\n", i, result);
    }

    clock_gettime(CLOCK_REALTIME, &main_end);
    int main_result = (main_end.tv_sec - main_start.tv_sec) * 1000000 + (main_end.tv_nsec - main_start.tv_nsec) / 1000.0;
    double main_time_val = 1.0 * main_result/1000000.0;
    printf("Main thread worked for %lf\n", main_time_val);

    write_output(output_filename);

    for(int i = 0; i < height; i++){
        free(input_matrix[i]);
        free(result_matrix[i]);
    }
    free(input_matrix);
    free(result_matrix);
    free(threads_set);
    return 0;
}