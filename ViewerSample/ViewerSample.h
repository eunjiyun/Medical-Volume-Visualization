#pragma once

#include <QtWidgets/QMainWindow>
#include <QCheckBox>
#include "ui_ViewerSample.h"
#include "SystemClass.h"
class ViewerSample : public QMainWindow
{
    Q_OBJECT

public:

	//Q_NULLPTR?? Qt???ï§??????ï¦«ëš®?æ´????²ãƒ«????Î¼ë¬¼ç­Œë¨?‰¸??
	//nullptr?? C++11???¬ê³£ë«????ï¦«ëš®Ä²??›ì­›????æºë‚†ë§??ç­Œë¨¦?‰í” ???????æºë†ë²?æ¿šë°¸Å¦?ê¹Â€???????¬ë¿´??
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
