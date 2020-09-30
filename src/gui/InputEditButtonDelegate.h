#ifndef INPUTEDITBUTTONDELEGATE_H
#define INPUTEDITBUTTONDELEGATE_H

#include <QWidget>
#include <QStyleOptionViewItem>
#include <QTreeView>
#include <QPushButton>
#include <QStyledItemDelegate>

namespace SOEMGui {
    class InputEditButtonDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        using QStyledItemDelegate::QStyledItemDelegate;

        explicit InputEditButtonDelegate(QObject* parent = 0);
        ~InputEditButtonDelegate();

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    public slots:
        void editButtonClicked(const QModelIndex &index);

    private:
        void getNewValueFromDialog(const QModelIndex &index) const;

        QTreeView* m_treeView;
        QPushButton* m_button;

    };
}



#endif // INPUTEDITBUTTONDELEGATE_H
