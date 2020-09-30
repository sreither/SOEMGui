#include "SlaveTreeModel.h"

#include "../Slave.h"
#include "../EthercatUnit.h"

using namespace SOEMGui;

SlaveTreeModel::SlaveTreeModel(const SOEMGuiController *controller) : m_gui_controller(controller)
{
    setupModelData(controller->getEthercatUnit()->getSlaves());

    connect(m_gui_controller, &SOEMGuiController::dataAvailable, this, &SlaveTreeModel::setEntryValueByHash);
}

SlaveTreeModel::~SlaveTreeModel()
{
    delete m_root;
}

QVariant SlaveTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case 0: return "Hello";
            default: return "adasdas";
        }
    }

    return QVariant();
}

QModelIndex SlaveTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    // hasIndex checks if the values are in the valid ranges by using
    // rowCount and columnCount
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    // In order to create an index we first need to get a pointer to the Item
    // To get started we have either the parent index, which contains a pointer
    // to the parent item, or simply the root item

    SlaveTreeItem *parentItem = parent.isValid()
        ? static_cast<SlaveTreeItem *>(parent.internalPointer())
        : m_root;

    // We can now simply look up the item we want given the parent and the row
    SlaveTreeItem *childItem = parentItem->m_children.at(row);

    // There is no public constructor in QModelIndex we can use, instead we need
    // to use createIndex, which does a little bit more, like setting the
    // model() in the QModelIndex to the model that calls createIndex
    return createIndex(row, column, childItem);
}

Qt::ItemFlags SlaveTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

//    if (static_cast<SlaveTreeItem *>(index.internalPointer())->m_type == SlaveTreeItemType::InputEntry && index.column() == NUM_COLUMNS - 1)
//    {
//        return Qt::ItemIsEditable;
//    }
    return QAbstractItemModel::flags(index);
}

QModelIndex SlaveTreeModel::parent(const QModelIndex &childIndex) const
{
    if (!childIndex.isValid())
    {
        return QModelIndex();
    }

    // Simply get the parent pointer and create an index for it
    SlaveTreeItem *parentItem = static_cast<SlaveTreeItem*>(childIndex.internalPointer())->m_parent;
    return parentItem == m_root
        ? QModelIndex() // the root doesn't have a parent
        : createIndex(parentItem->rowInParent(), 0, parentItem);
}

int SlaveTreeModel::rowCount(const QModelIndex &parent) const
{
    // Parent being invalid means we ask for how many rows the root of the
    // model has, thus we ask the root item
    // If parent is valid we access the Item from the pointer stored
    // inside the QModelIndex
    return parent.isValid()
        ? static_cast<SlaveTreeItem *>(parent.internalPointer())->m_children.size()
        : m_root->m_children.size();
}

int SlaveTreeModel::columnCount(const QModelIndex &parent) const
{
    return NUM_COLUMNS;
}

QVariant SlaveTreeModel::data(const QModelIndex &index, int role) const
{
    // Usually there will be more stuff here, like type conversion from
    // QVariant, handling more roles etc.
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    SlaveTreeItem *item = static_cast<SlaveTreeItem *>(index.internalPointer());

    switch (index.column())
    {
        case 0:
            return QString::fromStdString(item->m_values.name);
        case 1:
            return item->m_type == SlaveTreeItemType::InputEntry || item->m_type == SlaveTreeItemType::OutputEntry
                ? QString::fromStdString(helper::ec_datatype_toString(item->m_values.type).data())
                : QString();
        case 2:
            return item->m_type == SlaveTreeItemType::InputEntry || item->m_type == SlaveTreeItemType::OutputEntry
                ? QString::fromStdString(PDOValue_toString(item->m_values.value))
                : QString();
        case 3:
            return item->m_type == SlaveTreeItemType::InputEntry
                ? true
                : false;
        default:
            return QString();
    }
}

QString SlaveTreeModel::getName(const QModelIndex &index) const
{
    SlaveTreeItem *item = static_cast<SlaveTreeItem *>(index.internalPointer());
    return QString::fromStdString(item->m_values.name);
}

PDOValueT SlaveTreeModel::getValue(const QModelIndex &index) const
{
    SlaveTreeItem *item = static_cast<SlaveTreeItem *>(index.internalPointer());
    return item->m_values.value;
}

void SlaveTreeModel::setValue(const QModelIndex &index, PDOValueT value) const
{
    SlaveTreeItem *item = static_cast<SlaveTreeItem *>(index.internalPointer());
    if (!m_gui_controller->getEthercatUnit()->setInputValue(item->m_hash, value))
    {
        std::cerr << "Could not set new value\n";
    }
}

ec_datatype SlaveTreeModel::getDataType(const QModelIndex &index) const
{
    SlaveTreeItem *item = static_cast<SlaveTreeItem *>(index.internalPointer());
    return item->m_values.type;
}

void SlaveTreeModel::setEntryValueByHash(std::size_t pdoSubEntry_hash)
{
    SlaveTreeItem *item = m_pdoSubEntryHash_to_item.at(pdoSubEntry_hash);
    item->setValue(m_gui_controller->getEthercatUnit()->getValue(pdoSubEntry_hash));

    QModelIndex index = createIndex(item->rowInParent(), 2, item);
    emit dataChanged(index, index, QVector<int>() << Qt::EditRole);
}

void SlaveTreeModel::setupModelData(const std::vector<Slave>* slaves)
{
    m_root = new SlaveTreeItem();
    m_root->m_type = SlaveTreeItemType::RootDummy;

    auto createItem = [&](SlaveTreeItemType type, SlaveTreeItem* parent){
        SlaveTreeItem* item = new SlaveTreeItem();
        item->m_parent = parent;
        item->m_type = type;
        item->m_values.editable = (type == SlaveTreeItemType::InputEntry);
        item->m_parent->m_children.append(item);
        return item;
    };


    for (const Slave& slave : *slaves)
    {
        // Create Slave
        SlaveTreeItem* slaveItem = createItem(SlaveTreeItemType::SlaveDummy, m_root);
        slaveItem->m_values.name = slave.getName();

        // Add inputDummy
        SlaveTreeItem* inputDummy = createItem(SlaveTreeItemType::InputDummy, slaveItem);
        inputDummy->m_values.name = "Inputs";

        // Add inputs
        for (const std::string& pdoName : slave.getInputPDONames())
        {
            SlaveTreeItem* pdoItem = createItem(SlaveTreeItemType::InputRecord, inputDummy);
            pdoItem->m_values.name = pdoName;

            // Add SubEntries
            for (const PDOSubEntry& subEntry : slave.getInputPDOEntryRef(pdoName).entries)
            {
                SlaveTreeItem* pdoSubItem = createItem(SlaveTreeItemType::InputEntry, pdoItem);
                pdoSubItem->m_values.name = subEntry.name;
                pdoSubItem->m_values.type = subEntry.datatype;
                pdoSubItem->m_hash = subEntry.hash;

                m_pdoSubEntryHash_to_item[subEntry.hash] = pdoSubItem;
            }
        }

        // Add outputDummy
        SlaveTreeItem* outputDummy = createItem(SlaveTreeItemType::OutputDummy, slaveItem);
        outputDummy->m_values.name = "Outputs";

        // Add outputs
        for (const std::string& pdoName : slave.getOutputPDONames())
        {
            SlaveTreeItem* pdoItem = createItem(SlaveTreeItemType::OutputRecord, outputDummy);
            pdoItem->m_values.name = pdoName;

            // Add SubEntries
            for (const PDOSubEntry& subEntry : slave.getOutputPDOEntryRef(pdoName).entries)
            {
                SlaveTreeItem* pdoSubItem = createItem(SlaveTreeItemType::OutputEntry, pdoItem);
                pdoSubItem->m_values.name = subEntry.name;
                pdoSubItem->m_values.type = subEntry.datatype;
                pdoSubItem->m_hash = subEntry.hash;

                m_pdoSubEntryHash_to_item[subEntry.hash] = pdoSubItem;
            }
        }
    }
}
SlaveTreeModel::SlaveTreeItem::~SlaveTreeItem()
{
    qDeleteAll(m_children);
}

void SlaveTreeModel::SlaveTreeItem::setValue(PDOValueT value)
{
    if (value.index() == value.index())
    {
        m_values.value = value;
    }
}

int SlaveTreeModel::SlaveTreeItem::rowInParent() const
{
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<SlaveTreeItem *>(this));
    } else {
        return 0;
    }
}
