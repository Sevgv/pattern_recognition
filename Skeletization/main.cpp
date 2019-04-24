#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QAction>
#include <iostream>
#include <fstream>
#include <cmath>
#include "skelet.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    ImageOperations* io = new ImageOperations();
    QApplication a(argc, argv);

    QAction* exitAction=new QAction("&Exit", qApp);
    exitAction->setShortcut(Qt::Key_Escape);
    QObject::connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    QImage original_image("../F00027_09.bmp");

    Image image = io->qImage_to_image(original_image);

    Image binaryImage = io->binary_image(image, 120);
    QImage qBinaryImage = io->image_to_qImage(binaryImage);

    Image skeletImage = skelet(binaryImage, [](uint8_t v) {return v == 255;}, 255);  
    QImage qSkeletImage = io->image_to_qImage(skeletImage);

    Image cleanImage = io->filtration(skeletImage, [](uint8_t v) {return v == 255;}, 255);
    Image cleanImageTwo = io->filtration(cleanImage, [](uint8_t v) {return v == 255;}, 255);
    QImage qCleanImage = io->image_to_qImage(cleanImageTwo);

    bool b = qSkeletImage.save("skelet.png");
    bool f = qCleanImage.save("skelet_clean.png");

    QGraphicsScene scene;
    scene.addPixmap(QPixmap::fromImage(original_image))->setPos(0, 0);
    scene.addPixmap(QPixmap::fromImage(qBinaryImage))->setPos(original_image.width(), 0);
    scene.addPixmap(QPixmap::fromImage(qSkeletImage))->setPos(original_image.width() + qBinaryImage.width(), 0);
    scene.addPixmap(QPixmap::fromImage(qCleanImage))->setPos(original_image.width() + qBinaryImage.width() + qSkeletImage.width(), 0);

    QGraphicsView graphicsView(&scene);
    graphicsView.addAction(exitAction);
    graphicsView.show();

    return a.exec();
}
