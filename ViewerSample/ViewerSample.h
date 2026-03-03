#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_ViewerSample.h"

class ViewerSample : public QMainWindow
{
	Q_OBJECT

public:
	ViewerSample(QWidget *parent = nullptr);
	~ViewerSample();

	void adjustWindowSize();
	void connectSlots();

private:
	void closeEvent(QCloseEvent * event) override;
public slots:
	void init(bool success);
	void tick();
	void render();
	void onBtnColorInvertClicked();
	void huValueChanged(int value);
	void transparencyValueChanged(int value);


	void brightnessCenterChanged(double value);  // Window Center
	void contrastWidthChanged(double value);   // Window Width
	void sharpnessChanged(int value);   // Window Width

	void volumeShowHide();
	void meshShowHide();
	void meshScaleSet();
private:
	Ui::ViewerSampleClass* ui;
	QDirect3D11Widget * m_pScene;
	QSize               m_WindowSize;

	float m_initialWindowCenter{ -1 }; // 데이터 로드 시 초기값 저장
	float m_initialWindowWidth{ -1 };  // 데이터 로드 시 초기값 저장
};
