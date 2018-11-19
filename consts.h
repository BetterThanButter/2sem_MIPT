//
// Created by agavrilenko on 20.11.18.
//

#ifndef TRY_2017_CONSTS_H
#define TRY_2017_CONSTS_H


const int EXIT =  777;
const int OK =  1;
const int ERROR =  666;
const int PATHMAX =  1000;

const int mine_pwd =  101;
const int mine_cd =  102;
const int mine_time =  103;
const int mine_exit =  104;

const string path = "path";
const string dir = "dir";

struct passwd *pw = getpwuid(getuid());
const char *homedir = pw->pw_dir;

#endif //TRY_2017_CONSTS_H
