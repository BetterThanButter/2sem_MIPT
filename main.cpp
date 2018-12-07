#include <iostream>
#include <fstream>
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
#include <sys/times.h>
#include <cmath>
#include <utility>
#include <cstring>
#include <iomanip>

using namespace std;

const int KEY_UP = 72;
const int EXIT =  777;
const int OK =  1;
const int ERROR =  666;
const int PATHMAX =  1000;

const int STDIN = 0;
const int STDOUT = 1;

const int mine_pwd =  101;
const int mine_cd =  102;
const int mine_time =  103;
const int mine_exit =  104;

const string path = "path";
const string dir = "dir";

#include "string_functions.h"


void get_started(int * status_address);

void in_progress(int * status_address);

string run_without_pipes(int * in, int * out, int * status_address, vector <string> pipes, map <string, int> mapping);

void run_with_pipes(int * status_address, vector <string> pipes);

string my_pwd(string type_of_address, int * status_address) ;

string my_exit(int * status_address) ;

string my_cd(int * in, int * out, int * status_address, vector <string> commands_vector);

string my_exec( const int * in, const int * out, int * status_address, vector <string> commands_vector);

int myshell_time(int in, int out, std::vector<std::string>& arguments);
//void my_time(int * in, int * out, int * status_address, vector <string> commands_vector);


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
    string history;
    mapping["pwd"]  = mine_pwd;
    mapping["cd"]   = mine_cd;
    mapping["exit"] = mine_exit;
    mapping["time"] = mine_time;

    int in = 0;
    int out = 1;
    string commands_line;
    string result;


    while (*status_address != EXIT) {


        if(!getline(cin,commands_line)) {
            cout << endl;
            exit(EXIT_FAILURE);
        }


        bool TimeFlag = false;
        struct tms buf;
        times(&buf);
        clock_t t = clock();

        string firstWord = commands_line.substr(0, commands_line.find(" "));

        if (firstWord == "time") {

            TimeFlag = true;

            commands_line=commands_line.substr(commands_line.find_first_of(" \t")+1);
            if (commands_line.empty()) {
                continue;
            }
        }

        vector <string> pipes = split_for_pipes(commands_line);

        if (pipes.size() == 1) {
            result = run_without_pipes(&in, &out, status_address, pipes, mapping);
        }

        else {
            run_with_pipes(status_address, pipes);
        }

        if (TimeFlag) {
            t = clock() - t;
            times(&buf);

            double rm, rs, um,us, sm, ss;
            rm = floor(((double) t) * 1000 / CLOCKS_PER_SEC / 60);
            rs = ((double) t) * 1000 / CLOCKS_PER_SEC - rm * 60;
            um = floor((double) (buf.tms_utime + buf.tms_cutime) * 1000 / CLOCKS_PER_SEC / 60);
            us = (double) (buf.tms_utime + buf.tms_cutime) * 1000 / CLOCKS_PER_SEC - um * 60;
            sm = floor((double) (buf.tms_stime + buf.tms_cstime) * 1000 / CLOCKS_PER_SEC / 60);
            ss = (double) (buf.tms_stime + buf.tms_cstime) * 1000 / CLOCKS_PER_SEC - sm * 60;

            cout << "real "  << rm << "m"  << rs << "s" << endl;
            cout << "user "  << um << "m"  << us << "s" << endl;
            cout << "sys  "  << sm << "m"  << ss << "s" << endl;

        }


        if (*status_address == EXIT) {
            // perror("STATUS = EXIT");
            break;
        }
        string current_place = my_pwd(dir, status_address);

        cout << current_place << " > ";
        result = "";


    }

}

void run_with_pipes(int * status_address, vector <string> pipes) {

    int status;
    int i = 0;
    pid_t pid;
    int pipe_number = 0;;

    int fds[2*pipes.size()];

    for(i = 0; i < (pipes.size()); i++){
        if(pipe(fds + i*2) < 0) {
            perror("can't pipe");
            exit(EXIT_FAILURE);
        }
    }


    int j = 0;
    while(pipe_number <= pipes.size()) {

        pid = fork();

        if(pid == 0) {

            // делаем перенаправление входа если не первый pipe
            // аналгично вызод если не последний

            if (pipe_number + 1 < pipes.size()) {
                if (dup2(fds[j + 1], 1) < 0) {
                    perror("Not last pipe error!");
                    *status_address = ERROR;
                    exit(EXIT_FAILURE);
                }
            }

            if (pipe_number != 0) {
                if (dup2(fds[j - 2], 0) < 0) {
                    perror("Not first pipe error!");
                    *status_address = ERROR;
                    exit(EXIT_FAILURE);

                }
            }

            //закроем
            for (i = 0; i <= 2 * pipes.size(); i++) {
                close(fds[i]);
            }

            vector<string> commands_vector = split(pipes[pipe_number]);
            vector<const char *> argv;

            // преобразуем стринг в массив указателей
            for (int i = 0; i < commands_vector.size(); i++) {
                argv.push_back(commands_vector[i].c_str());
            }
            // добавим в конец нулевой указатель
            argv.push_back(nullptr);

            if (execvp(argv[0], (char *const *) &argv[0]) == -1) {
                *status_address = ERROR;

            } else {
                perror("Not ok1 \n");
            }



        } else if(pid < 0){
            perror("pipe error");
            exit(EXIT_FAILURE);
        }
        //wait(nullptr);
        pipe_number++;
        j+=2;
    }

    // close all pipes
    for(i = 0; i <= 2 * pipes.size(); i++){
        close(fds[i]);
    }
    //wait
    for(i = 0; i < pipes.size(); i++) {
        wait(&status);
    }
    wait(nullptr);

}

string my_exec(const int * in, const int * out, int * status_address, vector <string> commands_vector) {
    pid_t pid, ppid;;
    int status = 0;
    vector<const char *> argv;

    // преобразуем стринг в массив указателей
    for (int i = 0; i < commands_vector.size(); i++)
    {
        argv.push_back(commands_vector[i].c_str());
    }
    // добавим в конец нулевой указатель
    argv.push_back(nullptr);

    pid = fork();
    // если дочерний
    if (pid == 0) {

        if (*in != STDIN) {

            if(dup2(*in, 0) < 0) {
                perror("input redirection");
            }
            if(close(*in) < 0) {
                perror("close");
            }
        }

        if (*out != STDOUT) {

            if(dup2(*out, 1) < 0) {
                perror("output redirection");
            }
            if(close(*out) < 0) {
                perror("close");
            }
        }
        // выполняем после перенаправления

        if (execvp(argv[0], (char * const *)&argv[0]) == -1) {
            perror("execvp error");
            *status_address = ERROR;
            exit(EXIT_FAILURE);
        }


    }

    else {
        if (pid  > 0) {
            while (!WIFEXITED(status) && !WIFSIGNALED(status)) {
                ppid = waitpid(pid, &status, WUNTRACED);
            }
        }
        else {
            *status_address = ERROR;
            perror("wait error");
        }
    }
    wait(nullptr);

    string nothing = "";
    return  nothing;

}

string run_without_pipes(int * in, int * out, int * status_address, vector <string> pipes, map <string, int> mapping) {
    string result;
    vector <string> commands_vector = split(pipes[0]);

    for (int i = 0; i < commands_vector.size(); i++) {

        if(commands_vector[i] == "<") {

            const char * where = commands_vector[i + 1].c_str();
            *in = open(where, O_RDONLY, S_IRWXU);
            if (*in < 0) {

                perror("error redirection of < ");
            }
            commands_vector.erase(commands_vector.begin() + i, commands_vector.begin() + i + 2);
            i--;
        }
        if(commands_vector[i] == ">") {

            const char * where = commands_vector[i + 1].c_str();
            *out = open(where, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

            if (*out < 0) {

                perror("error redirection of >");
            }
            commands_vector.erase(commands_vector.begin() + i, commands_vector.begin() + i + 2);
            i--;
        }
    }

    if (!commands_vector.empty()) {

        switch (mapping[commands_vector[0]]) {

            case mine_pwd:
                result = my_pwd(path, status_address);
                cout << result << endl;
                break;

            case mine_cd:
                result = my_cd(in, out, status_address, commands_vector);
                break;

            case mine_exit:
                result = my_exit(status_address);
                cout << result;
                break;

            default:
                result = my_exec(in, out, status_address, commands_vector);
                *in = STDIN;
                *out = STDOUT;

        }
    }
    return result;

}

string my_pwd(const string type_of_address, int * status_address) {

    char cwd[PATHMAX];

    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        *status_address = OK;
    } else {
        perror("my_pwd(way) error");
        *status_address = ERROR;
    }

    string str(cwd);

    if (type_of_address == path) {
        return str;
    }

    if (type_of_address == dir) {
        unsigned long pos = str.find_last_of('/');
        str.erase(str.begin(), str.begin()+pos+1);
    }
    return str;

}

string my_exit(int * status_address) {
    *status_address = EXIT;
    string result = "OK, see u !\n";
    return result;
}

string my_cd(int * in, int * out, int * status_address, vector <string> commands_vector) {


    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;


    if (commands_vector.size() == 1) {
        if(chdir(homedir) == 0) {
            *status_address = OK;

        }
        else {
            *status_address = ERROR;
            perror("cd error");
        }

    }
    string result;

    if (commands_vector.size() == 2) {

        if (commands_vector[1] == "-") {


        } else {
            if (chdir(commands_vector[1].c_str()) == 0) {
                *status_address = OK;
            } else {
                *status_address = ERROR;
            }
        }
    }

    string nothing = "";
    return nothing;
}

