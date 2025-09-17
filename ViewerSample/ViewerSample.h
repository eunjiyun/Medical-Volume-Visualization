#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ViewerSample.h"

class ViewerSample : public QMainWindow
{
    Q_OBJECT

public:
    ViewerSample(QWidget *parent = nullptr);
    ~ViewerSample();

private:
    Ui::ViewerSampleClass ui;
};
