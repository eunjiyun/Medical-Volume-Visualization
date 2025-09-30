#include "ViewerSample.h"

//#include "GraphicsClass.h"
//#include "D3DClass.h"

//????썹땟?????癲ル슢?뤸뤃?꾨쨨???影?れ쉬?????산뭐?????ㅼ굣??
//?筌????? ?꿔꺂?????용Ъ嶺뚮Ŋ伊??異?솒?嫄욅뜏? ???熬곣뫗逾?汝뷴젆??踰???????뚯????????썹땟?????꿔꺂??袁ㅻ븶?ⓥ뫗????꿔꺂????????녾컯嶺???????繹먮굞??
#include <QStyle>

//???됰Ŧ???븍툖異??꿔꺂???????? ???Β?ы닍??
#include <QDebug>

//????썹땟??????????醫딆쓧??癲ル슢???몄쒜嚥▲룗??
#include <QTime>

//Qt????????됰Ŋ?좂뜏?????????援온???癲ル슢???ъ쒜????醫딆쓧??癲ル슢???몄쒜嚥△븠??ш낄猷??誘⑹Ŀ???嶺????????????β뼯爰귨㎘?????諛몄?
#include <QScreen>

//Qt????????꾩룆梨룬??꿔꺂???????? ???????됰Ŋ?쒐춯?? ????썹땟??????????β뼯爰귨㎘?????諛몄?
#include <QMessageBox>

//Qt??????꿔꺂?????????????熬곣뫖利든뜏類ｋ렱????β뼯爰귨㎘????嚥?癲? ?꿔꺂??節뉖き?????袁⑦꺙 ????꾣뤃????????β뼯爰귨㎘?????諛몄?
#include <QCloseEvent>

//Qt????????됰Ŧ六?????됰Ŋ?좂뜏???????癲ル슢???ъ쒜??????뗫떔?????袁⑦꺙 ????꾣뤃????????β뼯爰껓쭕?????諛몄?
#include <QDesktopWidget>

ViewerSample::ViewerSample(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::ViewerSampleClass)


	, m_WindowSize(QSize(1280, 800))

	//?꿔꺂?????용Ъ嶺뚮Ŋ伊??異?솒?嫄욅뜏?????썹땟??????醫딆쓧???잙갭큔?????????
	, m_pCbxDoFrames(new QCheckBox(this))
{
	//setupUi(this)??.ui ???????癲ル슢캉???┛???꿔꺂??袁ㅻ븶???????썹땟????this (ex: QMainWindow)?????거???브퀣????潁??용끏?????????
	ui->setupUi(this);

	//// System ??醫딆┻??瑗?????꾩룆???
	//System = new SystemClass;
	//
	//if (!System)
	//{
	//	return;
	//}

	//System->qtD3dWidget = ui->view;

	//// System ??醫딆┻??瑗???潁??용끏??????????덊떀
	//if (System->Initialize())
	//{
	//	
	//	System->Run();
	//}

	//// System ??醫딆┻??瑗??????띻샴癲????꿔꺂??????熬곣뫀???熬곣뫖利???
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
			m_pScene->continueFrames();
			//System->m_Graphics->m_Direct3D->qtD3dWidget->continueFrames();
		else
			//System->m_Graphics->m_Direct3D->qtD3dWidget
			m_pScene->pauseFrames();
			//System->m_Graphics->m_Direct3D->qtD3dWidget->pauseFrames();
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