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

//    connect(m_model, &SlaveTreeModel::setupFinished, ui->treeView, &QTreeView::update);

    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

    // Fill combobox
    auto adapterNames = m_gui_controller->getAdapterNames();
    ui->comboBox_adapterNames->addItems(adapterNames);
}

MainWindow::~MainWindow()
{
    delete m_gui_controller;
    delete m_model;
    delete ui;
}

void SOEMGui::MainWindow::on_pushButton_connect_released()
{
    QString ifname = ui->comboBox_adapterNames->currentText();
    m_gui_controller->connectToAdapter(ifname);
}
