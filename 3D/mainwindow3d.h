#ifndef MAINWINDOW3D_H
#define MAINWINDOW3D_H

#include <qmainwindow.h>
#include "3D/glviewsubd.h"
#include "3D/glbar.h"
#include "3D/control.h"

class QAction;
class QLabel;
class QMenu;
class QHBoxLayout;
class QVBoxLayout;
class QFrame;
class QSplitter;
class QScrollArea;
class QToolBar;

class MainWindow3D : public QMainWindow
{
	Q_OBJECT

public:
    MainWindow3D(GLuint iW, GLuint iH, cv::Mat *timg);
    ~MainWindow3D();

    GLviewsubd *glwidget1, *glwidget2;
    ControlW *ctrlWidget1, *ctrlWidget2;

signals:
	void resetSX1(int);
	void resetSY1(int);
	void resetSZ1(int);
	void resetSX2(int);
	void resetSY2(int);
	void resetSZ2(int);

public slots:
    void load1(std::istream &is);
    void load2(std::istream &is);
    void loadBatch1(std::istream &is);
    void loadBatch2(std::istream &is);

private slots:
	void open1();
	void open2();
	void openBatch1();
	void openBatch2();
	void save1();
	void save2();
	void saveLim1();
    void saveLim2();
	void saveImg1();
	void saveImg2();
	void saveBar();
	void help();
	void about();
	void fullScr();
	void setMyLayout(int view, bool full);
	void resetSliders1();
	void resetSliders2();
	void toggleMyLayout1(bool b);
	void toggleMyLayout2(bool b);
    void updateNumVFsub1(int);
    void updateNumVFsub2(int);
    void updateNumVFsub1();
    void updateNumVFsub2();


    void sub1();
    void tool1a();
    void tool2a();
    void dark();

private:
	void createActions();
	void createMenus();
	void connectAll();

	QWidget	 *widget;
	GLbar *glBar1, *glBar2;

	QHBoxLayout *horLayout;
	QVBoxLayout *mainLayout;


    QMenu *fileMenu1;
    QMenu *fileMenu2;
	QMenu *helpMenu;
    QAction *open1Act, *open2Act;
    QAction *openBatch1Act, *openBatch2Act;
	QAction *save1Act, *save2Act,
            *saveLim1Act, *saveLim2Act;
	QAction *saveImg1Act, *saveImg2Act;
	QAction *saveBarAct;
	QAction *exitAct;
	QAction *helpAct;
	QAction *aboutAct;
	QAction *fullScreenAct;
    QAction *sub1Act, *tool1aAct, *tool2aAct, *darkAct;
    QAction *panxpAct, *panxmAct, *panypAct, *panymAct;

    QSplitter	*horWidget;

	QFrame *vline;

	QLabel *label1, *label2;

    QPixmap* mainWindowPixmap;

    QToolBar    *toolBar1a,*toolBar2a;

	bool NotFullScr;

    QString pathToData;
};

#endif
