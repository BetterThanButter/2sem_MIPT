#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdio.h>

using namespace std;


const int EXIT =  777;
const int OK =  1;
const int ERROR =  666;
const int PATHMAX =  1000;

const string path = "path";
const string dir = "dir";

//поиск домашней папки
struct passwd *pw = getpwuid(getuid());
const char *homedir = pw->pw_dir;


vector<string> split(const string& commands);
string my_pwd(string s, int * status_address);
void get_started(int * status_address);
void in_progress(int * status_address);


int main() {

    int status = OK;

    get_started(&status);

    in_progress(&status);

    return 0;
}

void get_started(int * status_address) {

    cout << "Let's get started!\n";
    string type_of_way = "path";

    string start = my_pwd(type_of_way, status_address);
    cout << start << " > ";

}

void in_progress(int * status_address) {

    string commands_line;
    string result;

    vector<string> commands_vector = split(commands_line);

    while (*status_address != EXIT) {
        getline(cin, commands_line);

        commands_vector = split(commands_line);

        if (commands_vector.size() == 1) {

            if (commands_vector[0] == "pwd") {
                result = my_pwd("directory", status_address);
            }

            if (commands_vector[0] == "cd") {
                if(chdir(homedir) == 0) {
                    *status_address = OK;
                }
                else
                    *status_address = ERROR;
            }
            if (commands_vector[0] == "exit") {
                *status_address = EXIT;
                break;

            }

        }

        if ((commands_vector.size() == 2) and (commands_vector[0] == "cd")) {
            if(chdir(commands_vector[1].c_str()) == 0) {
                *status_address = OK;
            }
            else
                *status_address = ERROR;
        }

        if (commands_vector.size() > 2) {
            cout << "I can't do this :(\n";
        }

        string current_place = my_pwd(path, status_address);
        cout << current_place << " > " << result;


    }
}

string my_pwd(string type_of_address, int * status_address) {

    char cwd[PATHMAX];

    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        *status_address = OK;
    } else {
        perror("my_pwd(way) error");
        *status_address = ERROR;
    }

    // char * to string
    string str(cwd);

    if (type_of_address == "way") {
        return str;
    }

    if (type_of_address == "directory") {
        unsigned long pos = str.find_last_of('/');
        str.erase(str.begin(), str.begin()+pos+1);
    }

    return str;

}

vector<string> split(const string& commands) {
    vector<string> splitted;
    typedef string::size_type string_size;
    string_size i = 0;

    while (i != commands.size()) {

        while (i != commands.size() && isspace(commands[i]))
            ++i;

        string_size j = i;

        while (j != commands.size() && !isspace(commands[j]))
            j++;

        if (i != j) {

            splitted.push_back(commands.substr(i, j - i));
            i = j;
        }
    }
    return splitted;
}