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
	
private:
    Ui::ViewerSampleClass* ui;
    QDirect3D11Widget * m_pScene;
    QSize               m_WindowSize;
};
