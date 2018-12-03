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

    for (int i = 0; i < splitted.size(); i++) {

        unsigned long full_mask = string::npos;
        unsigned long solo_mask = string::npos;

        if (((full_mask = splitted[i].find_first_of('*')) != string::npos) or ((solo_mask = splitted[i].find_first_of('?')) != string::npos)) {

            string path = splitted[i];
            splitted.erase(splitted.begin() + i, splitted.begin() + i + 1);

            glob_t glob_result;
            glob(path.c_str(),GLOB_TILDE,NULL,&glob_result);
            vector<string> ret;
            for(unsigned int i=0;i<glob_result.gl_pathc;++i){
                splitted.push_back(string(glob_result.gl_pathv[i]));
            }
            globfree(&glob_result);
//            string full_path;
//            int pos;
//            struct stat object;
//            struct dirent * dir;
//            // пути
//            if ((pos = splitted[i].find_last_of('/', min(full_mask, solo_mask))) != string::npos) {
//                full_path = splitted[i].substr(0, pos);
//            }
//
//            else {
//                full_path = ".";
//            }
//
//            // занесем инфу
//            if(stat(full_path.c_str(), &object) == -1) {
//                perror(path.c_str());
//                exit(EXIT_FAILURE);
//            }
//            //если папка то выполняем
//            if (S_ISDIR(object.st_mode)) {
//
//                DIR * our_dir= opendir(full_path.c_str());
//
//                if (our_dir == nullptr) {
//                    perror("ошибка в регулярном выражении");
//                    exit(EXIT_FAILURE);
//                }
//                //обход папки
//                struct dirent *ent;
//                while ((ent = readdir (our_dir)) != nullptr) {
//
//                    string name;
//                    if (full_path != ".") {
//                        name = full_path + "/" + ent->d_name;
//                    }
//                    else {
//                        name = ent->d_name;
//                    }
//                    if (fnmatch(splitted[i].c_str(), name.c_str(), FNM_PATHNAME) == 0) {
//                        splitted.insert(splitted.begin() + i + 1, name);
//                    }
//                }
//                closedir(our_dir);
//            }
//

        }
    }

    return splitted;
}

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
