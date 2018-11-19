//
// Created by agavrilenko on 20.11.18.
//

#ifndef TRY_2017_STRING_FUNCTIONS_H
#define TRY_2017_STRING_FUNCTIONS_H

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
