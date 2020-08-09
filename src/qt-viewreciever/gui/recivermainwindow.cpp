#include "recivermainwindow.h"
#include "./ui_recivermainwindow.h"

#include <QStyleFactory>
#include <QtDebug>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <random>
struct Size {
    size_t height;
    size_t width;
};

std::vector<unsigned char> generate_image(const Size& size) {
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(33, 196);
  std::vector<unsigned char> data(size.height*size.width, static_cast<unsigned char>(distrib(gen)));

  return data;
}
ReciverMainWindow::ReciverMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ReciverMainWindow)
    , is_connected_(false)
    , io_context_()
    , socket_(io_context_)
{
    ui->setupUi(this);
    setFixedSize(width(),height());
    setRecieverStyle();
    setDefaultValues();

    std::vector<unsigned char> buffer = generate_image(Size{512,512});

    QImage image(buffer.data(), 512, 512, QImage::Format_Grayscale8);

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addPixmap(QPixmap::fromImage(image));
    scene->setSceneRect(image.rect());
    ui->graphicsView->setScene(scene);
    image.save("frame1.png");
/*
    void errorConfigRead();

*/
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

}

ReciverMainWindow::~ReciverMainWindow()
{
    delete ui;
}

void ReciverMainWindow::on_connectionButton_clicked()
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
        socket_.connect(tcp::endpoint(boost::asio::ip::address(ep), port), error);

        if (error) {
            emit errorConnection();
            return ;
        }

        boost::array<boost::int32_t, 1> config_size;
        socket_.read_some(boost::asio::buffer(config_size), error);
        if (error) {
            emit errorConfigRead();
            return ;
        }
        config_size_ = config_size[0];


        std::string serdata;
        // buffer enough to hold data
        boost::array<char, 1024> config_buffer;

        size_t len = socket_.read_some(boost::asio::buffer(config_buffer), error);
        if (error) {
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
    }
    catch (std::exception& e)
    {
      qDebug() << e.what();
    }
}

void ReciverMainWindow::handleDisconnection()
{
    boost::system::error_code error;
    setInfo("SuccessDisconnection");
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
    auto customPalette = ui->graphicsView->palette();
    customPalette.setColor(QPalette::Window, Qt::white);
    customPalette.setColor(QPalette::Base, Qt::white);
    ui->graphicsView->setPalette(customPalette);
}

void ReciverMainWindow::setDefaultValues()
{
    ui->ipLineEdit->setText("127.0.0.1");
    ui->portSpinBox->setValue(1234);
}
