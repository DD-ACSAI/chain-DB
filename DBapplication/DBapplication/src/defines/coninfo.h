#pragma once


#ifdef DB

#define DBSTRING "dbname = " DB

#else

#error Database name must be defined!

#endif // DB

#ifdef PASSWORD

#define PWSTR " password = " PASSWORD

#else 

#define PWSTR ""

#endif // PASSWORD

#ifdef USER

#define USERSTR " user = " USER

#else 

#define USERSTR ""

#endif // USER

#define CONNECT_QUERY DBSTRING PWSTR USERSTR


