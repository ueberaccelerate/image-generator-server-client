#include "recivermainwindow.h"
#include "./ui_recivermainwindow.h"

#include <QStyleFactory>
#include <QtDebug>
#include <QPushButton>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QTime>
#include <QDir>
#include <QCloseEvent>
#include <QMessageBox>

#include <random>
#include <algorithm>

#include <boost/bind.hpp>

ReciverMainWindow::ReciverMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ReciverMainWindow)
    , is_connected_(false)
    , socket_(io_context_)
    , work_(boost::asio::make_work_guard(io_context_))
    , work_thread_(new boost::thread(boost::bind(&boost::asio::io_context::run, &io_context_))) {
    ui->setupUi(this);
    setFixedSize(width(),height());
    setRecieverStyle();
    setDefaultValues();


    connect(ui->connectionButton, &QPushButton::pressed, this, &ReciverMainWindow::handleConnectionClicked);

    connect(this, &ReciverMainWindow::errorAddress,
                         this, &ReciverMainWindow::handleErrorAddress);
    connect(this, &ReciverMainWindow::errorConnection,
                         this, &ReciverMainWindow::handleErrorConnection);
    connect(this, &ReciverMainWindow::successConnection,
                         this, &ReciverMainWindow::handleSuccessConnection);

    connect(this, &ReciverMainWindow::errorCloseSocket,
                         this, &ReciverMainWindow::handleErrorCloseSocket);

    connect(this, &ReciverMainWindow::sendConnection,
                         this, &ReciverMainWindow::handleConnection);
    connect(this, &ReciverMainWindow::sendDisconnection,
                         this, &ReciverMainWindow::handleDisconnection);

    connect(this, &ReciverMainWindow::errorConfigRead,
                         this, &ReciverMainWindow::handleErrorConfigRead);


    qRegisterMetaType<std::vector<unsigned char>>("const std::vector<unsigned char>&");
    connect(this, SIGNAL(updateImage(const std::vector<unsigned char>&)), this, SLOT(handleUpdateImage(const std::vector<unsigned char>&)));
    async_save_ = std::make_unique<async::TimerThread>(async::TimerThread::Interval(30), boost::bind(&ReciverMainWindow::saveGeneratedImage, this, _1));
}

ReciverMainWindow::~ReciverMainWindow()
{
    delete ui;
    work_.reset();
    if (work_thread_)
      work_thread_->join();
}

void ReciverMainWindow::handleConnectionClicked()
{
    if (is_connected_) {
        emit sendDisconnection();
        return;
    }
    emit sendConnection();
}

void ReciverMainWindow::handleErrorAddress()
{
    setInfo("ErrorAddress");
    updateConnectionStatus(false);
    async_reader_ = nullptr;
}

void ReciverMainWindow::handleErrorConnection()
{
    setInfo("ErrorConnection");
    updateConnectionStatus(false);
    async_reader_ = nullptr;
}

void ReciverMainWindow::handleErrorCloseSocket()
{
    setInfo("CloseSocket");
    updateConnectionStatus(false);
    async_reader_ = nullptr;
}

void ReciverMainWindow::handleSuccessConnection()
{
    setInfo("SuccessConnection");
    updateConnectionStatus(true);

    frame_count_ = 0;
    ui->framerateLabel->setText(QString::number(config_.getFramerate()));
    ui->widthLabel->setText(QString::number(config_.getWidth()));
    ui->heightLabel->setText(QString::number(config_.getHeight()));

    image_data_.reserve(config_.getWidth() * config_.getHeight());

    async_reader_ = std::make_unique<async::TimerThread>(config_.getFramerate(), boost::bind(&ReciverMainWindow::recieveGeneratedImage, this, _1));
}



void ReciverMainWindow::handleConnection()
{
    boost::system::error_code error;
    try
    {
        const boost::asio::ip::address_v4 ep = boost::asio::ip::make_address_v4(ui->ipLineEdit->text().toStdString(),error);
        const auto port = ui->portSpinBox->value();
        if (error) {
            emit errorAddress();
            return ;
        }
        tcp::endpoint endpoint = tcp::endpoint(boost::asio::ip::address(ep), port);

        socket_.async_connect(endpoint, [&](const auto& error){
          if (error) {
              emit errorConnection();
              return ;
          }
          recieveConfig();
        });
    }
    catch (std::exception& e)
    {
      qDebug() << e.what();
      emit errorConnection();
    }
}

void ReciverMainWindow::handleDisconnection()
{
    boost::system::error_code error;
    setInfo("SuccessDisconnection");
    async_reader_ = nullptr;
    socket_.close(error);
    if (error) {
        emit errorCloseSocket();
    }
    updateConnectionStatus(false);
}


void ReciverMainWindow::handleErrorConfigRead()
{
    setInfo("Disconnecting");
    emit sendDisconnection();
}


void ReciverMainWindow::setInfo(const QString &info)
{
    ui->infoLabel->setText(info);
}

void ReciverMainWindow::updateConnectionStatus(bool connected)
{
    is_connected_ = connected;
    ui->connectionButton->setText(is_connected_ ? "Disconnect" : "Connect");
}

void ReciverMainWindow::recieveConfig()
{
    boost::system::error_code error_code;
    boost::array<boost::int32_t, 1> config_size;
    socket_.read_some(boost::asio::buffer(config_size), error_code);
    if (error_code) {
        emit errorConfigRead();
        return ;
    }
    // read config serdata size in bytes
    config_size_ = config_size[0];

    std::string serdata;
    // buffer enough to hold data
    boost::array<char, 1024> config_buffer;

    size_t len = socket_.read_some(boost::asio::buffer(config_buffer), error_code);
    if (error_code) {
        emit errorConfigRead();
        return ;
    }
    if (len != config_size_) {
        emit errorConfigRead();
        return ;
    }
    std::copy( config_buffer.begin(),config_buffer.begin() + len, std::back_inserter(serdata));

    try {
        config_.LoadFromData(serdata);
    } catch(std::exception e) {
        qDebug() << e.what();
        emit errorConfigRead();
        return;
    }
    emit successConnection();
}

void ReciverMainWindow::recieveGeneratedImage(async::TimerThread &)
{

    boost::system::error_code error;
    const auto buffer_size = config_.getWidth() * config_.getHeight();

    auto start = async::TimerThread::FastTimeNamespace::now();
    size_t total_read = buffer_size;

    ReadBuffer buff;
    image_data_.clear();
    while (true) {
      int len{};
      if (total_read > buff.size()) {
         len = socket_.read_some(boost::asio::buffer(buff), error);
      } else {
         len = socket_.read_some(boost::asio::buffer(buff, total_read), error);
      }
      if (len == 0) {
        emit errorConnection();
        return;
      }
      if (error) {
        emit errorConnection();
        return;
      }
      total_read -= len;

      std::copy(buff.begin(), buff.begin() + len, std::back_inserter(image_data_));
      if (total_read == 0) {
        break;
      }
    }
    auto duratio_ms = std::chrono::duration_cast<std::chrono::milliseconds>(async::TimerThread::FastTimeNamespace::now() - start).count();
    const auto fps = config_.getFramerate();
    const auto real_fps_ms = duratio_ms;
    const auto real_fps = (real_fps_ms == 0) ? fps : (1000.0) / real_fps_ms;

//    if (std::all_of(std::begin(image_data_), std::end(image_data_), [&](const auto value){return value == image_data_[0]; }))
    {

        frame_count_++;
        framerate_real_ = real_fps;

        SaveImageData data;
        data.index = frame_count_.load();
        data.image_data = image_data_;
        save_queue_.push(data);

        emit updateImage(image_data_);
    }
}
void ReciverMainWindow::saveGeneratedImage(async::TimerThread &)
{
    try {
        if(auto data_ptr = save_queue_.try_pop()) {
            QImage image = QImage(data_ptr->image_data.data(), config_.getWidth(), config_.getHeight(), QImage::Format_Grayscale8);
            image.save(QString("frame_")+QString::number(data_ptr->index) + ".png");
        }
    } catch (...) {
    }
}

void ReciverMainWindow::closeEvent(QCloseEvent *event)
{
    if(!save_queue_.empty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Saving data is still in progress");
        msgBox.setText("Close?");
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if(msgBox.exec() == QMessageBox::Yes){
          event->accept();
        }else {
          event->ignore();
        }
    } else {
        QMainWindow::closeEvent(event);
    }
}

void ReciverMainWindow::handleUpdateImage(const std::vector<unsigned char>& buffer)
{
  auto frame_count = frame_count_.load();

  ui->drawableArea->setPixmap(QPixmap::fromImage(QImage(buffer.data(), config_.getWidth(), config_.getHeight(), QImage::Format_Grayscale8)));
  ui->newFrameLabel->setText(QTime::currentTime().toString());
  ui->framecountLabel->setText(QString::number(frame_count));
  ui->realFramerateLabel->setText(QString::number(framerate_real_.load()));
}

void ReciverMainWindow::setRecieverStyle()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    qApp->setPalette(darkPalette);

    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}

void ReciverMainWindow::setDefaultValues()
{
    ui->ipLineEdit->setText("127.0.0.1");
    ui->portSpinBox->setValue(1234);
    ui->savePathLabel->setText(QDir::current().canonicalPath());

}
