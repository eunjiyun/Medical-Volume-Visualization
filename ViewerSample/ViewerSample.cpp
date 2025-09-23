#include "ViewerSample.h"

#include "GraphicsClass.h"
#include "D3DClass.h"

//??ш끽維쀨????嶺뚮ㅏ援욄뤃???節뗪콬????ㅻ깹????⑤챷??
//?類???? 癲ル슪???띿물筌먯쥙??춯癒?걞獒? ???袁⑹뵫?棺堉??벬?????れ삀?????ш끽維쀨???癲ル슢?꾤땟怨⑹젂??癲ル슣??????숆강筌???????源낆쓱
#include <QStyle>

//??釉먮폏?遺븍춴?癲ル슢??????? ??⑥レ툓??
#include <QDebug>

//??ш끽維????癰?????좊읈??嶺뚮ㅎ?닸쾮濡㏓섀?
#include <QTime>

//Qt???????釉먯뒠獒?????????굿???嶺뚮㉡?€쾮????좊읈??嶺뚮ㅎ?닸쾮濡╉?熬곥룊??믩쨨???筌?苑?????????嚥▲꺂痢?????밸쭬
#include <QScreen>

//Qt???????獄쏅챷逾?癲ル슢??????? 癲?????釉먯뒜筌?? ??ш끽維?????????嚥▲꺂痢?????밸쭬
#include <QMessageBox>

//Qt?????癲ル슓????????????袁⑸즵獒뺣뎾???嚥▲꺂痢????濚?嶺? 癲ル슪?ｇ몭?????꾨탿 ??ш낄援???????嚥▲꺂痢?????밸쭬
#include <QCloseEvent>

//Qt???????釉먮뻤????釉먯뒠獒???????嶺뚮㉡?€쾮?????쒋닪?????꾨탿 ??ш낄援???????嚥▲꺃留?????밸쭬
#include <QDesktopWidget>

ViewerSample::ViewerSample(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::ViewerSampleClass)


	, m_WindowSize(QSize(1280, 800))

	//癲ル슪???띿물筌먯쥙??춯癒?걞獒???ш끽維쀨?????좊읈??域밸Ŧ肉????????
	, m_pCbxDoFrames(new QCheckBox(this))
{
	//setupUi(this)??.ui ???????嶺뚮Ĳ?됭린??癲ル슢?꾤땟?????ш끽維쀨???this (ex: QMainWindow)????됰Ŧ??鈺곗삩???縕?猿녿뎨????????
	ui->setupUi(this);

	//// System ??좊즵??꼯????獄쏅똻??
	//System = new SystemClass;
	//
	//if (!System)
	//{
	//	return;
	//}

	//System->qtD3dWidget = ui->view;

	//// System ??좊즵??꼯???縕?猿녿뎨????????덈틖
	//if (System->Initialize())
	//{
	//	
	//	System->Run();
	//}

	//// System ??좊즵??꼯?????ろ꼤嶺???癲ル슢?????袁⑤렓??袁⑸즵???
	//System->Shutdown();
	//delete System;
	//System = nullptr;

	m_pScene = ui->view;
	m_pScene = ui->view;

	adjustWindowSize();
	addToolbarWidgets();
	connectSlots();
}

ViewerSample::~ViewerSample() = default;

void ViewerSample::adjustWindowSize()
{
	resize(m_WindowSize.width(), m_WindowSize.height());
	setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(),
		qApp->screens().first()->availableGeometry()));
}

void ViewerSample::addToolbarWidgets()
{
	// Add CheckBox to tool-bar to stop/continue frames execution.
	m_pCbxDoFrames->setText("Do Frames");
	m_pCbxDoFrames->setChecked(true);
	connect(m_pCbxDoFrames, &QCheckBox::stateChanged, [&] {
		if (m_pCbxDoFrames->isChecked())
			//System->m_Graphics->m_Direct3D->qtD3dWidget
			//m_pScene->continueFrames();
			System->m_Graphics->m_Direct3D->qtD3dWidget->continueFrames();
		else
			//System->m_Graphics->m_Direct3D->qtD3dWidget
			//m_pScene->pauseFrames();
			System->m_Graphics->m_Direct3D->qtD3dWidget->pauseFrames();
		});
	ui->mainToolBar->addWidget(m_pCbxDoFrames);
}

void ViewerSample::connectSlots()
{
	connect(m_pScene, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);
	connect(m_pScene, &QDirect3D11Widget::ticked, this, &ViewerSample::tick);
	connect(m_pScene, &QDirect3D11Widget::rendered, this, &ViewerSample::render);

	/*connect(System->m_Graphics->m_Direct3D->qtD3dWidget, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);
	connect(System->m_Graphics->m_Direct3D->qtD3dWidget, &QDirect3D11Widget::ticked, this, &ViewerSample::tick);
	connect(System->m_Graphics->m_Direct3D->qtD3dWidget, &QDirect3D11Widget::rendered, this, &ViewerSample::render);*/

	//connect(System->qtD3dWidget, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);
	//connect(System->qtD3dWidget, &QDirect3D11Widget::ticked, this, &ViewerSample::tick);
	//connect(System->qtD3dWidget, &QDirect3D11Widget::rendered, this, &ViewerSample::render);

	// NOTE: Additionally, you can listen to some basic IO events.
	// connect(m_pScene, &QDirect3D11Widget::keyPressed, this, &ViewerSample::onKeyPressed);
	// connect(m_pScene, &QDirect3D11Widget::mouseMoved, this, &ViewerSample::onMouseMoved);
	// connect(m_pScene, &QDirect3D11Widget::mouseClicked, this, &ViewerSample::onMouseClicked);
	// connect(m_pScene, &QDirect3D11Widget::mouseReleased, this,
	// &ViewerSample::onMouseReleased);
}

void ViewerSample::init(bool success)
{
	if (!success)
	{
		QMessageBox::critical(this, "ERROR", "Direct3D widget initialization failed.",
			QMessageBox::Ok);
		return;
	}

	// TODO: Add here your extra initialization here.
	// ...

	// Start processing frames with a short delay in case things are still initializing/loading
	// in the background.


	QTimer::singleShot(500, this, [&] { m_pScene->run(); });
	disconnect(m_pScene, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);


	//QTimer::singleShot(500, this, [&] { System->m_Graphics->m_Direct3D->qtD3dWidget->run(); });
	//disconnect(System->m_Graphics->m_Direct3D->qtD3dWidget, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);
}

void ViewerSample::tick()
{
	// TODO: Update the scene here.
	// m_pMesh->Tick();
}

void ViewerSample::render()
{
	// TODO: Present the scene here.
	// m_pMesh->Render();
}

void ViewerSample::closeEvent(QCloseEvent * event)
{
	event->ignore();
	//System->m_Graphics->m_Direct3D->qtD3dWidget
	//m_pScene->release();
	//System->m_Graphics->m_Direct3D->qtD3dWidget->release();
	QTime dieTime = QTime::currentTime().addMSecs(500);
	while (QTime::currentTime() < dieTime)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

	event->accept();
}