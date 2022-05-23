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
    std::ios_base::sync_with_stdio(false);
    ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    CLprinter printUtil;

    printUtil.printHeader();

    bool choice;
    std::cout << "Do you want to automatically input your credentials?\n (1) Yes\n (0) No \n: ";
    choice = _getch() - '0';
    std::system("CLS");

    std::string connect_query;
    connect_query.reserve(128);

    if (!choice)
    {
        std::string username, password, dbname;

        std::cout << "Insert DB Name to connect: ";
        std::cin >> dbname;
        std::cout << std::endl;

        std::cout << "Insert Username to connect: ";
        std::cin >> username;
        std::cout << std::endl;

        std::cout << "Insert Password to connect: ";
        std::cin >> password;
        std::cout << std::endl;

        connect_query = "dbname = " + dbname + " password = " + password + " user = " + username;
    }
    else
        connect_query = CONNECT_QUERY;

    std::system("CLS");
    auto conn = query::connect(connect_query.c_str());

    DBmanager man(conn);
    PGresult* res = nullptr;


    for (;;)
    {
        auto c =_getch();

        c = parseKey(c);

        if (c == 83)
            break;
        man.handleKeyboard(c);
    }

    return 0;
}