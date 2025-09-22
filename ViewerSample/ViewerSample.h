#pragma once

#include <QtWidgets/QMainWindow>
#include <QCheckBox>
#include "ui_ViewerSample.h"
#include "SystemClass.h"
class ViewerSample : public QMainWindow
{
    Q_OBJECT

public:

	//Q_NULLPTR?? Qt??甕곌쑴???紐낆넎??筌띲끋寃뺞에?
	//nullptr?? C++11??꾩뜎???類ㅻ뻼 ??쇱뜖??뺤쨮 ??????됱읈?源놁뵠 ???ル뿭??
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
	SystemClass* System = nullptr;
	QDirect3D11Widget * m_pScene;
	QSize               m_WindowSize;
	QCheckBox *         m_pCbxDoFrames;

};
