
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

int main(int argc, char** argv)
{
    PGconn* conn;
    PGresult* res = nullptr;

    ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    
    conn = query::connect(CONNECT_QUERY);
    CLprinter prnt;

    query::atomicQuery("SELECT * FROM information_schema.tables WHERE table_schema = \'public\'", res, conn);
    prnt.printTable(res);

    PQclear(res);
    std::getchar();

    query::beginTransaction(conn);
    query::executeQuery("SELECT * FROM \"People\" ORDER BY \"Name\"", res, conn);
    prnt.printTable(res);

    std::getchar();

    PQclear(res);
    query::beginTransaction(conn);
    query::executeQuery("SELECT * FROM \"People\" ORDER BY \"Name\"", res, conn);
    prnt.printTable(res);
    

    PQclear(res);
    query::endTransaction(conn);
    
    PQfinish(conn);


    return 0;
}