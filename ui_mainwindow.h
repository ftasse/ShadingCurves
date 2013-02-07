/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Thu Feb 7 12:56:30 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QToolBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "Views/GraphicsView.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen_Image;
    QAction *actionCreate_BSpline;
    QAction *actionSave_Image;
    QAction *actionOpen_Curves;
    QAction *actionSave_Curves;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QSplitter *splitter;
    GraphicsView *graphicsView;
    QToolBox *toolBox;
    QWidget *curves_toolbox;
    QPushButton *createCurveButton;
    QPushButton *moveCurveButton;
    QSlider *pointSizeSlider;
    QLabel *label;
    QWidget *shading_toolbox;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(840, 455);
        MainWindow->setAutoFillBackground(true);
        actionOpen_Image = new QAction(MainWindow);
        actionOpen_Image->setObjectName(QString::fromUtf8("actionOpen_Image"));
        actionCreate_BSpline = new QAction(MainWindow);
        actionCreate_BSpline->setObjectName(QString::fromUtf8("actionCreate_BSpline"));
        actionSave_Image = new QAction(MainWindow);
        actionSave_Image->setObjectName(QString::fromUtf8("actionSave_Image"));
        actionOpen_Curves = new QAction(MainWindow);
        actionOpen_Curves->setObjectName(QString::fromUtf8("actionOpen_Curves"));
        actionSave_Curves = new QAction(MainWindow);
        actionSave_Curves->setObjectName(QString::fromUtf8("actionSave_Curves"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        widget = new QWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setAutoFillBackground(true);
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        splitter = new QSplitter(widget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        graphicsView = new GraphicsView(splitter);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
        splitter->addWidget(graphicsView);
        toolBox = new QToolBox(splitter);
        toolBox->setObjectName(QString::fromUtf8("toolBox"));
        toolBox->setAutoFillBackground(true);
        curves_toolbox = new QWidget();
        curves_toolbox->setObjectName(QString::fromUtf8("curves_toolbox"));
        curves_toolbox->setGeometry(QRect(0, 0, 177, 297));
        createCurveButton = new QPushButton(curves_toolbox);
        createCurveButton->setObjectName(QString::fromUtf8("createCurveButton"));
        createCurveButton->setGeometry(QRect(40, 10, 51, 27));
        moveCurveButton = new QPushButton(curves_toolbox);
        moveCurveButton->setObjectName(QString::fromUtf8("moveCurveButton"));
        moveCurveButton->setGeometry(QRect(100, 10, 61, 27));
        pointSizeSlider = new QSlider(curves_toolbox);
        pointSizeSlider->setObjectName(QString::fromUtf8("pointSizeSlider"));
        pointSizeSlider->setGeometry(QRect(10, 80, 160, 29));
        pointSizeSlider->setMinimum(1);
        pointSizeSlider->setValue(8);
        pointSizeSlider->setOrientation(Qt::Horizontal);
        label = new QLabel(curves_toolbox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 60, 66, 17));
        toolBox->addItem(curves_toolbox, QString::fromUtf8("Curves"));
        shading_toolbox = new QWidget();
        shading_toolbox->setObjectName(QString::fromUtf8("shading_toolbox"));
        shading_toolbox->setGeometry(QRect(0, 0, 177, 297));
        toolBox->addItem(shading_toolbox, QString::fromUtf8("Shading"));
        splitter->addWidget(toolBox);

        horizontalLayout->addWidget(splitter);


        verticalLayout->addWidget(widget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 840, 25));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuFile->addAction(actionOpen_Image);
        menuFile->addAction(actionSave_Image);
        menuFile->addSeparator();
        menuFile->addAction(actionOpen_Curves);
        menuFile->addAction(actionSave_Curves);
        menuEdit->addAction(actionCreate_BSpline);

        retranslateUi(MainWindow);

        toolBox->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionOpen_Image->setText(QApplication::translate("MainWindow", "Open Image", 0, QApplication::UnicodeUTF8));
        actionCreate_BSpline->setText(QApplication::translate("MainWindow", "Create BSpline", 0, QApplication::UnicodeUTF8));
        actionSave_Image->setText(QApplication::translate("MainWindow", "Save Image", 0, QApplication::UnicodeUTF8));
        actionOpen_Curves->setText(QApplication::translate("MainWindow", "Open Curves", 0, QApplication::UnicodeUTF8));
        actionSave_Curves->setText(QApplication::translate("MainWindow", "Save Curves", 0, QApplication::UnicodeUTF8));
        createCurveButton->setText(QApplication::translate("MainWindow", "Create", 0, QApplication::UnicodeUTF8));
        moveCurveButton->setText(QApplication::translate("MainWindow", "Move", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Point Size", 0, QApplication::UnicodeUTF8));
        toolBox->setItemText(toolBox->indexOf(curves_toolbox), QApplication::translate("MainWindow", "Curves", 0, QApplication::UnicodeUTF8));
        toolBox->setItemText(toolBox->indexOf(shading_toolbox), QApplication::translate("MainWindow", "Shading", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuEdit->setTitle(QApplication::translate("MainWindow", "Edit", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
