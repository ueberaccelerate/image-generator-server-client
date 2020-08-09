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
    , isConnected(false)
    , io_context()
    , socket(io_context)
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

    connect(this, &ReciverMainWindow::errorAddress,
                         this, &ReciverMainWindow::handleErrorAddress);
    connect(this, &ReciverMainWindow::errorConnection,
                         this, &ReciverMainWindow::handleErrorConnection);
    connect(this, &ReciverMainWindow::successConnection,
                         this, &ReciverMainWindow::handleSuccessConnection);

}

ReciverMainWindow::~ReciverMainWindow()
{
    delete ui;
}

void ReciverMainWindow::on_connectionButton_clicked()
{
    boost::system::error_code error;
    if (isConnected) {
        socket.close(error);
        if (error) {
            emit errorCloseSocket();
        }
        updateConnectionStatus(false);
        return;
    }
    try
    {
        const boost::asio::ip::address_v4 ep = boost::asio::ip::make_address_v4(ui->ipLineEdit->text().toStdString(),error);
        const auto port = ui->portSpinBox->value();
        if (error) {
            emit errorAddress();
            return ;
        }
        socket.connect(tcp::endpoint(boost::asio::ip::address(ep), port), error);

        if (error) {
            emit errorConnection();
            return ;
        }
        emit successConnection();
    }
    catch (std::exception& e)
    {
      qDebug() << e.what();
    }
}

void ReciverMainWindow::handleErrorAddress()
{
    setInfo("handleErrorAddress");
    updateConnectionStatus(false);
}

void ReciverMainWindow::handleErrorConnection()
{
    setInfo("handleErrorConnection");
    updateConnectionStatus(false);

}

void ReciverMainWindow::handleCloseSocket()
{
    setInfo("handleCloseSocket");
    updateConnectionStatus(false);
}

void ReciverMainWindow::handleSuccessConnection()
{
    setInfo("handleSuccessConnection");
    updateConnectionStatus(true);
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

void ReciverMainWindow::setInfo(const QString &info)
{
    ui->infoLabel->setText(info);
}

void ReciverMainWindow::updateConnectionStatus(bool connected)
{
    isConnected = connected;
    ui->connectionButton->setText(isConnected ? "Disconnect" : "Connect");
}
