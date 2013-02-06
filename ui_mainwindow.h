/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Wed Feb 6 16:13:01 2013
**      by: Qt User Interface Compiler version 4.8.3
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
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
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
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout;
    GraphicsView *graphicsView;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(400, 300);
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
        horizontalLayout_2 = new QHBoxLayout(centralWidget);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        graphicsView = new GraphicsView(centralWidget);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));

        horizontalLayout->addWidget(graphicsView);


        horizontalLayout_2->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 25));
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
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuEdit->setTitle(QApplication::translate("MainWindow", "Edit", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
