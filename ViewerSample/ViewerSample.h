#pragma once

#include <QtWidgets/QMainWindow>
#include <QCheckBox>
#include "ui_ViewerSample.h"

class ViewerSample : public QMainWindow
{
    Q_OBJECT

public:

	//Q_NULLPTR?? Qt???筌??????癲ル슢?뤸뤃????꿔꺂????關臾쇘춯癒?돵??
	//nullptr?? C++11???ш끽維????癲ル슢캉??쏆춿????繹먮굞留??嶺뚮Ĳ?됲걫 ???????繹먮냱踰?嚥싲갭큔?源겶???????щ였??
    ViewerSample(QWidget *parent = nullptr);
    ~ViewerSample();

	void adjustWindowSize();
	void addToolbarWidgets();
	void connectSlots();

private:
	void closeEvent(QCloseEvent * event) override;
public slots:
	void init(bool success);
	void tick();
	void render();

private:
    Ui::ViewerSampleClass* ui;

	QDirect3D11Widget * m_pScene;
	QSize               m_WindowSize;
	QCheckBox *         m_pCbxDoFrames;

};
