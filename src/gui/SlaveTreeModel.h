#ifndef SLAVETREEMODEL_H
#define SLAVETREEMODEL_H

#include <QAbstractItemModel>

#include "SOEMGuiController.h"

namespace SOEMGui {
    static constexpr int NUM_COLUMNS = 4;

    class SlaveTreeModel : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        explicit SlaveTreeModel(const SOEMGuiController* controller);
        ~SlaveTreeModel();

        // QAbstractItemModel interface
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        Qt::ItemFlags flags(const QModelIndex &childIndex) const override;
        QModelIndex parent(const QModelIndex &childIndex = QModelIndex()) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &childIndex, int role) const override;

        QString getName(const QModelIndex &index) const;
        PDOValueT getValue(const QModelIndex &index) const;
        void setValue(const QModelIndex &index, PDOValueT value) const;
        ec_datatype getDataType(const QModelIndex &index) const;
    public slots:
        void setEntryValueByHash(std::size_t pdoSubEntry_hash);
        void fillTreeView();
    signals:
        void setupFinished();

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
                bool editable;
            };

            SlaveTreeItemType m_type{};
            std::size_t m_hash{0};

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
