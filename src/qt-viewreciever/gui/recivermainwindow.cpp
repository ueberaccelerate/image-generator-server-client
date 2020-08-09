#include "recivermainwindow.h"
#include "./ui_recivermainwindow.h"

#include <QStyleFactory>
#include <QtDebug>
#include <QPushButton>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QTime>

#include <random>
#include <algorithm>

#include <boost/bind.hpp>

ReciverMainWindow::ReciverMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ReciverMainWindow)
    , is_connected_(false)
    , io_context_()
    , socket_(io_context_)
    , work_(boost::asio::make_work_guard(io_context_))
    , work_thread_(new boost::thread(boost::bind(&boost::asio::io_context::run, &io_context_)))
{
    ui->setupUi(this);
    //setFixedSize(width(),height()); 
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

    qRegisterMetaType<std::vector<unsigned char>>("std::vector<unsigned char>");
    connect(this, SIGNAL(updateImage(std::vector<unsigned char>)), this, SLOT(handleUpdateImage(std::vector<unsigned char>)));
}

ReciverMainWindow::~ReciverMainWindow()
{
    delete ui;
    if (async_reader_) {
      async_reader_->stop();
    }
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
}

void ReciverMainWindow::handleErrorConnection()
{
    setInfo("ErrorConnection");
    updateConnectionStatus(false);
    emit sendDisconnection();
}

void ReciverMainWindow::handleErrorCloseSocket()
{
    setInfo("CloseSocket");
    updateConnectionStatus(false);
}

void ReciverMainWindow::handleSuccessConnection()
{
    setInfo("SuccessConnection");
    updateConnectionStatus(true);

    qDebug() << "Config size: " << config_size_;

    ui->framerateLabel->setText(QString::number(config_.getFramerate()));
    ui->widthLabel->setText(QString::number(config_.getWidth()));
    ui->heightLabel->setText(QString::number(config_.getHeight()));


    async_reader_ = std::make_unique<async::TimerThread>(config_.getFramerate(), [&](async::TimerThread& t) {

      boost::system::error_code error;
      const auto buffer_size = config_.getWidth() * config_.getHeight();

      std::vector<unsigned char> image_data;
      int total_read = buffer_size;


      while (true) {
        std::vector<unsigned char> buff(total_read);
        if (total_read < 0) {
          const int read_size = socket_.available() + total_read;
          std::copy(buff.begin(), buff.begin() + read_size, std::back_inserter(image_data));
          break;
        }
        auto len = socket_.read_some(boost::asio::buffer(buff), error);
        if (len == 0) {
          emit errorConnection();
          return;
        }
        if (error) {
          emit errorConnection();
          return;
        }
        total_read -= len;
        std::copy(buff.begin(), buff.begin() + len, std::back_inserter(image_data));
        if (total_read == 0) {
          break;
        }
      }
      if (image_data.size() != buffer_size) {
        qDebug() << "lost";
        return; 
      }
      emit updateImage(image_data);
    });
}

void ReciverMainWindow::handleUpdateImage(std::vector<unsigned char> buffer)
{
  QImage image(buffer.data(), config_.getWidth(), config_.getHeight(), QImage::Format_Grayscale8);
  ui->drawableArea->setPixmap(QPixmap::fromImage(image));
  ui->newFrameLabel->setText(QTime::currentTime().toString());
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

          boost::system::error_code error_code;
          boost::array<boost::int32_t, 1> config_size;
          socket_.read_some(boost::asio::buffer(config_size), error_code);
          if (error_code) {
              emit errorConfigRead();
              return ;
          }
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

          config_.LoadFromData(serdata);
          qDebug() << config_.getWidth() << config_.getHeight() << config_.getFramerate();
          emit successConnection();
          
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

    async_reader_->stop();
    async_reader_.reset();
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
}
