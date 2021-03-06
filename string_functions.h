//
// Created by agavrilenko on 20.11.18.
//

#ifndef TRY_2017_STRING_FUNCTIONS_H
#define TRY_2017_STRING_FUNCTIONS_H

#include <fcntl.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <sstream>
#include <glob.h>

vector<string> split(const string& commands) {

    vector<string> splitted;
    typedef string::size_type string_size;
    string_size i = 0;
    // spliting particular coomand to parts
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
    // checking special symbols
    for (int i = 0; i < splitted.size(); i++) {

        unsigned long full_mask = string::npos;
        unsigned long solo_mask = string::npos;

        // check for special symbols

        if (((full_mask = splitted[i].find_first_of('*')) != string::npos) or ((solo_mask = splitted[i].find_first_of('?')) != string::npos)) {

            string path = splitted[i];
            splitted.erase(splitted.begin() + i, splitted.begin() + i + 1);

            // seting glob function
            glob_t glob_result;
            memset(&glob_result, 0, sizeof(glob_result));


            int return_value = glob(path.c_str(), GLOB_TILDE, NULL, &glob_result);
            // check if glob return nothing
            if(return_value != 0) {
                globfree(&glob_result);
                cout << "невозможно получить доступ к: '" <<  path << "' Нет такого файла или каталога\n";
                vector <string> nothing ;
                return nothing;

            }

            // push our new filenames in args vector

            for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
                splitted.push_back(string(glob_result.gl_pathv[i]));
            }

            // clean glob
            globfree(&glob_result);

        }
    }

    return splitted;
}

// just basic function of splitiong string for vector <string> by '|'
vector <string> split_for_pipes(const string& commands) {

    string line = commands;
    vector <string> pipes_vector;
    string subs;

    int delim = line.find_first_of('|');
    while(delim != std::string::npos)
    {
        subs = line.substr(0, delim);
        pipes_vector.push_back(subs);
        line.erase(0, delim + 1);
        delim = line.find_first_of('|');

    }
    pipes_vector.push_back(line);

    return pipes_vector;

}
#endif //TRY_2017_STRING_FUNCTIONS_H
