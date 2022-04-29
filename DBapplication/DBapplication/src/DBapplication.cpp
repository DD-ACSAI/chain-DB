
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
#include "manager/dbhierarchy/Dbnode.h"

int main(int argc, char** argv)
{
    PGconn* conn;
    PGresult* res = nullptr;

    ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    
    conn = query::connect(CONNECT_QUERY);
    CLprinter prnt;

    Dbnode<DBNODETYPE::ROOT, DBNODETYPE::SCHEMA> root("Root");
    root.addChildren( std::make_unique<Dbnode<DBNODETYPE::SCHEMA, DBNODETYPE::TABLE>>("A") );
    root.addChildren(std::make_unique<Dbnode<DBNODETYPE::SCHEMA, DBNODETYPE::TABLE>>("A"));
    root.addChildren(std::make_unique<Dbnode<DBNODETYPE::SCHEMA, DBNODETYPE::TABLE>>("A"));

    auto const& c = root.getChildren();

    for (auto const& p : c)
    {
        std::clog << p->getName();
        p->addChildren(std::make_unique<Dbnode<DBNODETYPE::TABLE, DBNODETYPE::NONE>>("B"));
    }

    root.printRecursive();

    return 0;
}