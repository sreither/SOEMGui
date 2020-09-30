#ifndef SLAVETREEMODEL_H
#define SLAVETREEMODEL_H

#include <QAbstractItemModel>

#include "SOEMGuiController.h"

namespace SOEMGui {

    class SlaveTreeModel : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        explicit SlaveTreeModel(const SOEMGuiController* controller);
        ~SlaveTreeModel();

        // QAbstractItemModel interface
//        QVariant headerData(int section, Qt::Orientation orientation,
//                            int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex &parent) const override;
        Qt::ItemFlags flags(const QModelIndex &childIndex) const override;
        QModelIndex parent(const QModelIndex &childIndex) const override;
        int rowCount(const QModelIndex &parent) const override;
        int columnCount(const QModelIndex &parent) const override;
        QVariant data(const QModelIndex &childIndex, int role) const override;
        bool setData(const QModelIndex &index, const QVariant &value, const int role) override;

    public slots:
        void setEntryValueByHash(std::size_t pdoSubEntry_hash);

    private:
        void setupModelData(const std::vector<Slave>* slaves);

        enum SlaveTreeItemType
        {
            RootDummy,
            SlaveDummy,
            InputDummy,
            InputRecord,
            InputEntry,
            OutputDummy,
            OutputRecord,
            OutputEntry
        };

        struct SlaveTreeItem
        {
            ~SlaveTreeItem();

            struct SlaveTreeItemData
            {
                std::string name;
                ec_datatype type;
                PDOValueT value;
            };

            SlaveTreeItemType m_type{};
            std::string m_pdo_name{};
            int m_pdo_entry_sub_index{-1};

            SlaveTreeItemData m_values{};

            QVector<SlaveTreeItem *> m_children;
            SlaveTreeItem *m_parent = nullptr;

            void setValue(PDOValueT value);
            int rowInParent() const;
        };

        SlaveTreeItem *m_root;
        const SOEMGuiController *m_gui_controller;
        std::unordered_map<std::size_t, SlaveTreeItem*> m_pdoSubEntryHash_to_item;
    };
}


#endif // SLAVETREEMODEL_Hl
