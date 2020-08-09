#ifndef RECIVERMAINWINDOW_H
#define RECIVERMAINWINDOW_H

#include <async/TimerThread.h>
#include <resource/config.hpp>

#include <QMainWindow>
#include <QString>
#include <QGraphicsScene>

#include <memory>
#include <vector>

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

    void updateImage(std::vector<unsigned char>);
private slots:
    void handleErrorAddress();
    void handleErrorConnection();
    void handleErrorCloseSocket();
    void handleErrorConfigRead();

    void handleSuccessConnection();

    void handleConnection();
    void handleDisconnection();

    void handleConnectionClicked();
    void handleUpdateImage(std::vector<unsigned char>);

private:
    Ui::ReciverMainWindow *ui;

    bool is_connected_;
    boost::asio::io_context io_context_;
    tcp::socket socket_;

    resource::Config config_;
    size_t config_size_;

    async::TimerThread async_context_;
    std::unique_ptr< async::TimerThread> async_reader_;


    void setRecieverStyle();
    void setDefaultValues();
    void setInfo(const QString &info);
    void updateConnectionStatus(bool /*connected*/);
};
#endif // RECIVERMAINWINDOW_H
