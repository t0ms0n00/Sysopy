#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int max_commands = 10;
int max_command_length = 1000;
int max_arguments = 5;

void find_def_in_file(char* score, char* pattern, FILE* data){
    fseek(data, 0, 0);
    int n = strlen(pattern);
    char* file_fragment = calloc(n, sizeof(char));
    fread(file_fragment, sizeof(char), n, data);
    while(strcmp(file_fragment, pattern) != 0){
        fseek(data, -n+1, 1);
        fread(file_fragment, sizeof(char), n, data);
    }
    fseek(data, 3, 1);
    free(file_fragment);
    fgets(score, max_command_length, data);
}

void command_parse(char* non_parsed, char* parsed, FILE* data){
    int last_break_pos = 0;
    for(int i = 0; i < strlen(non_parsed); i++){
        if(non_parsed[i] == '\n' || non_parsed[i] == ' '){
            char* pattern = calloc(i-last_break_pos-1, sizeof(char));
            int counter = 0;
            for(int j = last_break_pos; j < i; j++){
                pattern[counter++] = non_parsed[j];
            }
            char* score = calloc(max_command_length, sizeof(char));
            find_def_in_file(score, pattern, data);
            i+=2;
            last_break_pos = i+1;
            strcat(parsed, score);
            strtok(parsed, "\n");
            if(i <strlen(non_parsed)) strcat(parsed, " | ");
            free(pattern);
            free(score);
        }
    }
}

int count_programs(char* command){
    int counter = 1;
    for(int i = 0; i < strlen(command); i++){
        if (command[i] == '|') counter++;
    }
    return counter;
}

void build_arguments_table(char* command, char* program, char** arguments, int from, int to){
    int table_index = 0;
    int last_break = from;
    int program_found = 0;
    int i;
    for(i = from; i <= to; i++){
        if(i == to || command[i] == ' '){
            if(!program_found){
                program_found = 1;
                for(int j = last_break; j < i; j++){
                    char* sign = calloc(1, sizeof(char));
                    sign[0] = command[j];
                    strcat(program, sign);
                    free(sign);
                }
            }
            for(int j = last_break; j < i; j++){
                char* sign = calloc(1, sizeof(char));
                sign[0] = command[j];
                strcat(arguments[table_index], sign);
                free(sign);
            }
            table_index += 1;
            last_break = i + 1;
        }
    }
    if(to == strlen(command)) {
        arguments[table_index] = NULL;
    }
    else{
        arguments[table_index-1] = NULL;
    }
}

void execute_command(char* command){
    printf("%s\n",command);
    int n = strlen(command);
    int program_counter = count_programs(command);
    int text_ptr = 0;
    int prev_break = 0;
    int current[2];
    int prev[2];
    for(int i = 0; i < program_counter; i++){
        while(text_ptr < n && command[text_ptr] != '|') text_ptr++;
        char* prog_name = calloc(50, sizeof(char));
        char** arguments = calloc(max_arguments, sizeof(char*));
        for(int j = 0; j < max_arguments; j++){
            arguments[j] = calloc(max_command_length, sizeof(char));
        }
        build_arguments_table(command, prog_name, arguments, prev_break, text_ptr);
        /*printf("Nazwa programu %s\n", prog_name);
        for(int j=0; j<max_arguments ; j++){
            printf("Argument %d to %s \n",j, arguments[j]);
        }*/
        pipe(current);
        pid_t child_pid = fork();
        if(child_pid == 0){
            close(current[0]);
            close(prev[1]);
            if(i > 0) dup2(prev[0], STDIN_FILENO);
            if(i < program_counter - 1) dup2(current[1], STDOUT_FILENO);
            execvp(prog_name, arguments);
            exit(1);
        }
        text_ptr += 2;
        prev_break = text_ptr;
        close(current[1]);
        close(prev[0]);
        prev[0] = current[0];
        prev[1] = current[1];
        for(int j = 0; j < max_arguments; j++){
            free(arguments[j]);
        }
        free(arguments);
        free(prog_name);
    }
    for(int i = 0; i < program_counter; i++) wait(NULL);
}

int main(int argc, char** argv){
    if(argc < 2){
        perror("./main [data_file]\n");
        exit(1);
    }
    FILE* data_set = fopen(argv[1], "r");
    if(data_set == NULL){
        perror("No such file\n");
        exit(1);
    }
    char* line = calloc(256, sizeof(char));
    size_t line_buf_size = 0;
    getline(&line, &line_buf_size, data_set);
    while(!feof(data_set) && strcmp(&line[0],"\n") != 0){
        getline(&line, &line_buf_size, data_set);
    }
    free(line);
    char** non_parsed_commands = calloc(max_commands, sizeof(char*));
    for (int i = 0; i < max_commands; i++){
        non_parsed_commands[i] = calloc(max_command_length, sizeof(char));
    }
    int counter = 0;
    while(!feof(data_set)){
        char* line = calloc(256, sizeof(char));
        size_t line_buf_size = 0;
        getline(&line, &line_buf_size, data_set);
        strcpy(non_parsed_commands[counter++], line);
        free(line);
    }
    int command_num = 0;
    while(strlen(non_parsed_commands[command_num]) > 0 && command_num < max_commands){
        char* cmd = calloc(max_command_length, sizeof(char));
        strcpy(cmd, non_parsed_commands[command_num]);
        char* parsed_command = calloc(max_command_length, sizeof(char));
        command_parse(cmd, parsed_command, data_set);
        if(strcmp(parsed_command,"\0") != 0){
            execute_command(parsed_command);
        }
        free(parsed_command);
        free(cmd);
        command_num++;
        printf("\n");
    }
    for(int i = 0; i < max_commands; i++){
        free(non_parsed_commands[i]);
    }
    free(non_parsed_commands);
    fclose(data_set);
    return 0;
}