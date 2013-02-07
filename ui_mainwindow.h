/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Thu 7. Feb 14:43:19 2013
**      by: Qt User Interface Compiler version 5.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
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
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(840, 455);
        MainWindow->setAutoFillBackground(true);
        actionOpen_Image = new QAction(MainWindow);
        actionOpen_Image->setObjectName(QStringLiteral("actionOpen_Image"));
        actionCreate_BSpline = new QAction(MainWindow);
        actionCreate_BSpline->setObjectName(QStringLiteral("actionCreate_BSpline"));
        actionSave_Image = new QAction(MainWindow);
        actionSave_Image->setObjectName(QStringLiteral("actionSave_Image"));
        actionOpen_Curves = new QAction(MainWindow);
        actionOpen_Curves->setObjectName(QStringLiteral("actionOpen_Curves"));
        actionSave_Curves = new QAction(MainWindow);
        actionSave_Curves->setObjectName(QStringLiteral("actionSave_Curves"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        widget = new QWidget(centralWidget);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setAutoFillBackground(true);
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        splitter = new QSplitter(widget);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        graphicsView = new GraphicsView(splitter);
        graphicsView->setObjectName(QStringLiteral("graphicsView"));
        splitter->addWidget(graphicsView);
        toolBox = new QToolBox(splitter);
        toolBox->setObjectName(QStringLiteral("toolBox"));
        toolBox->setAutoFillBackground(true);
        curves_toolbox = new QWidget();
        curves_toolbox->setObjectName(QStringLiteral("curves_toolbox"));
        curves_toolbox->setGeometry(QRect(0, 0, 177, 297));
        createCurveButton = new QPushButton(curves_toolbox);
        createCurveButton->setObjectName(QStringLiteral("createCurveButton"));
        createCurveButton->setGeometry(QRect(40, 10, 51, 27));
        moveCurveButton = new QPushButton(curves_toolbox);
        moveCurveButton->setObjectName(QStringLiteral("moveCurveButton"));
        moveCurveButton->setGeometry(QRect(100, 10, 61, 27));
        pointSizeSlider = new QSlider(curves_toolbox);
        pointSizeSlider->setObjectName(QStringLiteral("pointSizeSlider"));
        pointSizeSlider->setGeometry(QRect(10, 80, 160, 29));
        pointSizeSlider->setMinimum(1);
        pointSizeSlider->setValue(8);
        pointSizeSlider->setOrientation(Qt::Horizontal);
        label = new QLabel(curves_toolbox);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 60, 66, 17));
        toolBox->addItem(curves_toolbox, QStringLiteral("Curves"));
        shading_toolbox = new QWidget();
        shading_toolbox->setObjectName(QStringLiteral("shading_toolbox"));
        shading_toolbox->setGeometry(QRect(0, 0, 177, 297));
        toolBox->addItem(shading_toolbox, QStringLiteral("Shading"));
        splitter->addWidget(toolBox);

        horizontalLayout->addWidget(splitter);


        verticalLayout->addWidget(widget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 840, 25));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QStringLiteral("menuEdit"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
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
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        actionOpen_Image->setText(QApplication::translate("MainWindow", "Open Image", 0));
        actionCreate_BSpline->setText(QApplication::translate("MainWindow", "Create BSpline", 0));
        actionSave_Image->setText(QApplication::translate("MainWindow", "Save Image", 0));
        actionOpen_Curves->setText(QApplication::translate("MainWindow", "Open Curves", 0));
        actionSave_Curves->setText(QApplication::translate("MainWindow", "Save Curves", 0));
        createCurveButton->setText(QApplication::translate("MainWindow", "Create", 0));
        moveCurveButton->setText(QApplication::translate("MainWindow", "Move", 0));
        label->setText(QApplication::translate("MainWindow", "Point Size", 0));
        toolBox->setItemText(toolBox->indexOf(curves_toolbox), QApplication::translate("MainWindow", "Curves", 0));
        toolBox->setItemText(toolBox->indexOf(shading_toolbox), QApplication::translate("MainWindow", "Shading", 0));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0));
        menuEdit->setTitle(QApplication::translate("MainWindow", "Edit", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
