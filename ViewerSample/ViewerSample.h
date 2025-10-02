#pragma once

#include <QtWidgets/QMainWindow>
#include <QCheckBox>
#include "ui_ViewerSample.h"

class ViewerSample : public QMainWindow
{
    Q_OBJECT

public:
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
