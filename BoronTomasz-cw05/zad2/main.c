#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv){
    if(argc == 4){
        char* receiver = calloc(50, sizeof(char));
        char* subject = calloc(50, sizeof(char));
        char* text = calloc(1000, sizeof(char));
        strcpy(receiver, argv[1]);
        strcpy(subject, argv[2]);
        strcpy(text, argv[3]);
        char* command = calloc(1300, sizeof(char));
        sprintf(command, "echo %s | mail %s -s %s",text, receiver, subject);
        FILE* mail_input = popen(command, "w");
        pclose(mail_input);
        printf("Mail sended to %s\nSubject: %s\nText: %s\n",receiver, subject, text);
        free(command);
        free(receiver);
        free(subject);
        free(text);
    }
    else if(argc == 2){
        if(strcmp(argv[1], "sender") == 0){
            FILE* mails_list = popen("mail -H | sort -k 3", "r");
            char* str = calloc(150, sizeof(char));
            size_t buff_size = 150;
            printf("Mails sorted by sender\n");
            while(getline(&str, &buff_size, mails_list) != EOF){
                printf("%s", str);
            }
            pclose(mails_list);
        }
        else if(strcmp(argv[1], "date") == 0){
            FILE* mails_list = popen("mail -H | sort -k 7 | sort -k 6 | sort -k 5", "r");
            char* str = calloc(150, sizeof(char));
            size_t buff_size = 150;
            printf("Mails sorted by date\n");
            while(getline(&str, &buff_size, mails_list) != EOF){
                printf("%s", str);
            }
            pclose(mails_list);
        }
        else{
            perror("If one argument, it should be sender or date\n");
            exit(1);
        }
    }
    else{
        perror("Bad number of arguments\n");
        exit(1);
    }
    return 0;
}