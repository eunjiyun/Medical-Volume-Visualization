#pragma once

#include <QtWidgets/QMainWindow>
#include <QCheckBox>
#include "ui_ViewerSample.h"
#include "SystemClass.h"
class ViewerSample : public QMainWindow
{
    Q_OBJECT

public:

	//Q_NULLPTR?? Qt???類?????嶺뚮ㅏ援???癲ル슢???μ물筌먯쉸肉?
	//nullptr?? C++11??熬곣뫖????嶺뚮Ĳ?뉛쭛????源낆맫??筌먦끉큔 ???????源놁벁?濚밸Ŧ?깁??????レ뿴??
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
