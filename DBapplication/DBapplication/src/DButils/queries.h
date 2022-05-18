#pragma once
#include "libpq-fe.h"
#include <stdint.h>

#include <memory>
#include <string>
#include <stdexcept>

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
static void beginTransaction(PGconn*& connection)
{
    PGresult* res = PQexec(connection, "BEGIN");

    if (statusFailed(PQresultStatus(res)) || res == nullptr)
    {
        //Preprocessor defines in order to reduce code size.
#ifdef _DEBUG

        std::cerr << "BEGIN command failed: " << PQerrorMessage(connection) << std::endl;

#endif // DEBUG
        PQclear(res);
    }
    PQclear(res);
}

/**
 * Closes and commits an SQL transaction, checking for possible errors.
 *
 * \param connection    Pointer to a Database Connection.
 */
static void endTransaction(PGconn*& connection)
{
    PGresult* res = PQexec(connection, "END");
    if (statusFailed(PQresultStatus(res)) || res == nullptr)
    {
#ifdef _DEBUG

        std::cerr << "BEGIN command failed: " << PQerrorMessage(connection) << std::endl;

#endif
        PQclear(res);
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
static void executeQuery(const char* query, PGresult*& res, PGconn*& connection)
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
static void atomicQuery(const char* query, PGresult*& res, PGconn*& connection)
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
static PGconn* connect(const char* const conninfo)
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

/**
 * Consumes a PGresult* and wraps it, providing convenient access to a number of it's 
 * contained attributes and automatically freeing the resource once the struct goes out of
 * scope.
 */
struct queryRes
{
public:
    uint64_t fields;
    uint64_t rows;
    bool successful;
    PGresult* result;

    explicit queryRes(PGresult*& res) : result(res)
    {
        fields = PQnfields(res);
        rows = PQntuples(res);
        successful = !(statusFailed(PQresultStatus(res)) || res == nullptr);
    }

    ~queryRes()
    {
        PQclear(result);
    }

    explicit queryRes(queryRes const& other) = default;
    queryRes& operator=(queryRes const&) = default;
};

/**
 * Formats a string in a "pythonic" way.
 * 
 * Taken from: https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
 * 
 * \param format String to format
 * \param ...args Variadic pack of arguments
 * \return A formatted string
 */
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

}