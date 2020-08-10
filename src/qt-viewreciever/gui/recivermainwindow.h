#ifndef RECIVERMAINWINDOW_H
#define RECIVERMAINWINDOW_H

#include <async/TimerThread.h>
#include <async/threadsafe_queue.h>
#include <resource/config.hpp>

#include <QMainWindow>
#include <QString>
#include <QGraphicsScene>

#include <memory>
#include <vector>
#include <atomic>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/scoped_ptr.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class ReciverMainWindow; }
QT_END_NAMESPACE

struct SaveImageData {
    size_t index;
    std::vector<unsigned char> image_data;
};



using boost::asio::ip::tcp;
class ReciverMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    using ReadBuffer = boost::array<unsigned char, 65535>;
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

    void updateImage(const std::vector<unsigned char>&);
private slots:
    void handleErrorAddress();
    void handleErrorConnection();
    void handleErrorCloseSocket();
    void handleErrorConfigRead();

    void handleSuccessConnection();

    void handleConnection();
    void handleDisconnection();

    void handleConnectionClicked();
    void handleUpdateImage(const std::vector<unsigned char>&);

    void close();
private:
    Ui::ReciverMainWindow *ui;

    bool is_connected_;
    boost::asio::io_context io_context_;
    tcp::socket socket_;

    resource::Config config_;
    size_t config_size_;

    std::atomic_int frame_count_;
    std::atomic_int framerate_real_;

    async::threadsafe_queue<SaveImageData, 32> save_queue_;
    std::future<void> working_thread_;

    boost::asio::executor_work_guard<
      boost::asio::io_context::executor_type> work_;
    boost::scoped_ptr<boost::thread> work_thread_;

    std::unique_ptr< async::TimerThread> async_reader_;
    std::unique_ptr< async::TimerThread> async_save_;

    std::vector<unsigned char> image_data_;


    void setRecieverStyle();
    void setDefaultValues();
    void setInfo(const QString &info);
    void updateConnectionStatus(bool /*connected*/);

    void recieveConfig();
    void recieveGeneratedImage(async::TimerThread& );
    void saveGeneratedImage(async::TimerThread& );

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
};
#endif // RECIVERMAINWINDOW_H
