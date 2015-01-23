
#include <fotofing/workflow.h>
#include <fotofing/index.h>

using namespace std;

WorkflowIndex::WorkflowIndex(Index* index)
{
    m_index = index;

    Database* db = m_index->getDatabase();
    vector<Table> schema;

    Table workflowTable;
    workflowTable.name = "workflow";
    workflowTable.columns.insert(Column("workflow_id", "INTEGER", true));
    workflowTable.columns.insert(Column("pid"));
    schema.push_back(workflowTable);

    Table workflowOpTable;
    workflowOpTable.name = "workflow_ops";
    workflowOpTable.columns.insert(Column("operation_id", "INTEGER", true));
    workflowOpTable.columns.insert(Column("workflow_id"));
    schema.push_back(workflowOpTable);

    bool created;
    created = db->checkSchema(schema);
    if (created)
    {
        db->execute("CREATE UNIQUE INDEX IF NOT EXISTS workflow_uni_idx ON workflow (workflow_id, pid)");
        db->execute("CREATE UNIQUE INDEX IF NOT EXISTS workflow_op_uni_idx ON workflow_ops (operation_id, workflow_id)");

    }

}

WorkflowIndex::~WorkflowIndex()
{
}

Workflow* WorkflowIndex::getWorkflow(Photo* photo)
{
    vector<File*> files = m_index->getFiles(photo->getId());

    Workflow* workflow = new Workflow(photo, files.at(0));

    return workflow;
}


