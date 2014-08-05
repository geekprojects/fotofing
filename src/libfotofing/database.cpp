
#include <stdio.h>
#include <stdlib.h>

#include <string>

#include <fotofing/database.h>

using namespace std;

string GET_TABLES_SQL = "SELECT name FROM sqlite_master WHERE type='table'";

Database::Database()
{
    m_db = NULL;
    m_open = false;
    m_inTransaction = 0;
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

bool Database::startTransaction()
{
    m_inTransaction++;
    if (m_inTransaction > 1)
    {
        return true;
    }
    char* errmsg;
    sqlite3_exec(m_db, "BEGIN TRANSACTION", NULL, NULL, &errmsg);
    return true;
}

bool Database::endTransaction()
{
    m_inTransaction--;
    if (m_inTransaction > 0)
    {
        return true;

    }
    char* errmsg;
    sqlite3_exec(m_db, "COMMIT", NULL, NULL, &errmsg);

    m_inTransaction = false;

    return true;
}

bool Database::checkSchema(vector<Table> schema)
{
    bool created = false;
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
            for (
                columnIt = tableIt->columns.begin();
                columnIt != tableIt->columns.end();
                columnIt++)
            {
                if (comma)
                {
                    createSql += ", ";
                }
                comma = true;
                createSql += columnIt->name;
                if (columnIt->type.length() > 0)
                {
                    createSql += " " + columnIt->type;
                }
                if (columnIt->isPrimary)
                {
                    createSql += " PRIMARY KEY";
                }
            }

            createSql += ")";
            printf("Database::checkSchema:  -> %s\n", createSql.c_str());

            execute(createSql);
            created = true;
        }
    }

    return created;
}

PreparedStatement* Database::prepareStatement(string sql)
{
    PreparedStatement* ps;
    int res;

    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(m_db, sql.c_str(), sql.length(), &stmt, NULL);
    if (res)
    {
        printf("Database::preparedStatement: prepare res=%d\n", res);
        return NULL;
    }

    ps = new PreparedStatement(this, stmt);

    return ps;
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
        return resultSet;
    }

    vector<string>::iterator argIt;
    int arg = 1;
    for (argIt = args.begin(); argIt != args.end(); argIt++)
    {
        sqlite3_bind_text(
            stmt,
            arg++,
            argIt->c_str(),
            argIt->length(),
            SQLITE_STATIC);
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
                if (value == NULL)
                {
                    value = (const unsigned char*)"";
                }
                row.columns.insert(
                    make_pair(string(name),
                    string((char*)value)));
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
            break;
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
        sqlite3_bind_text(
            stmt,
            arg++,
            argIt->c_str(),
            argIt->length(),
            SQLITE_STATIC);
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

int64_t Database::getLastInsertId()
{
    return sqlite3_last_insert_rowid(m_db);
}

PreparedStatement::PreparedStatement(Database* db, sqlite3_stmt* stmt)
{
    m_db = db;
    m_stmt = stmt;
}

PreparedStatement::~PreparedStatement()
{
    sqlite3_finalize(m_stmt);
}

bool PreparedStatement::bindString(int i, string str)
{
    sqlite3_bind_text(m_stmt, i, str.c_str(), str.length(), SQLITE_TRANSIENT);
    return true;
}

bool PreparedStatement::bindInt64(int i, int64_t value)
{
    sqlite3_bind_int64(m_stmt, i, value);
    return true;
}

int PreparedStatement::getInt(int i)
{
   return sqlite3_column_int(m_stmt, i);
}

int64_t PreparedStatement::getInt64(int i)
{
   return sqlite3_column_int64(m_stmt, i);
}

string PreparedStatement::getString(int i)
{
   const void* str;
   str = sqlite3_column_text(m_stmt, i);
   return string((char*)str);
}

bool PreparedStatement::getBlob(int i, const void** data, uint32_t* length)
{
    *data = sqlite3_column_blob(m_stmt, i);
    *length = sqlite3_column_bytes(m_stmt, i);
   return true;
}


bool PreparedStatement::execute()
{
    int res;
    bool result;
    res = sqlite3_step(m_stmt);

    if (res != SQLITE_DONE)
    {
        printf("PreparedStatement::execute: Unexpected result: res=%d\n", res);
        result = false;
    }
    else
    {
        result = true;
    }
    sqlite3_reset(m_stmt);

    return result;
}

bool PreparedStatement::executeQuery()
{
    // There's not actually anything to do
    return true;
}

bool PreparedStatement::step()
{
    int res;
    res = sqlite3_step(m_stmt);

    if (res == SQLITE_ROW)
    {
        return true;
    }
    else if (res == SQLITE_DONE)
    {
        return false;
    }

    printf("PreparedStatement::executeQuery: Unexpected result: res=%d\n", res);
    return false;
}

bool PreparedStatement::reset()
{
    sqlite3_reset(m_stmt);
    return true;
}


