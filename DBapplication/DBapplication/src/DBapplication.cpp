#pragma once
/*

    This stuff is actually C, not Cxx, but if we are careful it *should* work just fine, right?

*/
#include <stdio.h>
#include <stdlib.h>
#include "libpq-fe.h"
#include "assert.h"
#include <string>
#include <iostream>
#include <sstream>
#include <memory>

// Yes, you heard that right, no crosscompat!
#ifdef _WIN32
    #include <Windows.h>
#else
    #error "No compatibility, sorry!"
#endif

#define USER     "postgres"
#define PASSWORD "password"
#define DB       "postgres"

#include "defines/coninfo.h" // static concat magic to define CONNECT_QUERY
#include "DButils/queries.h"
#include "DButils/CLprinter.h"
#include "manager/dbhierarchy/Dbnode.h"
#include "manager/DBmanager.h"

int main(int argc, char** argv)
{
    auto conn = query::connect(CONNECT_QUERY);


    ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    DBmanager man(conn);
    PQfinish(conn);
    

    
    int x = 0;
    return 0;
}