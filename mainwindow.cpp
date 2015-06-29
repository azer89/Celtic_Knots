
/*
 * Reza Adhitya Saputra
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "SystemParams.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //traceOneStepButton
    connect(ui->traceOneStepButton,	 SIGNAL(clicked()), this, SLOT(TraceOneStep()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::TraceOneStep()
{
    ui->widget->GetGLWidget()->TraceOneStep();
}
