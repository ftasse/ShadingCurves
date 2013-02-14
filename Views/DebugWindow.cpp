#include "DebugWindow.h"
#include "ui_DebugWindow.h"
#include <QGraphicsPixmapItem>

DebugWindow::DebugWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DebugWindow)
{
    ui->setupUi(this);

    ui->graphicsView->setScene(new QGraphicsScene());
}

DebugWindow::~DebugWindow()
{
    delete ui;
}

void DebugWindow::setImage(QImage image)
{
    QGraphicsPixmapItem *imageItem = ui->graphicsView->scene()->addPixmap(QPixmap().fromImage(image));
    setGeometry(20,20,image.width()+20,image.height()+30);
}
