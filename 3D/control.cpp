#include <QtGui>
#include "control.h"

ControlW::ControlW(QGLWidget *colorBar, QToolBar *toolBara)
{
    buffer2imgButton = new QPushButton("ShowShadedResult", this);
    toolBara->addWidget(buffer2imgButton);

    ctrlGroupBox = new QGroupBox(tr(""));

    checkCtrl = new QCheckBox(tr("Control Mesh"));
    checkOld = new QCheckBox(tr("All Meshes"));
    checkFeature = new QCheckBox(tr("Feature Lines"));
    radioCtrlShaded = new QRadioButton(tr("Shaded"));
    radioCtrlEdges = new QRadioButton(tr("Wireframe"));
    radioCtrlCulled = new QRadioButton(tr("Solidframe"));
    checkFrame = new QCheckBox(tr("Image"));
    checkFull = new QCheckBox(tr("Full"));

    checkCtrl->setChecked(false);
    checkOld->setChecked(false);
    checkFeature->setChecked(false);
    radioCtrlEdges->setChecked(true);
    checkFrame->setChecked(false);
	checkFull->setChecked(true);

    checkOnCtrl = new QCheckBox(tr("MultiRes."));
	checkOnCtrl->setChecked(true);
    checkTransf = new QCheckBox(tr("Transform"));
    checkTransf->setChecked(true);

    checkClear = new QCheckBox(tr("Clear"));
	checkClear->setChecked(true);

	QVBoxLayout *ctrlL = new QVBoxLayout;
	ctrlGroupBox->setLayout(ctrlL);

/////////////////////////////////////////
    meshGroupBox = new QGroupBox(tr(""));

    checkMesh = new QCheckBox(tr("Subdiv. Mesh"));
    radioMeshFlat = new QRadioButton(tr("Flat"));
    radioMeshSmooth = new QRadioButton(tr("Smooth"));
    radioMeshEdges = new QRadioButton(tr("Wireframe"));
    radioMeshCulled = new QRadioButton(tr("Solidframe"));
    radioMeshHeight = new QRadioButton(tr("Height"));
    radioMeshIP = new QRadioButton(tr("Refl. Lines"));

	checkMesh->setChecked(true);
    radioMeshHeight->setChecked(true);

    toolBara->addWidget(checkCtrl);
    toolBara->addWidget(checkFeature);
    toolBara->addWidget(checkMesh);
    toolBara->addWidget(radioMeshFlat);
    toolBara->addWidget(radioMeshSmooth);
    toolBara->addWidget(radioMeshEdges);
    toolBara->addWidget(radioMeshCulled);
    toolBara->addWidget(radioMeshHeight);

/////////////////////////////////////////

	subdLevelSpinbox = new QSpinBox;
    subdLevelSpinbox->setRange(0, 5);
	subdLevelSpinbox->setSingleStep(1);
	subdLevelSpinbox->setValue(0);
    subdLevelSpinbox->setPrefix("Subd. level:   ");

	colorMenu = new QComboBox();
	colorMenu->clear();
    colorMenu->insertItem(colorMenu->count(),tr("Hue spectrum"));
    colorMenu->insertItem(colorMenu->count(),tr("Red-Green"));
    colorMenu->insertItem(colorMenu->count(),tr("Discrete"));
    colorMenu->insertItem(colorMenu->count(),tr("Grayscale"));
    colorMenu->insertItem(colorMenu->count(),tr("Hue (B-R)"));

    colorMenu->setCurrentIndex(3);

	meshMenu = new QComboBox();
	meshMenu->clear();
	meshMenu->insertItem(meshMenu->count(), "Mesh");

    miscGroupBox = new QGroupBox(tr(""));
    QVBoxLayout *miscL = new QVBoxLayout;
    toolBara->addWidget(checkClear);
    toolBara->addWidget(checkTransf);
    toolBara->addWidget(checkFrame);
    toolBara->addWidget(meshMenu);

    miscL->setContentsMargins(0,0,0,0);
    miscGroupBox->setContentsMargins(0,0,0,0);
    miscGroupBox->setLayout(miscL);
    toolBara->addWidget(subdLevelSpinbox);

/////////////////////////////////////////

    min = -100;
    max = 100;
    sstep = 1;
    pstep = 5;

    slidWidget = new QWidget;

	sliderX = new QSlider(Qt::Horizontal);
    sliderX->setSingleStep(sstep);
    sliderX->setPageStep(pstep);
    sliderX->setMinimum(min);
    sliderX->setMaximum(max);
	sliderX->setValue(0);
//	sliderX->setTickPosition(QSlider::TicksBothSides);
    sliderX->setTickPosition(QSlider::NoTicks);
//    sliderX->setTickInterval(10);

	sliderY = new QSlider(Qt::Horizontal);
    sliderY->setSingleStep(sstep);
    sliderY->setPageStep(pstep);
    sliderY->setMinimum(min);
    sliderY->setMaximum(max);
	sliderY->setValue(0);
//	sliderY->setTickPosition(QSlider::TicksBothSides);
    sliderY->setTickPosition(QSlider::NoTicks);
//    sliderY->setTickInterval(10);

	sliderZ = new QSlider(Qt::Horizontal);
    sliderZ->setSingleStep(sstep);
    sliderZ->setPageStep(pstep);
    sliderZ->setMinimum(min);
    sliderZ->setMaximum(max);
	sliderZ->setValue(0);
//	sliderZ->setTickPosition(QSlider::TicksBothSides);
    sliderZ->setTickPosition(QSlider::NoTicks);
//    sliderZ->setTickInterval(10);

	sliderSm = new QSlider(Qt::Horizontal);
	sliderSm->setSingleStep(1);
	sliderSm->setPageStep(1);
	sliderSm->setMinimum(0);
    sliderSm->setMaximum(4);
	sliderSm->setValue(0);
//	sliderSm->setTickPosition(QSlider::TicksBothSides);
    sliderSm->setTickPosition(QSlider::NoTicks);
//    sliderSm->setTickInterval(1);

	sliderRld = new QSlider(Qt::Horizontal);
	sliderRld->setSingleStep(1);
	sliderRld->setPageStep(1);
    sliderRld->setMinimum(1);
    sliderRld->setMaximum(20);
    sliderRld->setValue(6);
//	sliderRld->setTickPosition(QSlider::TicksBothSides);
    sliderRld->setTickPosition(QSlider::NoTicks);
//	sliderRld->setTickInterval(2);

	xpButton = new QPushButton("X+", this);
	xmButton = new QPushButton("X-", this);
	ypButton = new QPushButton("Y+", this);
	ymButton = new QPushButton("Y-", this);
	zpButton = new QPushButton("Z+", this);
	zmButton = new QPushButton("Z-", this);

	zerButton = new QPushButton("0", this);

	int wi = 25;
	int he = 20;

	xpButton->setMaximumSize(wi, he);
	xmButton->setMaximumSize(wi, he);
	ypButton->setMaximumSize(wi, he);
	ymButton->setMaximumSize(wi, he);
	zpButton->setMaximumSize(wi, he);
	zmButton->setMaximumSize(wi, he);
	zerButton->setMaximumSize(wi, he);

	QVBoxLayout *slidLayout = new QVBoxLayout;

	QLabel *xLabel = new QLabel("x");
	QLabel *yLabel = new QLabel("y");
	QLabel *zLabel = new QLabel("z");
	QLabel *smLabel = new QLabel("s");
	QLabel *rldLabel = new QLabel("r");
	QLabel *c1Label = new QLabel("C");
	QLabel *c2Label = new QLabel("c");

	xLabel->setText("<font color='red'>x</font>");
	yLabel->setText("<font color='green'>y</font>");
	zLabel->setText("<font color='blue'>z</font>");
    smLabel->setText("<font color='grey'>s</font>");
    rldLabel->setText("<font color='grey'>r</font>");
    c1Label->setText("<font color='grey'>C</font>");
    c2Label->setText("<font color='grey'>c</font>");

	QWidget *sliderXw = new QWidget;
	QWidget *sliderYw = new QWidget;
	QWidget *sliderZw = new QWidget;
	QWidget *sliderRldw = new QWidget;

	QHBoxLayout *sliderXl = new QHBoxLayout;
	QHBoxLayout *sliderYl = new QHBoxLayout;
	QHBoxLayout *sliderZl = new QHBoxLayout;
//	QHBoxLayout *sliderSml = new QHBoxLayout;
	QHBoxLayout *sliderRldl = new QHBoxLayout;
	sliderXl->addWidget(xLabel);
	sliderXl->addWidget(sliderX);
    sliderXl->setContentsMargins(11,0,11,0);
	sliderXw->setLayout(sliderXl);
	sliderYl->addWidget(yLabel);
	sliderYl->addWidget(sliderY);
    sliderYl->setContentsMargins(11,0,11,0);
	sliderYw->setLayout(sliderYl);
	sliderZl->addWidget(zLabel);
	sliderZl->addWidget(sliderZ);
    sliderZl->setContentsMargins(11,0,11,0);
	sliderZw->setLayout(sliderZl);

	sliderRldl->addWidget(rldLabel);
	sliderRldl->addWidget(sliderRld);
    sliderRldl->addWidget(zerButton);
    sliderRldl->setContentsMargins(11,0,11,0);
	sliderRldw->setLayout(sliderRldl);
//    toolBara->addWidget(sliderSm);

    toolBara->addWidget(sliderXw);
    toolBara->addWidget(sliderYw);
    toolBara->addWidget(sliderZw);

    gridL = new QGridLayout;
    gridW = new QWidget;

    gridL->addWidget(xmButton, 2, 1);
    gridL->addWidget(xpButton, 2, 3);
    gridL->addWidget(ymButton, 3, 2);
    gridL->addWidget(ypButton, 1, 2);
    gridL->addWidget(zerButton, 2, 2);
    gridW->setLayout(gridL);

    toolBara->addWidget(gridW);

    slidLayout->setContentsMargins(0,0,0,0);
    slidWidget->setContentsMargins(0,0,0,0);
	slidWidget->setLayout(slidLayout);

/////////////////////////////////////////

    QHBoxLayout *barLayout = new QHBoxLayout;
    barWidget = new QWidget;
//    barLayout->addStretch(1);
    barLayout->addWidget(colorBar);
//    barLayout->addStretch(1);
    barWidget->setLayout(barLayout);

    numV = new QLineEdit;
    numF = new QLineEdit;
    numVsub = new QLineEdit;
    numFsub = new QLineEdit;
    numV->setFixedWidth(55);
    numF->setFixedWidth(55);
    numVsub->setFixedWidth(35);
    numFsub->setFixedWidth(35);

    QLabel *numVl = new QLabel("#V: ");
    QLabel *numFl = new QLabel("#F: ");
    QLabel *numVsubl = new QLabel("#V(K): ");
    QLabel *numFsubl = new QLabel("#F(K): ");

    QHBoxLayout *numLayout = new QHBoxLayout;
    numWidget = new QWidget;
//    numLayout->addStretch(1);
    numLayout->addWidget(numVl);
    numLayout->addWidget(numV);
    numLayout->addWidget(numFl);
    numLayout->addWidget(numF);
//    numLayout->addStretch(1);
//    numLayout->setContentsMargins(0,0,0,0);
    numWidget->setLayout(numLayout);
    numWidget->setMaximumWidth(200);

    QHBoxLayout *numSLayout = new QHBoxLayout;
    numSWidget = new QWidget;
//    numSLayout->addStretch(1);
    numSLayout->addWidget(numVsubl);
    numSLayout->addWidget(numVsub);
    numSLayout->addWidget(numFsubl);
    numSLayout->addWidget(numFsub);
//    numSLayout->addStretch(1);
//    numSLayout->setContentsMargins(0,0,0,0);
    numSWidget->setLayout(numSLayout);
    numSWidget->setMaximumWidth(200);

    toolBara->addWidget(colorMenu);
    toolBara->addWidget(barWidget);

    checkLab = new QCheckBox(tr("Lab space"));
    checkLab->setChecked(false);
    toolBara->addWidget(checkLab);

/////////////////////////////////////////

    QVBoxLayout *layout = new QVBoxLayout;

    layout->setContentsMargins(0,0,0,0);
    this->setContentsMargins(0,0,0,0);
    this->setLayout(layout);
}

ControlW::~ControlW()
{

}


