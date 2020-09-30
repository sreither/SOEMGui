#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "InputEditButtonDelegate.h"

#include <QTimer>
#include <QThread>
#include <QDesktopWidget>

using namespace SOEMGui;

MainWindow::MainWindow(const std::string& ifname, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qRegisterMetaType<std::size_t>("std::size_t");

    ui->setupUi(this);

    m_gui_controller = new SOEMGuiController(ifname);

    m_model = new SlaveTreeModel(m_gui_controller);
    ui->treeView->setModel(m_model);

    InputEditButtonDelegate* delegate = new InputEditButtonDelegate(ui->treeView);
    ui->treeView->setItemDelegate(delegate);

    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
}

MainWindow::~MainWindow()
{
    delete m_gui_controller;
    delete m_model;
    delete ui;
}
