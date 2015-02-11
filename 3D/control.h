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
                    *lapSmButton1, *lapSmButton10, *lapSmButton100,
                    *buffer2imgButton;
    QComboBox 		*colorMenu, *meshMenu;
   	QSpinBox 		*subdLevelSpinbox;
   	
	QSlider 		*sliderX, *sliderY, *sliderZ, *sliderSm, *sliderRld, 
                    *sliderCurv1, *sliderCurv2, *sliderLap;

	QRadioButton 	*radioCtrlShaded, *radioCtrlEdges, *radioCtrlCulled,
                    *radioMeshFlat, *radioMeshSmooth, *radioMeshEdges, *radioMeshCulled,
                    *radioMeshCurvM, *radioMeshCurvG, *radioMeshCurvTG, *radioMeshHeight, *radioMeshIP;
 	
    QCheckBox 		*checkCtrl, *checkOld, *checkFrame, *checkFull, *checkMesh,
                    *checkOnCtrl,
                    *checkTransf, *checkClear, *checkFeature, *checkLine,
                    *checkLab;
 	
    QGroupBox 		*ctrlGroupBox, *meshGroupBox, *miscGroupBox, *laplGroupBox;
 	
 	QFrame			*line;
 	
    int             min, max, sstep, pstep;

    QLineEdit       *numV, *numF, *numVsub, *numFsub;

    QWidget         *slidWidget, *barWidget, *numWidget, *numSWidget, *restWidget, *gridW;

    QGridLayout     *gridL;
};

#endif
