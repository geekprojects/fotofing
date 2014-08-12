
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fotofing/database.h>

using namespace std;

bool createVersion1()
{
    bool res;

    vector<Table> schema;

    Table table1;
    table1.name = "table1";
    table1.columns.insert(Column("id", "INTEGER", true));
    table1.columns.insert(Column("data1"));
    table1.columns.insert(Column("data2"));
    schema.push_back(table1);

    Database* db = new Database("schema_test.db");
    res = db->open();
    if (!res)
    {
        return false;
    }

    res = db->checkSchema(schema);
    if (!res)
    {
        printf("createVersion1: Error: checkSchema didn't make changes\n");
        return false;
    }

    res = db->execute("INSERT INTO table1 (id, data1, data2) VALUES (NULL, 'value1a', 'value1b')");
    if (!res)
    {
        printf("createVersion1: Error: Failed to insert row\n");
        return false;
    }

    delete db;

    return true;
}

bool createVersion2()
{
    bool res;
    vector<Table> schema;

    Table table1;
    table1.name = "table1";
    table1.columns.insert(Column("id", "INTEGER", true));
    table1.columns.insert(Column("data1"));
    table1.columns.insert(Column("data2"));
    table1.columns.insert(Column("data3"));
    schema.push_back(table1);

    Database* db = new Database("schema_test.db");
    res = db->open();
    if (!res)
    {
        return false;
    }

    res = db->checkSchema(schema);
    if (!res)
    {
        printf("createVersion1: Error: checkSchema didn't make changes\n");
        return false;
    }

    res = db->execute("INSERT INTO table1 (id, data1, data2, data3) VALUES (NULL, 'value2a', 'value2b', 'value2c')");
    if (!res)
    {
        printf("createVersion1: Error: Failed to insert row\n");
        return false;
    }

    delete db;

    return true;
}


int main(int argc, char** argv)
{
    unlink("fotofing.db");
    createVersion1();
    createVersion2();
    return 0;
}

