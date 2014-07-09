#ifndef __FOTOFING_DATABASE_H_
#define __FOTOFING_DATABASE_H_

#include <map>
#include <set>
#include <vector>
#include <string>

#include <sqlite3.h>

struct Column
{
    std::string name;
    bool isPrimary;

    Column() {}

    Column(std::string _name, bool _isPrimary)
    {
        name = _name;
        isPrimary = _isPrimary;
    }

    Column(std::string _name)
    {
        name = _name;
        isPrimary = false;
    }

    bool operator <(const Column& rhs) const
    {
        return this->name < rhs.name;
    }
};

struct Table
{
    std::string name;
    std::set<Column> columns;
};

struct Row
{
    std::map<std::string, std::string> columns;

    std::string getValue(std::string column)
    {
        std::map<std::string, std::string>::iterator it;
        it = columns.find(column);
        if (it != columns.end())
        {
            return it->second;
        }
        return "";
    }
};

struct ResultSet
{
    std::set<std::string> columns;
    std::vector<Row> rows;
};

class Database
{
 private:
    bool m_open;
    sqlite3* m_db;

 public:
    Database();
    ~Database();

    bool open();
    bool close();

    bool checkSchema(std::vector<Table> schema);

    ResultSet executeQuery(std::string query);
    ResultSet executeQuery(std::string query, std::vector<std::string> args);

    bool execute(std::string query);
    bool execute(std::string query, std::vector<std::string> args);

    std::set<std::string> getTables();

    sqlite3* getDB() { return m_db; }
};

#endif
