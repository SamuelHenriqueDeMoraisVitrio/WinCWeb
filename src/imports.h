#ifndef PRIVATE_CWEB_HTTP_IMPORTS_H
#define PRIVATE_CWEB_HTTP_IMPORTS_H

//#include <sys/wait.h>//NOT
#include <stdio.h>
//#include <sys/stat.h>//NOT
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/socket.h>//NOT
#include <signal.h>//NOT~
//#include <sys/resource.h>//NOT

//#include <arpa/inet.h>//NOT
//#include <unistd.h>//NOT
#include <winsock2.h>   // Substitui sys/socket.h e arpa/inet.h
#include <windows.h>    // Substitui unistd.h, sys/resource.h e sys/wait.h
#include <io.h>         // Substitui algumas funções de sys/stat.h

//Windows
/*
*/

#endif //PRIVATE_CWEB_HTTP_IMPORTS_H
