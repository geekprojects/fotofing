
#include <stdio.h>
#include <stdlib.h>

#include <string>

#include "database.h"

using namespace std;

string GET_TABLES_SQL = "SELECT name FROM sqlite_master WHERE type='table'";

Database::Database()
{
    m_open = false;
    m_db = NULL;
}

Database::~Database()
{
    close();
}

bool Database::open()
{
    if (m_open)
    {
        return true;
    }

    int res;

    res = sqlite3_open("fotofing.db", &m_db);
    if (res)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_db));
        sqlite3_close(m_db);
        return false;
    }

    m_open = true;

    return true;
}

bool Database::close()
{
    if (!m_open)
    {
        return true;
    }
    sqlite3_close(m_db);
    return true;
}

bool Database::checkSchema(vector<Table> schema)
{
    set<string> tables = getTables();
    set<string>::iterator it;

    vector<Table>::iterator tableIt;

    for (tableIt = schema.begin(); tableIt != schema.end(); tableIt++)
    {
        it = tables.find(tableIt->name);
        if (it == tables.end())
        {
            string createSql = "CREATE TABLE " + tableIt->name + " (";

            bool comma = false;
            set<Column>::iterator columnIt;
            for (columnIt = tableIt->columns.begin(); columnIt != tableIt->columns.end(); columnIt++)
            {
                if (comma)
                {
                    createSql += ", ";
                }
                comma = true;
                createSql += columnIt->name;
            }

            createSql += ")";
            printf("Database::checkSchema:  -> %s\n", createSql.c_str());

            execute(createSql);
        }
    }

    return true;
}

ResultSet Database::executeQuery(string query)
{
    vector<string> noargs;
    return executeQuery(query, noargs);
}

ResultSet Database::executeQuery(string query, vector<string> args)
{
    ResultSet resultSet;
    int res;

    if (!m_open)
    {
        open();
    }

    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(m_db, query.c_str(), query.length(), &stmt, NULL);
    if (res)
    {
        printf("Database::executeQuery: prepare res=%d\n", res);
    }

    vector<string>::iterator argIt;
    int arg = 1;
    for (argIt = args.begin(); argIt != args.end(); argIt++)
    {
        sqlite3_bind_text(stmt, arg++, argIt->c_str(), argIt->length(), SQLITE_STATIC);
    }

    while (true)
    {
        int s;
        s = sqlite3_step(stmt);

        if (s == SQLITE_ROW)
        {
            Row row;

            int count = sqlite3_column_count(stmt);
            int c;
            for (c = 0; c < count; c++)
            {
                const char* name;
                name = sqlite3_column_name(stmt, c);

                if (resultSet.rows.size() == 0)
                {
                    resultSet.columns.insert(string(name));
                }

                const unsigned char* value;
                value = sqlite3_column_text(stmt, 0);
                row.columns.insert(make_pair(string(name), string((char*)value)));
#if 0
                printf("Database::executeQuery: %s=%s\n", name, value);
#endif
            }
            resultSet.rows.push_back(row);
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
        else
        {
            printf("Database::open: Error: %s\n", sqlite3_errmsg(m_db));
        }
    }
    sqlite3_finalize(stmt);
    return resultSet;
}

bool Database::execute(string query)
{
    vector<string> noargs;
    return execute(query, noargs);
}

bool Database::execute(string query, vector<string> args)
{
    int res;

    if (!m_open)
    {
        open();
    }

    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(m_db, query.c_str(), query.length(), &stmt, NULL);
    if (res)
    {
        printf("Database::execute: prepare res=%d\n", res);
        return false;
    }

    vector<string>::iterator argIt;
    int arg = 1;
    for (argIt = args.begin(); argIt != args.end(); argIt++)
    {
        sqlite3_bind_text(stmt, arg++, argIt->c_str(), argIt->length(), SQLITE_STATIC);
    }

    res = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (res != SQLITE_DONE)
    {
        printf("Database::execute: Unexpected result: res=%d\n", res);
        return false;
    }

    return false;
}

set<string> Database::getTables()
{
    set<string> tables;
    ResultSet results;

    results = executeQuery(GET_TABLES_SQL);
    vector<Row>::iterator rowIt;
    for (rowIt = results.rows.begin(); rowIt != results.rows.end(); rowIt++)
    {
        map<string, string>::iterator it = rowIt->columns.find("name");
        if (it != rowIt->columns.end())
        {
            tables.insert(it->second);
        }
    }
    return tables;
}
