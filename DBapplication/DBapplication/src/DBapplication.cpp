#pragma once
#define NOMINMAX
/*

    This stuff is actually C, not Cxx, but if we are careful it *should* work just fine, right?

*/
#include <stdio.h>
#include <conio.h>
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

#include "defines/DBkeys.h"
#include "defines/coninfo.h" // static concat magic to define CONNECT_QUERY
#include "DButils/queries.h"
#include "DButils/CLprinter.h"
#include "manager/dbhierarchy/Dbnode.h"
#include "manager/DBmanager.h"

int main(int argc, char** argv)
{

    auto conn = query::connect(CONNECT_QUERY);

    DBmanager man(conn);
    PGresult* res = nullptr;

    ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    /*
    query::atomicQuery("SELECT * FROM \"public\".\"People\"", res, conn);
    CLprinter pr;
    pr.printTable(res);
    */

    for (;;)
    {
        auto c =_getch();

        c = parseKey(c);

        if (c == 27)
            break;

        std::cout << c << std::endl;
        man.handleKeyboard(c);
    }

    PQfinish(conn);
    
    return 0;
}