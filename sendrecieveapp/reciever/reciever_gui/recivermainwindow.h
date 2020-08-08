#ifndef RECIVERMAINWINDOW_H
#define RECIVERMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ReciverMainWindow; }
QT_END_NAMESPACE

class ReciverMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ReciverMainWindow(QWidget *parent = nullptr);
    ~ReciverMainWindow();

private:
    Ui::ReciverMainWindow *ui;
};
#endif // RECIVERMAINWINDOW_H
