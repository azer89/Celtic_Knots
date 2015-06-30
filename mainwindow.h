
/*
 * Reza Adhitya Saputra
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void TraceOneStep();
    void GenerateAKnot();
    void CheckBoxesTriggered();
    void DimensionChanged();
};

#endif // MAINWINDOW_H
