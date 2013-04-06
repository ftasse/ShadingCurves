#include <QtGui>
#include <QtOpenGL/QGLWidget>
#include <QWidget>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include "3D/mainwindow3d.h"
#include "3D/tostring.h"
#include "3D/mesh.h"

using namespace std;

MainWindow3D::MainWindow3D(GLuint iW, GLuint iH, cv::Mat *timg)
{
    pathToData = "../imageshading/Data";

	widget = new QWidget;
    setCentralWidget(widget);
    mainWindowPixmap = NULL;

    glwidget1  = new GLviewsubd(iW, iH, timg);
    glwidget2  = new GLviewsubd(iW, iH, timg);
	glBar1	   = new GLbar();
	glBar2	   = new GLbar();

//    glBar1->setFixedSize(30, 180);
//    glBar2->setFixedSize(30, 180);
    glBar1->setFixedSize(80, 20);
    glBar2->setFixedSize(80, 20);

    toolBar1a = new QToolBar(this);
    toolBar2a = new QToolBar(this);

    ctrlWidget1 = new ControlW(glBar1, toolBar1a);
    ctrlWidget2 = new ControlW(glBar2, toolBar2a);

	createActions();
	createMenus();

//	statusBar()->showMessage("Open a file...");

    setWindowTitle(tr("Mesh Visualisation"));
    setMinimumSize(350, 250);
//    resize(width/3, 3*height/4);

	NotFullScr = true;

	addAction(open1Act);
	addAction(open2Act);
	addAction(openBatch1Act);
	addAction(openBatch2Act);
	addAction(exitAct);
	addAction(fullScreenAct);
    addAction(sub1Act);
    addAction(tool1aAct);
    addAction(tool2aAct);
    addAction(darkAct);

    addAction(panxpAct);
    addAction(panxmAct);
    addAction(panypAct);
    addAction(panymAct);

    glwidget1->addAction(tool1aAct);
    glwidget2->addAction(tool2aAct);

    glwidget1->addAction(panxpAct);
    glwidget1->addAction(panxmAct);
    glwidget1->addAction(panypAct);
    glwidget1->addAction(panymAct);

	vline = new QFrame();
	vline->setFrameShape(QFrame::VLine);
	vline->setFrameShadow(QFrame::Sunken);

    horWidget = new QSplitter;

	horLayout = new QHBoxLayout;
	mainLayout = new QVBoxLayout;

	horLayout->setContentsMargins(0,0,0,0);

	setMyLayout(1, true);

	mainLayout->addWidget(horWidget);
    mainLayout->setContentsMargins(0,0,0,0);
	widget->setLayout(mainLayout);

//    QPixmap *mainWindowPixmap = new QPixmap("../imageshading/icons/MWicon3D.png");
//    QIcon mainWindowIcon(*mainWindowPixmap);
//    setWindowIcon(mainWindowIcon);

    toolBar1a->setOrientation(Qt::Vertical);

    toolBar2a->setOrientation(Qt::Vertical);

    toolBar2a->hide();

    this->addToolBar(Qt::LeftToolBarArea,toolBar1a);
    this->addToolBar(Qt::RightToolBarArea,toolBar2a);

    connectAll();
}

MainWindow3D::~MainWindow3D()
{
    delete open1Act; delete open2Act; delete openBatch1Act; delete openBatch2Act;
    delete save1Act; delete save2Act; delete saveBarAct;
    delete saveImg1Act; delete saveImg2Act;
    delete saveLim1Act; delete saveLim2Act;
    delete exitAct; delete fullScreenAct; delete sub1Act;
    delete darkAct; delete helpAct; delete aboutAct;
    delete tool1aAct; delete tool2aAct;
    delete panxpAct; delete panxmAct; delete panypAct; delete panymAct;


    delete mainWindowPixmap;
    delete vline;
    delete horLayout;
    delete mainLayout;
    delete ctrlWidget1;
    delete ctrlWidget2;
    delete glBar1;
    delete glBar2;
    delete toolBar1a;
    delete toolBar2a;
    delete glwidget1;
    delete glwidget2;
    delete horWidget;
    delete widget;
}

void MainWindow3D::setMyLayout(int view, bool full)
{
	int margin;

	margin = 3;

	QLayoutItem *child;

	while ((child = horLayout->takeAt(0)) != 0)
	{
		horLayout->removeItem(child);
	}

	if (!full)
	{
		horLayout->addWidget(glwidget1);
		horLayout->addWidget(glwidget2);
		horLayout->setContentsMargins(margin,margin,margin,margin);
		horWidget->setLayout(horLayout);
		horWidget->update();

		glwidget1->show();
		glwidget2->show();

	}
	else if (view == 1)
	{
		horLayout->addWidget(glwidget1);
		horLayout->setContentsMargins(margin,margin,margin,margin);
		horWidget->setLayout(horLayout);
		horWidget->update();

		glwidget2->hide();
	}
	else if (view == 2)
	{
		horLayout->addWidget(glwidget2);
		horLayout->setContentsMargins(margin,margin,margin,margin);
		horWidget->setLayout(horLayout);
		horWidget->update();

        glwidget1->hide();
	}
}

void MainWindow3D::toggleMyLayout1(bool b)
{
	setMyLayout(1, b);
}

void MainWindow3D::toggleMyLayout2(bool b)
{
	setMyLayout(2, b);
}

void MainWindow3D::resetSliders1()
{
	emit resetSX1(0);
	emit resetSY1(0);
	emit resetSZ1(0);
}

void MainWindow3D::resetSliders2()
{
	emit resetSX2(0);
	emit resetSY2(0);
	emit resetSZ2(0);
}

void MainWindow3D::load1(std::istream &is)
{
	QString			qs, qsn;

	ctrlWidget1->subdLevelSpinbox->setValue(0);

    glwidget1->loadFile(is);

    ctrlWidget1->numV->setText(QString::number(glwidget1->meshCtrl[0]->my_numV));
    ctrlWidget1->numF->setText(QString::number(glwidget1->meshCtrl[0]->my_numF));
    ctrlWidget1->numVsub->setText(QString::number(glwidget1->meshCtrl[0]->my_numV/1000));
    ctrlWidget1->numFsub->setText(QString::number(glwidget1->meshCtrl[0]->my_numF/1000));

	resetSliders1();

	qs = QString(glwidget1->meshCtrl[glwidget1->indexMesh]->my_s.c_str());
    bool isGhost = glwidget1->meshCtrl[glwidget1->indexMesh]->isGhost;

	if (glwidget1->clear)
	{
		ctrlWidget1->meshMenu->clear();
        ctrlWidget1->meshMenu->insertItem(ctrlWidget1->meshMenu->count(), "All meshes");
	}
        if (!isGhost)
        {
            qsn = QString(to_string(ctrlWidget1->meshMenu->count()).c_str());
            ctrlWidget1->meshMenu->insertItem(ctrlWidget1->meshMenu->count(), qsn + " " + qs);
            ctrlWidget1->meshMenu->setCurrentIndex(ctrlWidget1->meshMenu->count() - 1);
        }
}

void MainWindow3D::loadBatch1(std::istream &is)
{
    unsigned int	i, num;
    std::string		str;
    bool			orig;

    orig = ctrlWidget1->checkClear->checkState();
    ctrlWidget1->checkClear->setChecked(true);

    is >> num;
    for (i = 0 ; i < num ; i++)
    {
        is >> str;
        std::ifstream   ifs(str.c_str());
        if(!ifs.is_open())
        {
            cout<< "Failed to find the file:" << str << endl;
            return;
        }
        else
        {
            load1(ifs);
        }
        if (i == 0)
        {
            ctrlWidget1->checkClear->setChecked(false);
        }
    }
    ctrlWidget1->checkClear->setChecked(orig);
    if (num > 0)
    {
        ctrlWidget1->checkClear->setChecked(orig);
        ctrlWidget1->numV->setText(QString::number(glwidget1->meshCtrl[0]->my_numV));
        ctrlWidget1->numF->setText(QString::number(glwidget1->meshCtrl[0]->my_numF));
        ctrlWidget1->numVsub->setText(QString::number(glwidget1->meshCtrl[0]->my_numV/1000));
        ctrlWidget1->numFsub->setText(QString::number(glwidget1->meshCtrl[0]->my_numF/1000));
    }
}

void MainWindow3D::load2(istream &is)
{
	QString			qs, qsn;

	ctrlWidget2->subdLevelSpinbox->setValue(0);
    setMyLayout(2, false);
    ctrlWidget1->checkFull->setChecked(false);

    glwidget2->loadFile(is);

    ctrlWidget2->numV->setText(QString::number(glwidget2->meshCtrl[0]->my_numV));
    ctrlWidget2->numF->setText(QString::number(glwidget2->meshCtrl[0]->my_numF));
    ctrlWidget2->numVsub->setText(QString::number(glwidget2->meshCtrl[0]->my_numV/1000));
    ctrlWidget2->numFsub->setText(QString::number(glwidget2->meshCtrl[0]->my_numF/1000));

    resetSliders2();

	qs = QString(glwidget2->meshCtrl[glwidget2->indexMesh]->my_s.c_str());

	if (glwidget2->clear)
	{
		ctrlWidget2->meshMenu->clear();
        ctrlWidget2->meshMenu->insertItem(ctrlWidget2->meshMenu->count(), "All meshes");
	}
    qsn = QString(to_string(ctrlWidget2->meshMenu->count()).c_str());
    ctrlWidget2->meshMenu->insertItem(ctrlWidget2->meshMenu->count(), qsn + " " + qs);
    ctrlWidget2->meshMenu->setCurrentIndex(ctrlWidget2->meshMenu->count() - 1);

}

void MainWindow3D::loadBatch2(std::istream &is)
{
    unsigned int	i, num;
    std::string		str;
    bool			orig;

    orig = ctrlWidget2->checkClear->checkState();
    ctrlWidget2->checkClear->setChecked(true);

    is >> num;
    for (i = 0 ; i < num ; i++)
    {
        is >> str;
        std::ifstream   ifs(str.c_str());
        if(!ifs.is_open())
        {
            cout<< "Failed to find the file:" << str << endl;
            return;
        }
        else
        {
            load2(ifs);
        }
        if (i == 0)
        {
            ctrlWidget2->checkClear->setChecked(false);
        }
    }
    ctrlWidget2->checkClear->setChecked(orig);
    if (num > 0)
    {
        ctrlWidget2->numV->setText(QString::number(glwidget2->meshCtrl[0]->my_numV));
        ctrlWidget2->numF->setText(QString::number(glwidget2->meshCtrl[0]->my_numF));
        ctrlWidget2->numVsub->setText(QString::number(glwidget2->meshCtrl[0]->my_numV/1000));
        ctrlWidget2->numFsub->setText(QString::number(glwidget2->meshCtrl[0]->my_numF/1000));
    }
}

void MainWindow3D::open1()
{
	QByteArray 		bytes;
	char 			*file_name;
	QString 		meshFileName;

	meshFileName = QFileDialog::getOpenFileName(this,
												tr("Open File"),
                                                pathToData,
                                                tr("(*.off *.ply)"));
	if (!meshFileName.isEmpty())
	{
		bytes = meshFileName.toAscii();
		file_name = bytes.data();
        std::ifstream   ifs(file_name);
        if(!ifs.is_open())
        {
            cout<< "Failed to find the file:" << file_name << endl;
        }
        else
        {
            load1(ifs);
        }
	}
//	statusBar()->showMessage("File: " + meshFileName);
}

void MainWindow3D::openBatch1()
{
	QByteArray 		bytes;
	char 			*file_name;
	QString 		meshFileName;

	meshFileName = QFileDialog::getOpenFileName(this,
												tr("Open File"),
                                                pathToData,
												tr("(*.bat)"));
	if (!meshFileName.isEmpty())
	{
		bytes = meshFileName.toAscii();
		file_name = bytes.data();
        std::ifstream   ifs(file_name);
        if(!ifs.is_open())
        {
            cout<< "Failed to find the file:" << file_name << endl;
        }
        else
        {
            loadBatch1(ifs);
        }
	}
//	statusBar()->showMessage("File: " + meshFileName);
}

void MainWindow3D::open2()
{
	QByteArray 		bytes;
	char 			*file_name;
	QString 		meshFileName;

	meshFileName = QFileDialog::getOpenFileName(this,
												tr("Open File"),
                                                pathToData,
                                                tr("(*.off *.ply)"));
	if (!meshFileName.isEmpty())
	{
        bytes = meshFileName.toAscii();
        file_name = bytes.data();
        std::ifstream   ifs(file_name);
        if(!ifs.is_open())
        {
            cout<< "Failed to find the file:" << file_name << endl;
        }
        else
        {
            load2(ifs);
        }
	}
//	statusBar()->showMessage("File: " + meshFileName);
}

void MainWindow3D::openBatch2()
{
	QByteArray 		bytes;
	char 			*file_name;
	QString 		meshFileName;

	meshFileName = QFileDialog::getOpenFileName(this,
												tr("Open File"),
                                                pathToData,
                                                tr("(*.bat)"));
	if (!meshFileName.isEmpty())
    {
        bytes = meshFileName.toAscii();
        file_name = bytes.data();
        std::ifstream   ifs(file_name);
        if(!ifs.is_open())
        {
            cout<< "Failed to find the file:" << file_name << endl;
        }
        else
        {
            loadBatch2(ifs);
        }
    }
//	statusBar()->showMessage("File: " + meshFileName);
}

void MainWindow3D::save1()
{
	QByteArray 	bytes;
	char 		*file_name;
	QStringList filters;
	bool		isPly;

    if (glwidget1->indexMesh < 0)
	{
        std::cout << "I have no data -> not saving!" << endl;
		QMessageBox msgBox;
		msgBox.setText("I have no data -> not saving!");
		msgBox.exec();
	}
	else
	{
		filters << "OFF file (*.off)"
				<< "PLY file (*.ply)";

        QFileDialog fileDialog(this, tr("Save as..."), pathToData + glwidget1->meshCtrl[glwidget1->indexMesh]->my_save.c_str(), NULL);
		fileDialog.setFilters(filters);
		fileDialog.setFileMode(QFileDialog::AnyFile);
		fileDialog.setAcceptMode(QFileDialog::AcceptSave);
		fileDialog.setDefaultSuffix("off");

		if (fileDialog.exec())
		{
			QStringList files = fileDialog.selectedFiles();
			if (!files.isEmpty())
			{
				if (files[0].endsWith(".ply"))
				{
					isPly = true;
				}
				else
				{
					isPly = false;
				}
				bytes = files[0].toAscii();
				file_name = bytes.data();
				glwidget1->saveFile(file_name, isPly);
//				statusBar()->showMessage("Saved " + files[0], 5000);
			}
		}
	}
}

void MainWindow3D::save2()
{
	QByteArray 	bytes;
	char 		*file_name;
	QStringList filters;
	bool		isPly;

	if (glwidget2->indexMesh < 0)
	{
        std::cout << "I have no data -> not saving!" << endl;
		QMessageBox msgBox;
		msgBox.setText("I have no data -> not saving!");
		msgBox.exec();
	}
	else
	{
		filters << "OFF file (*.off)"
				<< "PLY file (*.ply)";

        QFileDialog fileDialog(this, tr("Save as..."), pathToData + glwidget2->meshCtrl[glwidget2->indexMesh]->my_save.c_str(), NULL);
		fileDialog.setFilters(filters);
		fileDialog.setFileMode(QFileDialog::AnyFile);
		fileDialog.setAcceptMode(QFileDialog::AcceptSave);
		fileDialog.setDefaultSuffix("off");

		if (fileDialog.exec())
		{
			QStringList files = fileDialog.selectedFiles();
			if (!files.isEmpty())
			{
				if (files[0].endsWith(".ply"))
				{
					isPly = true;
				}
				else
				{
					isPly = false;
				}
				bytes = files[0].toAscii();
				file_name = bytes.data();
				glwidget2->saveFile(file_name, isPly);
//				statusBar()->showMessage("Saved " + files[0], 5000);
			}
		}
	}
}

void MainWindow3D::saveLim1()
{
	QByteArray 	bytes;
	char 		*file_name;
	QStringList filters;
	bool		isPly;

	if (glwidget1->indexMesh < 0)
	{
        std::cout << "I have no data -> not saving!" << endl;
		QMessageBox msgBox;
		msgBox.setText("I have no data -> not saving!");
		msgBox.exec();
	}
	else
	{
		filters << "OFF file (*.off)"
				<< "PLY file (*.ply)";

        QFileDialog fileDialog(this, tr("Save as..."), pathToData + glwidget1->meshCtrl[glwidget1->indexMesh]->my_save.c_str(), NULL);
		fileDialog.setFilters(filters);
		fileDialog.setFileMode(QFileDialog::AnyFile);
		fileDialog.setAcceptMode(QFileDialog::AcceptSave);
		fileDialog.setDefaultSuffix("off");

		if (fileDialog.exec())
		{
			QStringList files = fileDialog.selectedFiles();
			if (!files.isEmpty())
			{
				if (files[0].endsWith(".ply"))
				{
					isPly = true;
				}
				else
				{
					isPly = false;
				}
				bytes = files[0].toAscii();
				file_name = bytes.data();
				glwidget1->saveFileLim(file_name, isPly);
//				statusBar()->showMessage("Saved " + files[0], 5000);
			}
		}
	}
}

void MainWindow3D::saveLim2()
{
    QByteArray 	bytes;
    char 		*file_name;
    QStringList filters;
    bool		isPly;

    if (glwidget2->indexMesh < 0)
    {
        std::cout << "I have no data -> not saving!" << endl;
        QMessageBox msgBox;
        msgBox.setText("I have no data -> not saving!");
        msgBox.exec();
    }
    else
    {
        filters << "OFF file (*.off)"
                << "PLY file (*.ply)";

        QFileDialog fileDialog(this, tr("Save as..."), pathToData + glwidget2->meshCtrl[glwidget2->indexMesh]->my_save.c_str(), NULL);
        fileDialog.setFilters(filters);
        fileDialog.setFileMode(QFileDialog::AnyFile);
        fileDialog.setAcceptMode(QFileDialog::AcceptSave);
        fileDialog.setDefaultSuffix("off");

        if (fileDialog.exec())
        {
            QStringList files = fileDialog.selectedFiles();
            if (!files.isEmpty())
            {
                if (files[0].endsWith(".ply"))
                {
                    isPly = true;
                }
                else
                {
                    isPly = false;
                }
                bytes = files[0].toAscii();
                file_name = bytes.data();
                glwidget2->saveFileLim(file_name, isPly);
//				statusBar()->showMessage("Saved " + files[0], 5000);
            }
        }
    }
}

void MainWindow3D::saveImg1()
{
	QStringList filters;

//	if (glwidget1->indexMesh < 0)
//	{
//        std::cout << "I have no data -> not saving!" << endl;
//		QMessageBox msgBox;
//		msgBox.setText("I have no data -> not saving!");
//		msgBox.exec();
//	}
//	else
//	{
        filters << "Raster image files (*.png)";

//		QFileDialog fileDialog(this, tr("Save image"), QDir::currentPath() + "/" + glwidget1->meshCtrl[glwidget1->indexMesh]->my_save.c_str(), NULL);
        QFileDialog fileDialog(this, tr("Save image"), pathToData, NULL);
		fileDialog.setFilters(filters);
		fileDialog.setFileMode(QFileDialog::AnyFile);
		fileDialog.setAcceptMode(QFileDialog::AcceptSave);
		fileDialog.setDefaultSuffix("png");

		// Save the snapshot in memory before the frame buffer is covered by the
		// file save dialog
		QImage snapshot = glwidget1->grabFrameBuffer(true);
		if (fileDialog.exec())
		{
			QStringList files = fileDialog.selectedFiles();
			if (!files.isEmpty())
			{
                snapshot.save(files[0]);
//				statusBar()->showMessage("Saved image " + files[0], 5000);
			}
		}
//	}
}

void MainWindow3D::saveImg2()
{
	QStringList filters;

//    if (glwidget2->indexMesh > 0)
//	{
//        std::cout << "I have no data -> not saving!" << endl;
//		QMessageBox msgBox;
//		msgBox.setText("I have no data -> not saving!");
//		msgBox.exec();
//	}
//	else
//	{
        filters << "Raster image files (*.png)";

//		QFileDialog fileDialog(this, tr("Save image"), QDir::currentPath() + "/" + glwidget2->meshCtrl[0]->my_save.c_str(), NULL);
        QFileDialog fileDialog(this, tr("Save image"), pathToData, NULL);
		fileDialog.setFilters(filters);
		fileDialog.setFileMode(QFileDialog::AnyFile);
		fileDialog.setAcceptMode(QFileDialog::AcceptSave);
		fileDialog.setDefaultSuffix("png");

		// Save the snapshot in memory before the frame buffer is covered by the
		// file save dialog
		QImage snapshot = glwidget2->grabFrameBuffer(true);
		if (fileDialog.exec())
		{
			QStringList files = fileDialog.selectedFiles();
			if (!files.isEmpty())
			{
                snapshot.save(files[0]);
//				statusBar()->showMessage("Saved image " + files[0], 5000);
			}
		}
//	}
}

void MainWindow3D::saveBar()
{
	QStringList filters;
    filters << "Raster image files (*.png)";

    QFileDialog fileDialog(this, tr("Save collor bar"), pathToData, NULL);
	fileDialog.setFilters(filters);
	fileDialog.setFileMode(QFileDialog::AnyFile);
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);
	fileDialog.setDefaultSuffix("png");

	// Save the snapshot in memory before the frame buffer is covered by the
	// file save dialog
	QImage snapshot = glBar1->grabFrameBuffer(true);
	if (fileDialog.exec())
	{
		QStringList files = fileDialog.selectedFiles();
		if (!files.isEmpty())
		{
				snapshot.save(files[0]);
//				statusBar()->showMessage("Saved image " + files[0], 5000);
		}
	}
}

void MainWindow3D::fullScr()
{
	if (NotFullScr)
	{
		showFullScreen();
		NotFullScr = false;
		menuBar()->setVisible(false);
//		labelWidget->setVisible(false);
	}
	else
	{
		showMaximized();
		NotFullScr = true;
		menuBar()->setVisible(true);
//        labelWidget->setVisible(false); // set true to enable
	}
}

void MainWindow3D::help()
{
		QMessageBox hlpBox;
		hlpBox.setText("Basic usage: \n\n"
                       "Open and save *.off, *.ply files using the File menu\n"
					   "Save a view as png, eps or pdf using the File menu\n"
					   "Right click and drag to rotate a view\n"
					   "Scroll in a viewport to zoom\n"
					   "Double-Right click in a viewport to toggle its full view\n"
					   "Middle click to toggle full screen\n"
					   "Select a control point using the left mouse button\n"
					   "Move the selected point using x-, y- and z-sliders\n"
					   "Subdivide using the spin box\n"
					   "Other functions are controlled by buttons and menus provided");
		hlpBox.exec();
}

void MainWindow3D::about()
{
	QMessageBox::about(this, tr("About Menu"),
            tr("Jiri's implementation of subdivision.\n"
               "Fabruary 2013\n"
			   "Jiri.Kosinka(at)cl.cam.ac.uk"));
}

void MainWindow3D::createActions()
{
    open1Act = new QAction(tr("&Open mesh (left)..."), this);
	open1Act->setShortcut(QKeySequence(tr("Ctrl+1")));
    open1Act->setStatusTip(tr("Open an existing mesh in left viewport"));
	connect(open1Act, SIGNAL(triggered()), this, SLOT(open1()));

    open2Act = new QAction(tr("&Open mesh (right)..."), this);
	open2Act->setShortcut(QKeySequence(tr("Ctrl+2")));
    open2Act->setStatusTip(tr("Open an existing mesh in right viewport"));
	connect(open2Act, SIGNAL(triggered()), this, SLOT(open2()));

    openBatch1Act = new QAction(tr("&Open Batch (left)..."), this);
	openBatch1Act->setShortcut(QKeySequence(tr("Ctrl+5")));
    openBatch1Act->setStatusTip(tr("Open a batch in left viewport"));
	connect(openBatch1Act, SIGNAL(triggered()), this, SLOT(openBatch1()));

    openBatch2Act = new QAction(tr("&Open Batch (right)..."), this);
	openBatch2Act->setShortcut(QKeySequence(tr("Ctrl+6")));
    openBatch2Act->setStatusTip(tr("Open a batch in right viewport"));
	connect(openBatch2Act, SIGNAL(triggered()), this, SLOT(openBatch2()));

    save1Act = new QAction(tr("&Save control mesh as (left)..."), this);
    save1Act->setStatusTip(tr("Save the control mesh in left viewport to disk"));
	connect(save1Act, SIGNAL(triggered()), this, SLOT(save1()));

    saveLim1Act = new QAction(tr("&Save subdivided mesh as (left)..."), this);
    saveLim1Act->setStatusTip(tr("Save the subdivided mesh in left viewport to disk"));
	connect(saveLim1Act, SIGNAL(triggered()), this, SLOT(saveLim1()));

    save2Act = new QAction(tr("&Save control mesh as (right)..."), this);
    save2Act->setStatusTip(tr("Save the control mesh in right viewport to disk"));
	connect(save2Act, SIGNAL(triggered()), this, SLOT(save2()));

    saveLim2Act = new QAction(tr("&Save subdivided mesh as (right)..."), this);
    saveLim2Act->setStatusTip(tr("Save the limit mesh in viewport 2 to disk"));
    connect(saveLim2Act, SIGNAL(triggered()), this, SLOT(saveLim2()));

    saveImg1Act = new QAction(tr("&Save image as (left)..."), this);
    saveImg1Act->setStatusTip(tr("Save the image in left viewport to disk"));
	connect(saveImg1Act, SIGNAL(triggered()), this, SLOT(saveImg1()));

    saveImg2Act = new QAction(tr("&Save image as (right)..."), this);
    saveImg2Act->setStatusTip(tr("Save the image in right viewport to disk"));
	connect(saveImg2Act, SIGNAL(triggered()), this, SLOT(saveImg2()));

    saveBarAct = new QAction(tr("&Save the colour bar as (left)..."), this);
    saveBarAct->setStatusTip(tr("Save the colour bar of left vieport to disk"));
	connect(saveBarAct, SIGNAL(triggered()), this, SLOT(saveBar()));

	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	fullScreenAct = new QAction(tr("&Full Screen"), this);
	fullScreenAct->setShortcuts(QKeySequence::Find);
	fullScreenAct->setStatusTip(tr("Toggle Full screen"));
	connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(fullScr()));

	helpAct = new QAction(tr("&Help"), this);
	helpAct->setStatusTip(tr("Show the application's Help box"));
	connect(helpAct, SIGNAL(triggered()), this, SLOT(help()));

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    sub1Act = new QAction(this);
    sub1Act->setShortcut(QKeySequence(tr("S")));
    connect(sub1Act, SIGNAL(triggered()), this, SLOT(sub1()));

    tool1aAct = new QAction(this);
    tool1aAct->setShortcut(QKeySequence(tr("T")));
    connect(tool1aAct, SIGNAL(triggered()), this, SLOT(tool1a()));

    tool2aAct = new QAction(this);
    tool2aAct->setShortcut(QKeySequence(tr("Y")));
    connect(tool2aAct, SIGNAL(triggered()), this, SLOT(tool2a()));

    darkAct = new QAction(this);
    darkAct->setShortcut(QKeySequence(tr("D")));
    connect(darkAct, SIGNAL(triggered()), this, SLOT(dark()));

    panxpAct = new QAction(tr("Pan x+"), this);
    panxpAct->setShortcuts(QKeySequence::SelectNextChar);
    connect(panxpAct, SIGNAL(triggered()), glwidget1, SLOT(setRotXp()));
    connect(panxpAct, SIGNAL(triggered()), glwidget2, SLOT(setRotXp()));

    panxmAct = new QAction(tr("Pan x-"), this);
    panxmAct->setShortcuts(QKeySequence::SelectPreviousChar);
    connect(panxmAct, SIGNAL(triggered()), glwidget1, SLOT(setRotXm()));
    connect(panxmAct, SIGNAL(triggered()), glwidget2, SLOT(setRotXm()));

    panypAct = new QAction(tr("Pan y+"), this);
    panypAct->setShortcuts(QKeySequence::SelectPreviousLine);
    connect(panypAct, SIGNAL(triggered()), glwidget1, SLOT(setRotYp()));
    connect(panypAct, SIGNAL(triggered()), glwidget2, SLOT(setRotYp()));

    panymAct = new QAction(tr("Pan y-"), this);
    panymAct->setShortcuts(QKeySequence::SelectNextLine);
    connect(panymAct, SIGNAL(triggered()), glwidget1, SLOT(setRotYm()));
    connect(panymAct, SIGNAL(triggered()), glwidget2, SLOT(setRotYm()));
}

void MainWindow3D::sub1()
{
    if (glwidget1->meshCtrl.size() > 0)
    {
        ctrlWidget1->subdLevelSpinbox->setValue(ctrlWidget1->subdLevelSpinbox->value() + 1);
    }
}

void MainWindow3D::tool1a()
{
    if (toolBar1a->isVisible())
    {
        toolBar1a->hide();
    }
    else
    {
        toolBar1a->show();
    }
}

void MainWindow3D::tool2a()
{
    if (toolBar2a->isVisible())
    {
        toolBar2a->hide();
    }
    else
    {
        toolBar2a->show();
    }
}

void MainWindow3D::dark()
{
    glwidget1->joinTheDarkSide = !(glwidget1->joinTheDarkSide);
    glwidget2->joinTheDarkSide = !(glwidget2->joinTheDarkSide);

    glwidget1->resetColours();
    glwidget1->updateAll();

    glwidget2->resetColours();
    glwidget2->updateAll();
}

void MainWindow3D::createMenus()
{
    fileMenu1 = menuBar()->addMenu(tr("File_Left"));
    fileMenu1->addAction(open1Act);
    fileMenu1->addAction(openBatch1Act);
    fileMenu1->addAction(save1Act);
    fileMenu1->addAction(saveLim1Act);
    fileMenu1->addSeparator();
    fileMenu1->addAction(saveImg1Act);
    fileMenu1->addAction(saveBarAct);
    fileMenu1->addSeparator();
    fileMenu1->addAction(exitAct);

    fileMenu2 = menuBar()->addMenu(tr("File_Right"));
    fileMenu2->addAction(open2Act);
    fileMenu2->addAction(openBatch2Act);
    fileMenu2->addAction(save2Act);
    fileMenu2->addAction(saveLim2Act);
    fileMenu2->addSeparator();
    fileMenu2->addAction(saveImg2Act);

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(helpAct);
	helpMenu->addAction(fullScreenAct);
	helpMenu->addSeparator();
	helpMenu->addAction(aboutAct);
}

void MainWindow3D::updateNumVFsub1(void)
{
    if (glwidget1->meshCurr.size() > 0)
    {
        ctrlWidget1->numVsub->setText(QString::number(glwidget1->meshCurr[0]->my_numV/1000));
        ctrlWidget1->numFsub->setText(QString::number(glwidget1->meshCurr[0]->my_numF/1000));

        //for linSub
        ctrlWidget1->numV->setText(QString::number(glwidget1->meshCtrl[0]->my_numV));
        ctrlWidget1->numF->setText(QString::number(glwidget1->meshCtrl[0]->my_numF));
    }
}

void MainWindow3D::updateNumVFsub2(void)
{
    if (glwidget2->meshCurr.size() > 0)
    {
        ctrlWidget2->numVsub->setText(QString::number(glwidget1->meshCurr[0]->my_numV/1000));
        ctrlWidget2->numFsub->setText(QString::number(glwidget1->meshCurr[0]->my_numF/1000));

        //for linSub
        ctrlWidget2->numV->setText(QString::number(glwidget2->meshCtrl[0]->my_numV));
        ctrlWidget2->numF->setText(QString::number(glwidget2->meshCtrl[0]->my_numF));
    }
}

void MainWindow3D::updateNumVFsub1(int value)
{
    if (glwidget1->meshCurr.size() > 0)
    {
        ctrlWidget1->numVsub->setText(QString::number(glwidget1->meshCurr[0]->my_numV/1000));
        ctrlWidget1->numFsub->setText(QString::number(glwidget1->meshCurr[0]->my_numF/1000));

        //for linSub
        ctrlWidget1->numV->setText(QString::number(glwidget1->meshCtrl[0]->my_numV));
        ctrlWidget1->numF->setText(QString::number(glwidget1->meshCtrl[0]->my_numF));
    }
}

void MainWindow3D::updateNumVFsub2(int value)
{
    if (glwidget2->meshCurr.size() > 0)
    {
        ctrlWidget2->numVsub->setText(QString::number(glwidget1->meshCurr[0]->my_numV/1000));
        ctrlWidget2->numFsub->setText(QString::number(glwidget1->meshCurr[0]->my_numF/1000));

        //for linSub
        ctrlWidget2->numV->setText(QString::number(glwidget2->meshCtrl[0]->my_numV));
        ctrlWidget2->numF->setText(QString::number(glwidget2->meshCtrl[0]->my_numF));
    }
}

void MainWindow3D::connectAll()
{
	connect(glwidget1, SIGNAL(scaleChanged(double)),
			glwidget1, SLOT(setScale(double)));
	connect(glwidget1, SIGNAL(scaleChanged(double)),
			glwidget2, SLOT(setScale(double)));


	connect(glwidget2, SIGNAL(scaleChanged(double)),
			glwidget1, SLOT(setScale(double)));
	connect(glwidget2, SIGNAL(scaleChanged(double)),
			glwidget2, SLOT(setScale(double)));

	connect(glwidget1, SIGNAL(rotChanged(double, float, float, float)),
			glwidget1, SLOT(setRot(double, float, float, float)));
	connect(glwidget1, SIGNAL(rotChanged(double, float, float, float)),
            glwidget2, SLOT(setRot(double, float, float, float)));

	connect(glwidget2, SIGNAL(rotChanged(double, float, float, float)),
			glwidget1, SLOT(setRot(double, float, float, float)));
	connect(glwidget2, SIGNAL(rotChanged(double, float, float, float)),
			glwidget2, SLOT(setRot(double, float, float, float)));

	connect(glwidget1, SIGNAL(middleClicked()),
			this, SLOT(fullScr()));
	connect(glwidget2, SIGNAL(middleClicked()),
			this, SLOT(fullScr()));

//	connect(glwidget1, 	SIGNAL(updateNeeded()),
//			this, 		  SLOT(update1()));
//	connect(glwidget2, 	SIGNAL(updateNeeded()),
//			this, 		  SLOT(update2()));

	connect(glwidget1, 	SIGNAL(callBar(int)),
			glBar1, 		  SLOT(setBar(int)));
	connect(glwidget2, 	SIGNAL(callBar(int)),
			glBar2, 		  SLOT(setBar(int)));

	connect(ctrlWidget1->colorMenu, SIGNAL(currentIndexChanged(int)),
			glwidget1, SLOT(setClr(int)));
	connect(ctrlWidget1->colorMenu, SIGNAL(currentIndexChanged(int)),
			glBar1, SLOT(setBar(int)));
	connect(ctrlWidget2->colorMenu, SIGNAL(currentIndexChanged(int)),
			glwidget2, SLOT(setClr(int)));
	connect(ctrlWidget2->colorMenu, SIGNAL(currentIndexChanged(int)),
			glBar2, SLOT(setBar(int)));

	connect(ctrlWidget1->meshMenu, SIGNAL(currentIndexChanged(int)),
			glwidget1, SLOT(setIndMesh(int)));
	connect(ctrlWidget2->meshMenu, SIGNAL(currentIndexChanged(int)),
			glwidget2, SLOT(setIndMesh(int)));

	connect(glwidget1, 	SIGNAL(indexChanged(int)),
			glwidget1, 	  SLOT(setPoint(int)));
	connect(glwidget2, 	SIGNAL(indexChanged(int)),
			glwidget2, 	  SLOT(setPoint(int)));

	connect(ctrlWidget1->checkClear, 	SIGNAL(toggled(bool)),
			glwidget1, 			  SLOT(setClear(bool)));
	connect(ctrlWidget2->checkClear, 	SIGNAL(toggled(bool)),
			glwidget2, 			  SLOT(setClear(bool)));

    connect(glwidget1, 			SIGNAL(subdivLevelChanged(int)),
            ctrlWidget1->subdLevelSpinbox, 	  SLOT(setValue(int)));
	connect(ctrlWidget1->subdLevelSpinbox, 	SIGNAL(valueChanged(int)),
		   glwidget1, 			  SLOT(setSubdivLevel(int)));
    connect(ctrlWidget1->subdLevelSpinbox, 	SIGNAL(valueChanged(int)),
           this, 			  SLOT(updateNumVFsub1(int)));

    connect(glwidget2, 			SIGNAL(subdivLevelChanged(int)),
            ctrlWidget2->subdLevelSpinbox, 	  SLOT(setValue(int)));
	connect(ctrlWidget2->subdLevelSpinbox, 	SIGNAL(valueChanged(int)),
		   glwidget2, 			  SLOT(setSubdivLevel(int)));
    connect(ctrlWidget2->subdLevelSpinbox, 	SIGNAL(valueChanged(int)),
           this, 			  SLOT(updateNumVFsub2(int)));

	connect(ctrlWidget1->checkCtrl, 	SIGNAL(toggled(bool)),
			glwidget1, 			  SLOT(setShowCtrl(bool)));
    connect(ctrlWidget1->checkOld, 	SIGNAL(toggled(bool)),
            glwidget1, 			  SLOT(setShowOld(bool)));
    connect(ctrlWidget1->checkFeature, 	SIGNAL(toggled(bool)),
            glwidget1, 			  SLOT(setShowFeatureLines(bool)));
    connect(ctrlWidget1->checkLine, 	SIGNAL(toggled(bool)),
            glwidget1, 			  SLOT(setShowLine(bool)));
//	connect(ctrlWidget1->radioCtrlShaded, 	SIGNAL(toggled(bool)),
//			glwidget1, 			  SLOT(setShowShadedCtrl(bool)));
//	connect(ctrlWidget1->radioCtrlEdges, 	SIGNAL(toggled(bool)),
//			glwidget1, 			  SLOT(setShowEdgedCtrl(bool)));
//	connect(ctrlWidget1->radioCtrlCulled, 	SIGNAL(toggled(bool)),
//			glwidget1, 			  SLOT(setShowCulledCtrl(bool)));
	connect(ctrlWidget1->checkFull, 		SIGNAL(toggled(bool)),
			this, 				  SLOT(toggleMyLayout1(bool)));
	connect(glwidget1, 			SIGNAL(doubleClicked()),
			ctrlWidget1->checkFull, 		  SLOT(toggle()));

	connect(ctrlWidget2->checkCtrl, 	SIGNAL(toggled(bool)),
			glwidget2, 			  SLOT(setShowCtrl(bool)));
    connect(ctrlWidget2->checkOld, 	SIGNAL(toggled(bool)),
            glwidget2, 			  SLOT(setShowOld(bool)));
    connect(ctrlWidget2->checkFeature, 	SIGNAL(toggled(bool)),
            glwidget2, 			  SLOT(setShowFeatureLines(bool)));
    connect(ctrlWidget2->checkLine, 	SIGNAL(toggled(bool)),
            glwidget2, 			  SLOT(setShowLine(bool)));
//	connect(ctrlWidget2->radioCtrlShaded, 	SIGNAL(toggled(bool)),
//			glwidget2, 			  SLOT(setShowShadedCtrl(bool)));
//	connect(ctrlWidget2->radioCtrlEdges, 	SIGNAL(toggled(bool)),
//			glwidget2, 			  SLOT(setShowEdgedCtrl(bool)));
//	connect(ctrlWidget2->radioCtrlCulled, 	SIGNAL(toggled(bool)),
//			glwidget2, 			  SLOT(setShowCulledCtrl(bool)));
    connect(ctrlWidget2->checkFull, 		SIGNAL(toggled(bool)),
			this, 				  SLOT(toggleMyLayout2(bool)));
	connect(glwidget2, 			SIGNAL(doubleClicked()),
			ctrlWidget2->checkFull, 		  SLOT(toggle()));

	connect(ctrlWidget1->checkMesh, 	SIGNAL(toggled(bool)),
			glwidget1, 			  SLOT(setShowMesh(bool)));
    connect(ctrlWidget1->radioMeshFlat, 	SIGNAL(toggled(bool)),
            glwidget1, 			  SLOT(setShowFlatMesh(bool)));
    connect(ctrlWidget1->radioMeshSmooth, 	SIGNAL(toggled(bool)),
            glwidget1, 			  SLOT(setShowSmoothMesh(bool)));
	connect(ctrlWidget1->radioMeshEdges, 	SIGNAL(toggled(bool)),
			glwidget1, 			  SLOT(setShowEdgedMesh(bool)));
	connect(ctrlWidget1->radioMeshCulled, 	SIGNAL(toggled(bool)),
			glwidget1, 			  SLOT(setShowCulledMesh(bool)));
	connect(ctrlWidget1->radioMeshCurvM, 	SIGNAL(toggled(bool)),
			glwidget1, 			  SLOT(setShowCurvMMesh(bool)));
	connect(ctrlWidget1->radioMeshCurvG, 	SIGNAL(toggled(bool)),
			glwidget1, 			  SLOT(setShowCurvGMesh(bool)));
    connect(ctrlWidget1->radioMeshHeight, 	SIGNAL(toggled(bool)),
            glwidget1, 			  SLOT(setShowHeightMesh(bool)));
	connect(ctrlWidget1->radioMeshIP, 	SIGNAL(toggled(bool)),
			glwidget1, 			  SLOT(setShowIPMesh(bool)));

	connect(ctrlWidget2->checkMesh, 	SIGNAL(toggled(bool)),
			glwidget2, 			  SLOT(setShowMesh(bool)));
    connect(ctrlWidget2->radioMeshFlat, 	SIGNAL(toggled(bool)),
            glwidget2, 			  SLOT(setShowFlatMesh(bool)));
    connect(ctrlWidget2->radioMeshSmooth, 	SIGNAL(toggled(bool)),
            glwidget2, 			  SLOT(setShowSmoothMesh(bool)));
	connect(ctrlWidget2->radioMeshEdges, 	SIGNAL(toggled(bool)),
			glwidget2, 			  SLOT(setShowEdgedMesh(bool)));
	connect(ctrlWidget2->radioMeshCulled, 	SIGNAL(toggled(bool)),
			glwidget2, 			  SLOT(setShowCulledMesh(bool)));
	connect(ctrlWidget2->radioMeshCurvM, 	SIGNAL(toggled(bool)),
			glwidget2, 			  SLOT(setShowCurvMMesh(bool)));
	connect(ctrlWidget2->radioMeshCurvG, 	SIGNAL(toggled(bool)),
			glwidget2, 			  SLOT(setShowCurvGMesh(bool)));
    connect(ctrlWidget2->radioMeshHeight, 	SIGNAL(toggled(bool)),
            glwidget2, 			  SLOT(setShowHeightMesh(bool)));
	connect(ctrlWidget2->radioMeshIP, 	SIGNAL(toggled(bool)),
			glwidget2, 			  SLOT(setShowIPMesh(bool)));

    connect(this, 		SIGNAL(resetSX1(int)),
			ctrlWidget1->sliderX, 	  SLOT(setValue(int)));
	connect(this, 		SIGNAL(resetSY1(int)),
			ctrlWidget1->sliderY, 	  SLOT(setValue(int)));
	connect(this, 		SIGNAL(resetSZ1(int)),
			ctrlWidget1->sliderZ, 	  SLOT(setValue(int)));
	connect(this, 		SIGNAL(resetSX2(int)),
			ctrlWidget2->sliderX, 	  SLOT(setValue(int)));
	connect(this, 		SIGNAL(resetSY2(int)),
			ctrlWidget2->sliderY, 	  SLOT(setValue(int)));
	connect(this, 		SIGNAL(resetSZ2(int)),
			ctrlWidget2->sliderZ, 	  SLOT(setValue(int)));

	connect(glwidget1, 	SIGNAL(poiChanged()),
			this, 		  SLOT(resetSliders1()));
	connect(glwidget2, 	SIGNAL(poiChanged()),
			this, 		  SLOT(resetSliders2()));

	connect(ctrlWidget1->sliderX, 	SIGNAL(valueChanged(int)),
			glwidget1, 	  SLOT(movePointX(int)));
	connect(ctrlWidget1->sliderY, 	SIGNAL(valueChanged(int)),
			glwidget1, 	  SLOT(movePointY(int)));
	connect(ctrlWidget1->sliderZ, 	SIGNAL(valueChanged(int)),
			glwidget1, 	  SLOT(movePointZ(int)));
	connect(ctrlWidget1->sliderSm, 	SIGNAL(valueChanged(int)),
			glwidget1, 	  SLOT(changeSmoothing(int)));
	connect(ctrlWidget1->sliderRld, 	SIGNAL(valueChanged(int)),
			glwidget1, 	  SLOT(changeStripeDensity(int)));
//    connect(ctrlWidget1->sliderRld, 	SIGNAL(valueChanged(int)),
//            this, 	  SLOT(setTimeInt1(int)));
	connect(ctrlWidget1->sliderCurv1, 	SIGNAL(valueChanged(int)),
			glwidget1, 	  SLOT(changeCurvRatio1(int)));
	connect(ctrlWidget1->sliderCurv2, 	SIGNAL(valueChanged(int)),
			glwidget1, 	  SLOT(changeCurvRatio2(int)));
	connect(ctrlWidget2->sliderX, 	SIGNAL(valueChanged(int)),
			glwidget2, 	  SLOT(movePointX(int)));
	connect(ctrlWidget2->sliderY, 	SIGNAL(valueChanged(int)),
			glwidget2, 	  SLOT(movePointY(int)));
	connect(ctrlWidget2->sliderZ, 	SIGNAL(valueChanged(int)),
			glwidget2, 	  SLOT(movePointZ(int)));
	connect(ctrlWidget2->sliderSm, 	SIGNAL(valueChanged(int)),
			glwidget2, 	  SLOT(changeSmoothing(int)));
	connect(ctrlWidget2->sliderRld, 	SIGNAL(valueChanged(int)),
			glwidget2, 	  SLOT(changeStripeDensity(int)));
//    connect(ctrlWidget2->sliderRld, 	SIGNAL(valueChanged(int)),
//            this, 	  SLOT(setTimeInt2(int)));
	connect(ctrlWidget2->sliderCurv1, 	SIGNAL(valueChanged(int)),
			glwidget2, 	  SLOT(changeCurvRatio1(int)));
	connect(ctrlWidget2->sliderCurv2, 	SIGNAL(valueChanged(int)),
			glwidget2, 	  SLOT(changeCurvRatio2(int)));

	connect(ctrlWidget1->xpButton, SIGNAL(clicked()),
			glwidget1, SLOT(setRotXp()));
	connect(ctrlWidget1->xmButton, SIGNAL(clicked()),
			glwidget1, SLOT(setRotXm()));
	connect(ctrlWidget1->ypButton, SIGNAL(clicked()),
			glwidget1, SLOT(setRotYp()));
	connect(ctrlWidget1->ymButton, SIGNAL(clicked()),
			glwidget1, SLOT(setRotYm()));
	connect(ctrlWidget1->zpButton, SIGNAL(clicked()),
			glwidget1, SLOT(setRotZp()));
	connect(ctrlWidget1->zmButton, SIGNAL(clicked()),
			glwidget1, SLOT(setRotZm()));
	connect(ctrlWidget1->zerButton, SIGNAL(clicked()),
			glwidget1, SLOT(setRotZero()));

	connect(ctrlWidget2->xpButton, SIGNAL(clicked()),
			glwidget2, SLOT(setRotXp()));
	connect(ctrlWidget2->xmButton, SIGNAL(clicked()),
			glwidget2, SLOT(setRotXm()));
	connect(ctrlWidget2->ypButton, SIGNAL(clicked()),
			glwidget2, SLOT(setRotYp()));
	connect(ctrlWidget2->ymButton, SIGNAL(clicked()),
			glwidget2, SLOT(setRotYm()));
	connect(ctrlWidget2->zpButton, SIGNAL(clicked()),
			glwidget2, SLOT(setRotZp()));
	connect(ctrlWidget2->zmButton, SIGNAL(clicked()),
			glwidget2, SLOT(setRotZm()));
	connect(ctrlWidget2->zerButton, SIGNAL(clicked()),
			glwidget2, SLOT(setRotZero()));

	connect(ctrlWidget1->checkFrame, 		SIGNAL(toggled(bool)),
			glwidget1, 				  SLOT(setShowFrame(bool)));
	connect(ctrlWidget2->checkFrame, 		SIGNAL(toggled(bool)),
			glwidget2, 				  SLOT(setShowFrame(bool)));

	connect(ctrlWidget1->checkOnCtrl, 	SIGNAL(toggled(bool)),
			glwidget1, 			  SLOT(setProbeOnCtrl(bool)));
	connect(ctrlWidget2->checkOnCtrl, 	SIGNAL(toggled(bool)),
			glwidget2, 			  SLOT(setProbeOnCtrl(bool)));

    connect(ctrlWidget1->lapSmButton1, SIGNAL(clicked()),
            glwidget1, SLOT(lapSm1()));
    connect(ctrlWidget2->lapSmButton1, SIGNAL(clicked()),
            glwidget2, SLOT(lapSm1()));

    connect(ctrlWidget1->lapSmButton10, SIGNAL(clicked()),
            glwidget1, SLOT(lapSm10()));
    connect(ctrlWidget2->lapSmButton10, SIGNAL(clicked()),
            glwidget2, SLOT(lapSm10()));

    connect(ctrlWidget1->lapSmButton100, SIGNAL(clicked()),
            glwidget1, SLOT(lapSm100()));
    connect(ctrlWidget2->lapSmButton100, SIGNAL(clicked()),
            glwidget2, SLOT(lapSm100()));

    connect(ctrlWidget1->sliderLap,	SIGNAL(valueChanged(int)),
            glwidget1, 	  SLOT(changeLapSmValue(int)));
    connect(ctrlWidget2->sliderLap,	SIGNAL(valueChanged(int)),
            glwidget2, 	  SLOT(changeLapSmValue(int)));

    connect(glwidget1, 	SIGNAL(openFile(const char *)),
            this, 		  SLOT(load1(const char*)));
    connect(glwidget2, 	SIGNAL(openFile(const char *)),
            this, 		  SLOT(load2(const char*)));

    connect(ctrlWidget1->buffer2imgButton, 	SIGNAL(clicked()),
            glwidget1, 		  SLOT(buffer2img()));
    connect(ctrlWidget2->buffer2imgButton, 	SIGNAL(clicked()),
            glwidget2, 		  SLOT(buffer2img()));

    connect(glwidget1, 	SIGNAL(subdivToLevel(int)),
            ctrlWidget1->subdLevelSpinbox,	  SLOT(setValue(int)));
    connect(glwidget2, 	SIGNAL(subdivToLevel(int)),
            ctrlWidget2->subdLevelSpinbox,	  SLOT(setValue(int)));

    connect(ctrlWidget1->checkLab, SIGNAL(clicked(bool)),
            glwidget1, SLOT(setShowLab(bool)));
    connect(ctrlWidget2->checkLab, SIGNAL(clicked(bool)),
            glwidget2, SLOT(setShowLab(bool)));
}
