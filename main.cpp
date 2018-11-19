#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <map>
#include <sys/types.h>
#include <sys/wait.h>
#include <utility>
#include <cstring>

using namespace std;
//поиск домашней папки


#include "consts.h"
#include "string_functions.h"
#include "my_functions.h"



void in_progress(int * status_address);

string run_without_pipes(int * status_address, vector <string> pipes, map <string, int> mapping);

void get_started(int * status_address);

void run_with_pipes(int * status_address, vector <string> pipes);



int main() {

    int status = OK;

    get_started(&status);

    in_progress(&status);

    return 0;
}

void get_started(int * status_address) {

    cout << "Let's get started!\n";

    string start = my_pwd(dir, status_address);

    cout << start << " > ";

}

void in_progress(int * status_address) {

    map <string, int> mapping;

    mapping["pwd"]  = mine_pwd;
    mapping["cd"]    = mine_cd;
    mapping["exit"] = mine_exit;
    mapping["time"]  = mine_time;


    string commands_line;
    string result;

    while (*status_address != EXIT) {

        getline(cin, commands_line);
        vector <string> pipes = split_for_pipes(commands_line);

        if (pipes.size() == 1) {
            result = run_without_pipes(status_address, pipes, mapping);
        }

        else {
            run_with_pipes(status_address, pipes);
        }


        if (*status_address == EXIT) {
            break;
        }


        string current_place = my_pwd(dir, status_address);
        cout << current_place << " > " << result;
        result = "";


    }

}

void run_with_pipes(int * status_address, vector <string> pipes) {

    int status;
    int i = 0;
    pid_t pid;

    int pipe_number  = 0;
    int gates_counter = 0;
    int fds[2*pipes.size()];

    for(i = 0; i < (pipes.size()); i++){
        if(pipe(fds + i*2) < 0) {
            perror("At the beginning error");
            *status_address = ERROR;
            exit(EXIT_FAILURE);
        }
    }

    int j = 0;
    while(pipe_number <= pipes.size()) {

        pid = fork();

        if(pid == 0) {

            // делаем перенаправление входа если не первый pipe
            // аналгично вызод если не последний

            if(pipe_number+1 < pipes.size()){
                if(dup2(fds[gates_counter + 1], 1) < 0){
                    *status_address = ERROR;
                    perror("not end dup troubles");
                }
            }

            if(pipe_number != 0 ){
                if(dup2(fds[gates_counter-2], 0) < 0){
                    *status_address = ERROR;
                    perror(" not start problemes");
                }
            }

            //закроем
            for(i = 0; i < 2*pipes.size(); i++){
                close(fds[i]);
            }
            // так как у нас были вектора...
            vector <string> commands_vector = split(pipes[pipe_number]);
            vector<const char *> argv;

            // преобразуем стринг в массив указателей
            for (int i = 0; i < commands_vector.size(); i++)
            {
                argv.push_back(commands_vector[i].c_str());
            }
            // добавим в конец нулевой указатель
            argv.push_back(nullptr);

            if (execvp(argv[0], (char * const *)&argv[0]) == -1) {
                *status_address = ERROR;

            }
            else {perror("Not ok1 \n");
            }

        } else if(pid < 0){
            perror("error");
            *status_address = ERROR;

        }

        pipe_number++;
        gates_counter+=2;
    }

    // close all pipes
    for(i = 0; i < 2 * pipes.size(); i++){
        close(fds[i]);
    }
    //wait
    for(i = 0; i < pipes.size(); i++) {
        wait(&status);
    }

}


string run_without_pipes(int * status_address, vector <string> pipes, map <string, int> mapping) {
    string result;
    vector <string> commands_vector = split(pipes[0]);

    if (!commands_vector.empty()) {

        switch (mapping[commands_vector[0]]) {

            case mine_pwd:
                result = my_pwd(path, status_address);
                break;

            case mine_cd:
                my_cd(status_address, commands_vector);
                break;

            case mine_exit:
                result = my_exit(status_address);
                cout << result;
                break;

            case mine_time:
                break;

            default:
                my_exec(status_address, commands_vector);

        }
    }
    return result;

}