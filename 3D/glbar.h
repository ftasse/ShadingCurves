#ifndef GLBAR_H
#define GLBAR_H

#include <QSize>
#include <iostream>
#include "3D/glabstract.h"

class GLbar : public GLabstract
{
    Q_OBJECT

public:
    GLbar(QWidget *parent = 0);
    ~GLbar();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;	

   	void buildBar(void);
	
public slots:
	void drawBar(void);
	void setBar(int color);

protected:
    void initializeGL(void);
    void paintGL(void);

private:
	bool bar_enabled;

	unsigned int bar_list;
};

#endif
