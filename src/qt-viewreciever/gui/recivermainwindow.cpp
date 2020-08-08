#include "recivermainwindow.h"
#include "./ui_recivermainwindow.h"

ReciverMainWindow::ReciverMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ReciverMainWindow)
{
    ui->setupUi(this);
}

ReciverMainWindow::~ReciverMainWindow()
{
    delete ui;
}

