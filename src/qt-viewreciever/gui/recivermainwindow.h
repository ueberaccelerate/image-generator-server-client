#ifndef RECIVERMAINWINDOW_H
#define RECIVERMAINWINDOW_H

#include <resource/config.hpp>

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
    void errorConfigRead();

    void successConnection();
    void sendConnection();
    void sendDisconnection();
private slots:
    void on_connectionButton_clicked();
    void handleErrorAddress();
    void handleErrorConnection();
    void handleErrorCloseSocket();
    void handleErrorConfigRead();

    void handleSuccessConnection();


    void handleConnection();
    void handleDisconnection();


private:
    Ui::ReciverMainWindow *ui;
    resource::Config config_;
    size_t config_size_;
    bool is_connected_;
    boost::asio::io_context io_context_;
    tcp::socket socket_;

    void setRecieverStyle();
    void setDefaultValues();
    void setInfo(const QString &info);
    void updateConnectionStatus(bool /*connected*/);



};
#endif // RECIVERMAINWINDOW_H
