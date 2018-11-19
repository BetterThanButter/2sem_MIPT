//
// Created by agavrilenko on 20.11.18.
//

#ifndef TRY_2017_MY_FUNCTIONS_H
#define TRY_2017_MY_FUNCTIONS_H



string my_pwd(string type_of_address, int * status_address) {

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
    string result = "OK, see u !";
    return result;
}

void my_cd(int * status_address, vector <string> commands_vector) {

    if (commands_vector.size() == 1) {
        if(chdir(homedir) == 0) {
            *status_address = OK;
        }
        else {
            *status_address = ERROR;
        }

    }

//    char cwd[PATHMAX];
//
//    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
//        *status_address = OK;
//    } else {
//        perror("my_pwd(way) error");
//        *status_address = ERROR;
//    }

    if (commands_vector.size() == 2) {

        if (commands_vector[1] == "..") {
//
//            unsigned long pos = str.find_last_of('/');
//            cwd.erase(str.begin(), str.begin()+pos+1);
        } else {
            if (chdir(commands_vector[1].c_str()) == 0) {
                *status_address = OK;
            } else {
                *status_address = ERROR;
            }
        }
    }
}

void my_exec(int * status_address, vector <string> commands_vector) {
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
        if (execvp(argv[0], (char * const *)&argv[0]) == -1) {
            *status_address = ERROR;
        }
        exit(EXIT_FAILURE);
    }
    else {
        if (pid  > 0) {
            while (!WIFEXITED(status) && !WIFSIGNALED(status)) {
                ppid = waitpid(pid, &status, WUNTRACED);
            }
        }
        else {
            *status_address = ERROR;
        }
    }

}


#endif //TRY_2017_MY_FUNCTIONS_H
