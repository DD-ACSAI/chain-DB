
/*

    This stuff is actually C, not Cxx, but if we are careful it *should* work just fine, right?

*/
#include <stdio.h>
#include <stdlib.h>
#include "libpq-fe.h"
#include "assert.h"
#include <string>
#include <iostream>

// Yes, you heard that right, no crosscompat!
#include <windows.h>

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
    
    conn = query::connect(CONNECT_QUERY);

    query::beginTransaction(conn);
    query::executeQuery("SELECT * FROM \"People\" ORDER BY \"Age\"", res, conn);

    CLprinter prnt;

    prnt.printTable(res, TRUE);


    PQclear(res);
    query::endTransaction(conn);
    
    PQfinish(conn);

    return 0;
}