#pragma once
#include "libpq-fe.h"

inline void exit_program(PGconn* connection)
{
    PQfinish(connection);
    exit(1);
}

/**
 * Predicate for connection failure.
 * 
 * \param status    A Connection Status.
 * \return          True if the connection failed, False if the connection is successful.
 */
inline bool statusFailed(ExecStatusType status)
{
    return status == PGRES_EMPTY_QUERY || status == PGRES_BAD_RESPONSE || status == PGRES_NONFATAL_ERROR || status == PGRES_FATAL_ERROR;
}

namespace query
{


/**
 * Begins a SQL transaction and checks for possible errors.
 *
 * \param connection    Pointer to a Database Connection.
 */
void beginTransaction(PGconn*& connection)
{
    PGresult* res = PQexec(connection, "BEGIN");

    if (statusFailed(PQresultStatus(res)) || res == nullptr)
    {
        //Preprocessor defines in order to reduce code size.
#ifdef _DEBUG

        std::cerr << "BEGIN command failed: " << PQerrorMessage(connection) << std::endl;

#endif // DEBUG
        PQclear(res);
        exit_program(connection);
    }
    PQclear(res);
}

/**
 * Closes and commits an SQL transaction, checking for possible errors.
 *
 * \param connection    Pointer to a Database Connection.
 */
void endTransaction(PGconn*& connection)
{
    PGresult* res = PQexec(connection, "END");
    if (statusFailed(PQresultStatus(res)) || res == nullptr)
    {
#ifdef _DEBUG

        std::cerr << "BEGIN command failed: " << PQerrorMessage(connection) << std::endl;

#endif
        PQclear(res);
        exit_program(connection);
    }
    PQclear(res);
}


/**
 * Executes a SQL query providing some safety checks for connection errors.
 * 
 * \brief           Executes a SQL query
 * \param query     A CString containing an SQL query
 * \param res           Pointer to a PGresult, results of the query will be dumped here
 * \param connection    Pointer to a Database Connection.
 */
void executeQuery(const char* query, PGresult*& res, PGconn*& connection)
{
    res = PQexec(connection, query);

    auto status = PQresultStatus(res);

    if (statusFailed(status) || res == nullptr)
    {
        std::cerr << "Query failed: " << PQerrorMessage(connection) << std::endl;

#ifdef _DEBUG
        //ADDITIONAL INFORMATION IF DEBUG IS ENABLED
        std::cerr << "Result Status: " << PQresStatus(status) << std::endl;
        std::cerr << "Query was: " << query << std::endl;
#endif // DEBUG

        PQclear(res);
        exit_program(connection);
    }
}

/**
 *
 *
 *  Provides a convenient way to execute a single query in a transaction block.
 *
 * \brief Executes a query wrapped in a transaction.
 * \param query         CString with the SQL query to be executed
 * \param res           Pointer to a PGresult, results of the query will be dumped here
 * \param connection    Pointer to a Database Connection.
 */
void atomicQuery(const char* query, PGresult*& res, PGconn*& connection)
{
    beginTransaction(connection);
    executeQuery(query, res, connection);
    endTransaction(connection);
}


/**
 * Connects to a PostgreSQL with error-checking.
 * 
 * \param conninfo const CString storing all the parameters of the connection.
 * \return      Returns a pointer to PGconn, a struct representing a connection to the DB.
 */
PGconn* connect(const char* const conninfo)
{
    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK)
    {
#ifdef _DEBUG

        std::cerr << PQerrorMessage(conn) << std::endl;

#endif _DEBUG
        exit_program(conn);
    }

#ifdef _DEBUG
    std::clog << "Connection Successful!, printing connection information as follows: " << std::endl
        << "DB name: " << PQdb(conn) << std::endl
        << "Username: " << PQuser(conn) << std::endl
        << "Password: " << PQpass(conn) << std::endl
        << "Host Address: " << PQhostaddr(conn) << " at port " << PQport(conn) << std::endl
        << "Connection Options: " << PQoptions(conn) << std::endl
        << "Connection Status: " << PQstatus(conn) << std::endl;
#endif // _DEBUG

    return conn;
}


}