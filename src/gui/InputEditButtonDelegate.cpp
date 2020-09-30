#include "InputEditButtonDelegate.h"
#include "SlaveTreeModel.h"

#include <QPainter>
#include <QInputDialog>

using namespace SOEMGui;

InputEditButtonDelegate::InputEditButtonDelegate(QObject *parent)
{
    if(QTreeView *treeView = qobject_cast<QTreeView *>(parent))
    {
        m_treeView = treeView;
        m_button = new QPushButton("...", m_treeView);
        m_button->hide();
        connect(m_treeView, SIGNAL(clicked(QModelIndex)),
                              this, SLOT(editButtonClicked(QModelIndex)));
    }
}

InputEditButtonDelegate::~InputEditButtonDelegate()
{
    delete m_button;
}

void InputEditButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() == NUM_COLUMNS-1)
    {
        if (index.model()->data(index, Qt::DisplayRole).toBool())
        {
            m_button->setGeometry(option.rect);

            m_button->setText("Edit " + qobject_cast<const SlaveTreeModel*>(index.model())->getName(index));
            if (option.state == QStyle::State_Selected)
                         painter->fillRect(option.rect, option.palette.highlight());
            QPixmap map = m_button->grab();
            painter->drawPixmap(option.rect.x(),option.rect.y(),map);
        }
    } else {
        QStyledItemDelegate::paint(painter,option, index);
    }
}

void InputEditButtonDelegate::editButtonClicked(const QModelIndex &index)
{
    if(index.column() == NUM_COLUMNS-1)
    {
        if (index.model()->data(index, Qt::DisplayRole).toBool())
        {
            getNewValueFromDialog(index);
        }
    }
}

void InputEditButtonDelegate::getNewValueFromDialog(const QModelIndex &index) const
{
    const SlaveTreeModel* model = qobject_cast<const SlaveTreeModel*>(index.model());

    auto [min, max] = helper::ec_datatype_getLimits(model->getDataType(index));

    bool ok;
    int i = QInputDialog::getInt(m_treeView,
                                tr("Set Input"),
                                model->getName(index),
                                model->data(index.sibling(index.row(),2),
                                Qt::DisplayRole).toInt(),
                                min,
                                max,
                                1,
                                &ok);
    if (ok)
    {
        model->setValue(index, PDOValue_fromIntegralType(i, model->getDataType(index)));
    }
}
