
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

    Table workflowoptable;
    workflowoptable.name = "workflow_ops";
    workflowoptable.columns.insert(Column("operation_id", "integer", true));
    workflowoptable.columns.insert(Column("workflow_id"));
    workflowoptable.columns.insert(Column("op_order"));
    workflowoptable.columns.insert(Column("name"));
    schema.push_back(workflowoptable);

    Table workflowopattrtable;
    workflowopattrtable.name = "workflow_ops_attr";
    workflowopattrtable.columns.insert(Column("workflow_id"));
    workflowopattrtable.columns.insert(Column("operation_id"));
    workflowopattrtable.columns.insert(Column("name"));
    workflowopattrtable.columns.insert(Column("value"));
    schema.push_back(workflowopattrtable);

    bool created;
    created = db->checkSchema(schema);
    if (created)
    {
        db->execute("CREATE UNIQUE INDEX IF NOT EXISTS workflow_uni_idx ON workflow (workflow_id, pid)");
        db->execute("CREATE UNIQUE INDEX IF NOT EXISTS workflow_op_uni_idx ON workflow_ops (workflow_id, operation_id)");
        db->execute("CREATE UNIQUE INDEX IF NOT EXISTS workflow_op_attr_uni_idx ON workflow_ops_attr (operation_id, workflow_id, name)");

    }

}

WorkflowIndex::~WorkflowIndex()
{
}

Workflow* WorkflowIndex::getWorkflow(Photo* photo)
{
    vector<File*> files = m_index->getFiles(photo->getId());
    Workflow* workflow = NULL;

    Database* db = m_index->getDatabase();
    PreparedStatement* ps = db->prepareStatement(
        "SELECT workflow_id FROM workflow WHERE pid=?");
    ps->bindString(1, photo->getId());
    ps->executeQuery();
    if (ps->step())
    {
        int64_t wid = ps->getInt64(0);
        workflow = new Workflow(wid, photo, files.at(0));

        PreparedStatement* opAttrPS = db->prepareStatement(
            "SELECT name, value "
            "  FROM workflow_ops_attr"
            "  WHERE workflow_id=? AND operation_id=?");

        PreparedStatement* opPS = db->prepareStatement(
            "SELECT operation_id, name"
            "  FROM workflow_ops"
            "  WHERE workflow_id = ?"
            "  ORDER BY op_order ASC");

        opPS->bindInt64(1, wid);
        opPS->executeQuery();
        while (opPS->step())
        {
            int64_t opId = opPS->getInt64(0);
            string name = opPS->getString(1);
#if 0
            printf(
                "WorkflowIndex::getWorkflow: -> opId=%ld, name=%s\n",
                opId,
                name.c_str());
#endif
            Operation* op = (Operation*)FotofingPlugin::getPlugin(
                "operation",
                name);
#if 0
            printf("WorkflowIndex::getWorkflow: -> op=%p\n", op);
#endif
            if (op != NULL)
            {
                OperationInstance* instance = op->createInstance();
                workflow->addOperation(instance);
                vector<OperationAttribute> attrs = op->getAttributes();
                opAttrPS->bindInt64(1, wid);
                opAttrPS->bindInt64(2, opId);
                opAttrPS->executeQuery();
                while (opAttrPS->step())
                {
                    string attrName = opAttrPS->getString(0);
                    vector<OperationAttribute>::iterator it;
                    int type = 0;
                    for (it = attrs.begin(); it != attrs.end(); it++)
                    {
                        OperationAttribute attr = *it;
                        if (attr.name == attrName)
                        {
                            type = attr.type;
                            break;
                        }
                    }

                    switch (type)
                    {
                        case OPERATION_ATTR_TYPE_INT:
                            instance->setAttribute(
                                attrName,
                                opAttrPS->getInt(1));
                            break;
                        case OPERATION_ATTR_TYPE_DOUBLE:
                            instance->setAttribute(
                                attrName,
                                opAttrPS->getDouble(1));
                            break;
                    }

                }
                opAttrPS->reset();
            }
            else
            {
                printf(
                    "WorkflowIndex::getWorkflow: Unable to find operation: %s\n",
                    name.c_str());
            }
        }
        delete opPS;
        delete opAttrPS;
    }
    else
    {
        // Create a new workflow
        workflow = new Workflow(photo, files.at(0));
        saveWorkflow(workflow);
    }
    delete ps;

    return workflow;
}

bool WorkflowIndex::saveWorkflow(Workflow* w)
{
    Database* db = m_index->getDatabase();
    PreparedStatement* ps;

    db->startTransaction();

    if (w->getId() == -1)
    {
        PreparedStatement* ps = db->prepareStatement(
            "INSERT INTO workflow (pid) VALUES (?)");
        ps->bindString(1, w->getPhoto()->getId());
        ps->execute();
        int64_t rowId = db->getLastInsertId();
        w->setId(rowId);
        printf(
            "WorkflowIndex::saveWorkflow: Inserted new workflow: %ld",
            rowId);
        delete ps;
    }

    printf(
        "WorkflowIndex::saveWorkflow: Clearing old ops: wid=%ld",
        w->getId());

    ps = db->prepareStatement("DELETE FROM workflow_ops WHERE workflow_id=?");
    ps->bindInt64(1, w->getId());
    ps->execute();
    delete ps;

    ps = db->prepareStatement(
        "DELETE FROM workflow_ops_attr WHERE workflow_id=?");
    ps->bindInt64(1, w->getId());
    ps->execute();
    delete ps;

    PreparedStatement* opsPS = db->prepareStatement(
        "INSERT INTO workflow_ops (workflow_id, op_order, name)"
        "  VALUES (?, ?, ?)");
    PreparedStatement* opsAttrPS = db->prepareStatement(
        "INSERT INTO workflow_ops_attr (workflow_id, operation_id, name, value)"
        "  VALUES (?, ?, ?, ?)");

    vector<OperationInstance*> ops = w->getOperations();
    vector<OperationInstance*>::iterator it;
    int order = 1;
    for (it = ops.begin(); it != ops.end(); it++)
    {
        OperationInstance* op = *it;

        opsPS->bindInt64(1, w->getId());
        opsPS->bindInt64(2, order++);
        opsPS->bindString(3, op->getOperation()->getName());
        opsPS->execute();
        int64_t opId = db->getLastInsertId();

        vector<OperationAttribute> attrs = op->getOperation()->getAttributes();
        vector<OperationAttribute>::iterator attrIt;
        for (attrIt = attrs.begin(); attrIt != attrs.end(); attrIt++)
        {
            OperationAttribute attr = *attrIt;
            printf(
                "WorkflowIndex::saveWorkflow: Attribute: %ld, %ld, %s\n",
                w->getId(),
                opId,
                attr.name.c_str());
            opsAttrPS->bindInt64(1, w->getId());
            opsAttrPS->bindInt64(2, opId);
            opsAttrPS->bindString(3, attr.name);
            switch (attr.type)
            {
                case OPERATION_ATTR_TYPE_INT:
                    opsAttrPS->bindInt64(4, op->getAttributeInt(attr.name));
                    break;

                case OPERATION_ATTR_TYPE_DOUBLE:
                    opsAttrPS->bindDouble(4, op->getAttributeDouble(attr.name));
                    break;
            }
            opsAttrPS->execute();
        }
    }

    delete opsPS;
    delete opsAttrPS;

    db->endTransaction();

    return true;
}

