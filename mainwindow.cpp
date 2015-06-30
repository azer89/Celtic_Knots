
/*
 * Reza Adhitya Saputra
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include "SystemParams.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //traceOneStepButton
    connect(ui->traceOneStepButton,	 SIGNAL(clicked()), this, SLOT(TraceOneStep()));
    connect(ui->showGridCB,	 SIGNAL(stateChanged(int)), this, SLOT(CheckBoxesTriggered()));
    connect(ui->generateAKnotButton,	 SIGNAL(clicked()), this, SLOT(GenerateAKnot()));

    connect(ui->wSpinBox, SIGNAL(valueChanged(int)),    this, SLOT(DimensionChanged()));
    connect(ui->hSpinBox, SIGNAL(valueChanged(int)),    this, SLOT(DimensionChanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::GenerateAKnot()
{
     ui->widget->GetGLWidget()->GenerateAKnot();
     ui->widget->GetGLWidget()->repaint();
}

void MainWindow::TraceOneStep()
{
    ui->widget->GetGLWidget()->TraceOneStep2();
}

void MainWindow::CheckBoxesTriggered()
{
    SystemParams::show_grid = ui->showGridCB->isChecked();
    ui->widget->GetGLWidget()->repaint();
}

void MainWindow::DimensionChanged()
{
    SystemParams::w = ui->wSpinBox->value();
    SystemParams::h = ui->hSpinBox->value();

    ui->widget->GetGLWidget()->InitCells();
    ui->widget->GetGLWidget()->InitDots();
    ui->widget->GetGLWidget()->ResetData();

    ui->widget->SetScrolls();
    ui->widget->GetGLWidget()->repaint();
}
