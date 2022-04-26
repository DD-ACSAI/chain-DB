
/*

    This stuff is actually C, not Cxx, but if we are careful it *should* work just fine, right?

*/
#include <stdio.h>
#include <stdlib.h>
#include "libpq-fe.h"
#include "assert.h"
#include <string>
#include <iostream>

#define USER     "postgres"
#define PASSWORD "password"
#define DB       "postgres"

#include "coninfo.h" // static concat magic to define CONNECT_QUERY
#include "queries.h"

int main(int argc, char** argv)
{
    const char* conninfo;
    PGconn* conn;
    PGresult* res = nullptr;
    
    conn = connect(CONNECT_QUERY);

    beginTransaction(conn);
    
    executeQuery("SELECT * FROM \"People\"", res, conn);

    unsigned int ntup = PQntuples(res);
    unsigned int nfield = PQnfields(res);

    printf("we have %u tuples and %u fields", ntup, nfield);

    PQclear(res);
    endTransaction(conn);
    
    PQfinish(conn);

    return 0;
}