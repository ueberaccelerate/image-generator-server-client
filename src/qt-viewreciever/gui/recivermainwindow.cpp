#include "recivermainwindow.h"
#include "./ui_recivermainwindow.h"

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
{
    ui->setupUi(this);
    setFixedSize(width(),height());

    std::vector<unsigned char> buffer = generate_image(Size{512,512});

    QImage image(buffer.data(), 512, 512, QImage::Format_Grayscale8);

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addPixmap(QPixmap::fromImage(image));
    scene->setSceneRect(image.rect());
    ui->graphicsView->setScene(scene);
    image.save("frame1.png");
}

ReciverMainWindow::~ReciverMainWindow()
{
    delete ui;
}

void ReciverMainWindow::on_connectionButton_clicked()
{
    isConnected = !isConnected;
    ui->connectionButton->setText(isConnected ? "Disconnect" : "Connect");
    qDebug() << (isConnected ? "Disconnect" : "Connect");
}
