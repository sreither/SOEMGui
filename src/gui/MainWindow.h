#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../EthercatUnit.h"

#include "SlaveTreeModel.h"
#include "SOEMGuiController.h"


namespace Ui {
class MainWindow;
}

namespace SOEMGui {
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MainWindow(const std::string& ifname, QWidget *parent = 0);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;

        EthercatUnit* m_ethercat_unit;
        SlaveTreeModel* m_model;
        SOEMGuiController* m_gui_controller;
    };

}

#endif // MAINWINDOW_H
