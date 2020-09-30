#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QTimer>
#include <QThread>
#include <thread>

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

    connect(ui->pushButton, SIGNAL(released()), m_gui_controller, SLOT(testSlot()));
}

MainWindow::~MainWindow()
{
    delete m_gui_controller;
    delete m_model;
    delete ui;
}
