
#include "GraphicsImageItem.h"

GraphicsImageItem::GraphicsImageItem(QGraphicsItem *parent):
    QGraphicsPixmapItem(parent)
{
    currentImage = NULL;
}

GraphicsImageItem::~GraphicsImageItem()
{
    if (currentImage != NULL)   delete currentImage;
}

/*QRectF GraphicsImageItem::boundingRect() const
{

}

void GraphicsImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

}*/

void GraphicsImageItem::updateImage()
{
    QImage img_qt = QImage((const unsigned char*)currentImage->data,
                        currentImage->cols, currentImage->rows,
                        currentImage->step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(img_qt.rgbSwapped());
    setPixmap(pixmap);
}

void GraphicsImageItem::loadImage(std::string imageLocation)
{
    resetTransform();
    cv::Mat image = cv::imread(imageLocation);
    if (currentImage != NULL)   delete currentImage;
    currentImage = new cv::Mat(image);
    updateImage();
}
