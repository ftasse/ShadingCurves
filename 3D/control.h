#ifndef CONTROL_H
#define CONTROL_H

#include <QWidget>

#include <QGLWidget>

class QGroupBox;
class QPushButton;
class QComboBox;
class QSpinBox;
class QSlider;
class QFrame;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class QScrollArea;
class QToolBar;
class QGridLayout;

class ControlW : public QWidget
{
	Q_OBJECT
	
public slots:

public:
    ControlW(QGLWidget *colorBar, QToolBar *toolBara);
   ~ControlW();

    QPushButton 	*subdivButton,
                    *zeroButton,
					*xpButton, *xmButton, *ypButton, *ymButton, 
                    *zpButton, *zmButton, *zerButton,
                    *buffer2imgButton;
    QComboBox 		*colorMenu, *meshMenu;
   	QSpinBox 		*subdLevelSpinbox;
   	
    QSlider 		*sliderX, *sliderY, *sliderZ, *sliderSm, *sliderRld;

	QRadioButton 	*radioCtrlShaded, *radioCtrlEdges, *radioCtrlCulled,
                    *radioMeshFlat, *radioMeshSmooth, *radioMeshEdges, *radioMeshCulled,
                    *radioMeshHeight, *radioMeshIP;
 	
    QCheckBox 		*checkCtrl, *checkOld, *checkFrame, *checkFull, *checkMesh,
                    *checkOnCtrl,
                    *checkTransf, *checkClear, *checkFeature,
                    *checkLab;
 	
    QGroupBox 		*ctrlGroupBox, *meshGroupBox, *miscGroupBox;
 	
 	QFrame			*line;
 	
    int             min, max, sstep, pstep;

    QLineEdit       *numV, *numF, *numVsub, *numFsub;

    QWidget         *slidWidget, *barWidget, *numWidget, *numSWidget, *restWidget, *gridW;

    QGridLayout     *gridL;
};

#endif
