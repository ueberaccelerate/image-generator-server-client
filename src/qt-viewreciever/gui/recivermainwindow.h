#ifndef RECIVERMAINWINDOW_H
#define RECIVERMAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include <boost/array.hpp>
#include <boost/asio.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class ReciverMainWindow; }
QT_END_NAMESPACE

using boost::asio::ip::tcp;
class ReciverMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ReciverMainWindow(QWidget *parent = nullptr);
    ~ReciverMainWindow();
signals:
    void errorAddress();
    void errorConnection();
    void errorCloseSocket();

    void successConnection();
private slots:
    void on_connectionButton_clicked();
    void handleErrorAddress();
    void handleErrorConnection();
    void handleCloseSocket();
    void handleSuccessConnection();

private:
    Ui::ReciverMainWindow *ui;
    bool isConnected;
    boost::asio::io_context io_context;
    tcp::socket socket;

    void setRecieverStyle();
    void setDefaultValues();
    void setInfo(const QString &info);
    void updateConnectionStatus(bool /*connected*/);

};
#endif // RECIVERMAINWINDOW_H
