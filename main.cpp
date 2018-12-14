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
#include <csignal>

using namespace std;

static bool already_invited = true;
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

void in_progress(int * status_address);

string run_without_pipes(int * in, int * out, int * status_address, vector <string> pipes, map <string, int> mapping);

void run_with_pipes(int * in, int * out, int * status_address, vector <string> pipes);

string my_pwd(string type_of_address, int * status_address) ;

string my_exit(int * status_address) ;

string my_cd(int * in, int * out, int * status_address, vector <string> commands_vector);

string my_exec( const int * in, const int * out, int * status_address, vector <string> commands_vector);

void sigintHandler(int sig_num)
{
    if(already_invited) {
        int status_address = OK;
        string current_place = my_pwd(dir, &status_address);
        // print invite
        cout << endl << current_place << " > ";
        //already_invited = true;
        fflush(stdout);
    }
    // already_invited = false;
}

int main() {

    int status = OK;
    //invite
    cout << "Let's get started!\n";
    //main loop
    signal(SIGINT, sigintHandler);

    in_progress(&status);

    return 0;
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
    bool TimeFlag = false;

    while (*status_address != EXIT) {


        string current_place = my_pwd(dir, status_address);
        // print invite
        cout << current_place << " > ";

        if(!getline(cin,commands_line)) {
            cout << endl;
            cout << "Ok, see u!" << endl;
            exit(EXIT_FAILURE);
        }

        already_invited = false;

        //initialize time command
        struct tms buf;
        times(&buf);
        clock_t t = clock();

        string firstWord = commands_line.substr(0, commands_line.find(" "));

        if (firstWord == "time") {
            // if time is found => put up a flag
            TimeFlag = true;
            commands_line = commands_line.substr(commands_line.find_first_of(" \t") + 1);
            // if command is just "time" exit the loop
            if (commands_line.empty()) {
                continue;
            }
        }

        //split for pipes
        vector <string> pipes = split_for_pipes(commands_line);

        //check for pipes
        if (pipes.size() == 1) {
            // if there are notw pipes
            result = run_without_pipes(&in, &out, status_address, pipes, mapping);
        }

        else {
            // with pipes
            run_with_pipes(&in, &out, status_address, pipes);
        }

        //if time => print time

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
          //  perror("STATUS = EXIT");
            break;
        }

        // update result and flags
        result = "";
        already_invited = true;
        TimeFlag = false;


    }

}

void run_with_pipes(int * in, int * out, int * status_address, vector <string> pipes) {

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
    // chech first and last pipe elements for redirection
    vector <string> first = split(pipes[0]);
    vector <string> last = split(pipes[pipes.size() - 1]);

    //checking for redirection of input
    for (int i = 0; i < first.size(); i++) {

        if(first[i] == "<") {
            //open file if it is necessary
            const char * where = first[i + 1].c_str();
            *in = open(where, O_RDONLY, S_IRWXU);

            if (*in < 0) {
                perror("error redirection of < ");
            }
            //delete of useless args
            first.erase(first.begin() + i , first.begin() + i + 2);

            i--;
            break;
        }

    }
    //checking for redirection of output
    for (int i = 0; i < last.size(); i++) {

        if(last[i] == ">") {
            //open new file if it is necessary
            const char * where = last[i + 1].c_str();
            *out = open(where, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

            if (*out < 0) {
                perror("error redirection of >");
            }
            //delete of useless args
            last.erase(last.begin() + i, last.begin() + i + 2);
            i--;
            break;
        }
    }

    while(pipe_number < pipes.size()) {

        pid = fork();

        if(pid == 0) {

            //redirection in or out of pipeline

            if (*in != STDIN and pipe_number == 0) {
                if(dup2(*in, 0) < 0) {
                    perror("input redirection");
                }
                if(close(*in) < 0) {
                    perror("close");
                }
            }

            if (*out != STDOUT and pipe_number == pipes.size()-1) {
                if(dup2(*out, 1) < 0) {
                    perror("output redirection");
                }
                if(close(*out) < 0) {
                    perror("close");
                }
            }

            //if not last => piping
            if (pipe_number + 1 < pipes.size()) {
                if (dup2(fds[j + 1], 1) < 0) {
                    perror("Not last pipe error!");
                    *status_address = ERROR;
                    exit(EXIT_FAILURE);
                }
            }

            //if not first => piping
            if (pipe_number != 0) {
                if (dup2(fds[j - 2], 0) < 0) {
                    perror("Not first pipe error!");
                    *status_address = ERROR;
                    exit(EXIT_FAILURE);

                }
            }

            //close pipes
            for (i = 0; i <= 2 * pipes.size(); i++) {
                close(fds[i]);
            }

            // prepare for execvp
            vector<string> commands_vector;
            commands_vector = split(pipes[pipe_number]);

            if (pipe_number == 0) {
                commands_vector = first;
            }
            if (pipe_number == pipes.size()-1) {
                commands_vector = last;
            }

            //check for redirection into pipe
            if( pipe_number > 0 && pipe_number < pipes.size() -1 ) {
                for (int i = 0; i < commands_vector.size(); i++)
                    if ((commands_vector[i] == ">") or (commands_vector[i] == "<") ) {
                        cerr << "redirection in pipes is forbidden" << endl;
                        exit(EXIT_FAILURE);
                    }
            }

            vector<const char *> argv;

            // convert string to char * for exec
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

        }
        else if(pid < 0){
            perror("pipe error");
            exit(EXIT_FAILURE);
        }
        //wait(nullptr);
        pipe_number++;
        j+=2;
    }
    //redirect to standart input and output
    *in = STDIN;
    *out = STDOUT;
    // close all pipes
    for(i = 0; i <= 2 * pipes.size(); i++){
        close(fds[i]);
    }
    //wait
    for(i = 0; i <= pipes.size(); i++) {
        wait(&status);
    }
    wait(nullptr);

}

//custom exec
string my_exec(const int * in, const int * out, int * status_address, vector <string> commands_vector) {

    pid_t pid, ppid;;
    int status = 0;
    vector<const char *> argv;

    // convert string to char * array
    for (int i = 0; i < commands_vector.size(); i++)
    {
        argv.push_back(commands_vector[i].c_str());
    }
    // aslo we need nullptr at the end
    argv.push_back(nullptr);

    pid = fork();
    // if it is child:
    if (pid == 0) {

        //check for input redirection
        if (*in != STDIN) {

            if(dup2(*in, 0) < 0) {
                perror("input redirection");
            }
            if(close(*in) < 0) {
                perror("close");
            }
        }
        //check for output redirection
        if (*out != STDOUT) {

            if(dup2(*out, 1) < 0) {
                perror("output redirection");
            }
            if(close(*out) < 0) {
                perror("close");
            }
        }
        // basic exec with converted arguments

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
    // a little bit weird construction
    string nothing = "";
    return  nothing;

}

string run_without_pipes(int * in, int * out, int * status_address, vector <string> pipes, map <string, int> mapping) {

    string result = "";
    vector <string> commands_vector = split(pipes[0]);

    bool input_redir = false;
    bool output_redir = false;
    bool redir_error = false;

    for (int i = 0; i < commands_vector.size(); i++) {

        if(commands_vector[i] == "<") {
            if (input_redir) {
                cerr << "double input rediretion" << endl;
            }
            input_redir = true;
            const char * where = commands_vector[i + 1].c_str();
            *in = open(where, O_RDONLY, S_IRWXU);
            if (*in < 0) {

                perror("error redirection of < ");
            }
            commands_vector.erase(commands_vector.begin() + i, commands_vector.begin() + i + 2);
            i--;

        }
        if(commands_vector[i] == ">") {
            if (output_redir) {
                cerr << "double output rediretion" << endl;
            }
            output_redir = true;
            const char * where = commands_vector[i + 1].c_str();
            *out = open(where, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

            if (*out < 0) {

                perror("error redirection of >");
            }
            commands_vector.erase(commands_vector.begin() + i, commands_vector.begin() + i + 2);
            i--;

        }
    }

    if (!commands_vector.empty() /*&& !redir_error*/) {

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

//custom pwd, exit, cd, time( in main)
string my_pwd(const string type_of_address, int * status_address) {

    char cwd[PATHMAX];

    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        *status_address = OK;
    } else {
        perror("my_pwd(way) error");
        *status_address = ERROR;
    }
    // convert to string
    string str(cwd);
    // return full path
    if (type_of_address == path) {
        return str;
    }
    //return just dir
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

    // find home dir
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;

    // if command = cd
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
    // else
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

void signal_function(int signal) {
    int status_address = OK;
    string start = my_pwd(dir, &status_address);
    cout << start << " > ";

}
