

#include <stdio.h>
#include <stdlib.h>


#pragma comment(lib, "libpq.lib")
#pragma comment(lib, "libintl.lib")

/*

    This stuff is actually C, not Cxx, but if we are careful it *should* work just fine, right?

*/
#include "libpq-fe.h"
#include "assert.h"

void exit_program(PGconn* connection)
{
    PQfinish(connection);
    exit(1);
}

int main(int argc, char** argv)
{
    const char* conninfo = "dbname = postgres";
    PGconn* connection;

    assert(connection = PQconnectdb(conninfo), "Connection Error!!");


    return 0;
}

