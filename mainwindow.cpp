#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Setup
    ui->setupUi(this);
    scene = new QGraphicsScene ();
    ui->graphicsView->setScene(scene);

    imageItem = NULL;

    //Connections
    connect(ui->actionOpen_Image, SIGNAL(triggered()), this, SLOT(loadImage()));
}

MainWindow::~MainWindow()
{
    if (imageItem != NULL)  delete imageItem;
    delete scene;
    delete ui;
}

void MainWindow::setupImageView()
{
    imageItem = new GraphicsImageItem();
    imageItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    scene->addItem(imageItem);
}

void MainWindow::loadImage()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open Image Files"),
                                    "",
                                    tr("Image files (*.jpg *.jpeg *.png *.gif *.bmp)"),
                                    &selectedFilter,
                                    options);
    if (!fileName.isEmpty())
    {
        loadImage(fileName.toStdString());
    }
}

void MainWindow::loadImage(std::string imageLocation)
{
    if (imageItem != NULL)  imageItem->loadImage(imageLocation);
}
