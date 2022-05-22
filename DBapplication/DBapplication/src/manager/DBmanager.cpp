#include "DBmanager.h"
#include "queries/Procedure.h"
DBcontext DBmanager::context = DBcontext::DIR_TREE;
std::unordered_set<std::string> const DBmanager::SQLtokens = { "SELECT", "FROM", "WHERE", "ORDER BY", "GROUP BY", "SUM", "AVG", "DROP", "SCHEMA", "CASCADE", "CALL", "AS", "JOIN", "ON"};
