#pragma once
#include "libpq-fe.h"
#include <stdint.h>

#include <unordered_set>
#include <memory>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "../defines/clicolors.h"

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
static bool beginTransaction(PGconn*& connection)
{
    PGresult* res = PQexec(connection, "BEGIN");

    if (statusFailed(PQresultStatus(res)) || res == nullptr)
    {
        //Preprocessor defines in order to reduce code size.
#ifdef _DEBUG

        std::cerr << "BEGIN command failed: " << PQerrorMessage(connection) << "\n";

#endif // DEBUG
        PQclear(res);
        return false;
    }
    PQclear(res);
    return true;
}

/**
 * Closes and commits an SQL transaction, checking for possible errors.
 *
 * \param connection    Pointer to a Database Connection.
 */
static bool endTransaction(PGconn*& connection)
{
    PGresult* res = PQexec(connection, "END");
    if (statusFailed(PQresultStatus(res)) || res == nullptr)
    {
#ifdef _DEBUG

        std::cerr << "BEGIN command failed: " << PQerrorMessage(connection) << "\n";

#endif
        PQclear(res);
        return false;
    }
    PQclear(res);
    return true;
}


/**
 * Executes a SQL query providing some safety checks for connection errors.
 * 
 * \brief           Executes a SQL query
 * \param query     A CString containing an SQL query
 * \param res           Pointer to a PGresult, results of the query will be dumped here
 * \param connection    Pointer to a Database Connection.
 */
static bool executeQuery(const char* query, PGresult*& res, PGconn*& connection)
{
    res = PQexec(connection, query);

    auto status = PQresultStatus(res);

    if (statusFailed(status) || res == nullptr)
    {
        std::cerr << "Query failed: " << PQerrorMessage(connection) << "\n";

#ifdef _DEBUG
        //ADDITIONAL INFORMATION IF DEBUG IS ENABLED
        std::cerr << "Result Status: " << PQresStatus(status) << "\n";
        std::cerr << "Query was: " << query << "\n";
#endif // DEBUG

        PQclear(res);
        return false;
    }
    return true;
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
static bool atomicQuery(const char* query, PGresult*& res, PGconn*& connection)
{
    bool status = false;
    if (beginTransaction(connection))
    {
        status = executeQuery(query, res, connection);
    }

    endTransaction(connection);
    return status;
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

        std::cerr << PQerrorMessage(conn) << "\n";

        exit_program(conn);
    }

#ifdef _DEBUG
    std::clog << "Connection Successful!, printing connection information as follows: " << "\n"
        << "DB name: " << PQdb(conn) << "\n"
        << "Username: " << PQuser(conn) << "\n"
        << "Password: " << PQpass(conn) << "\n"
        << "Host Address: " << PQhostaddr(conn) << " at port " << PQport(conn) << "\n"
        << "Connection Options: " << PQoptions(conn) << "\n"
        << "Connection Status: " << PQstatus(conn) << "\n";
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

std::unordered_set<std::string> const static SQLtokens = { "SELECT", "FROM", "DELETE", "WHERE", "ORDER BY", "GROUP", 
                                                            "SUM", "AVG", "DROP", "SCHEMA", "CASCADE", "CALL", "AS", 
                                                            "JOIN", "ON", "ORDER", "BY", "EXISTS", "NOT"};


std::string static parseQuery(std::string_view query_str)
{

    auto const& tokens = query::SQLtokens;
    std::stringstream out_str;
    std::vector<std::string> split_str;

    size_t last = 0;
    size_t next = 0;
    while ((next = query_str.find(' ', last)) != std::string::npos)
    {
        split_str.emplace_back(query_str.substr(last, next - last));
        last = next + 1;
    }
    split_str.emplace_back(query_str.substr(last));

    size_t i = 0;
    for (auto const& tkn : split_str)
    {
        std::string lwr = tkn;
        std::transform(tkn.begin(), tkn.end(), lwr.begin(),
            [](char c) { return std::toupper(c); });

        if (tokens.find(lwr) != tokens.end())
            out_str << color::FIELD << split_str[i++] << color::RESET;
        else
            out_str << split_str[i++];
        out_str << " ";
    }

    auto str = out_str.str();
    str.erase(str.size() - 1, 1);

    return str;
}

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