#include "ViewerSample.h"

#include "GraphicsClass.h"
#include "D3DClass.h"

//?熬곣뫗議???筌뤿굞援???ｋ걠???⑤벡異??怨몄톭
//?뺢퀗??? 嶺뚳퐢?얍칰類좎낯筌먦끇裕? ???꾩씩?β뼯?뉓????リ옇????熬곣뫗議??嶺뚮ㅄ維곩젆??嶺뚯쉳????잙갭梨???????깅쾳
#include <QStyle>

//??븐뼚?붺뭐?嶺뚮∥???낆?? ?怨쀫츊??
#include <QDebug>

//?熬곣뫗????蹂?뜟 ?띠럾??筌뤾쑴沅롧뼨?
#include <QTime>

//Qt???????븐슜裕????????㉱???筌먲퐢沅???띠럾??筌뤾쑴沅롦ㅀ袁㏉→뤃???戮?꽑?????????濡ル츎 ???녹맠
#include <QScreen>

//Qt???????諛몄뵜 嶺뚮∥???낆?? 嶺?????븐슙留?? ?熬곣뫗?????????濡ル츎 ???녹맠
#include <QMessageBox>

//Qt?????嶺뚢돦???????????꾩룇裕뉑틦??濡ル츎 ???繹?筌? 嶺뚳퐣瑗????얄뵛 ?熬곥굥???????濡ル츎 ???녹맠
#include <QCloseEvent>

//Qt???????븐뻼????븐슜裕???????筌먲퐢沅????얜∥????얄뵛 ?熬곥굥???????濡レ맪 ???녹맠
#include <QDesktopWidget>

ViewerSample::ViewerSample(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::ViewerSampleClass)


	, m_WindowSize(QSize(1280, 800))

	//嶺뚳퐢?얍칰類좎낯筌먦끇裕??熬곣뫗議???띠럾??洹먮뿪????????
	, m_pCbxDoFrames(new QCheckBox(this))
{
	//setupUi(this)??.ui ???逾???筌먦끉踰??嶺뚮ㅄ維獄??熬곣뫗議??this (ex: QMainWindow)???釉먮듌?議온??貫?껆뵳????얜???
	ui->setupUi(this);

	//// System ?띠룇鍮섊뙼???諛댁뎽
	//System = new SystemClass;
	//
	//if (!System)
	//{
	//	return;
	//}

	//System->qtD3dWidget = ui->view;

	//// System ?띠룇鍮섊뙼??貫?껆뵳???????덈뺄
	//if (System->Initialize())
	//{
	//	
	//	System->Run();
	//}

	//// System ?띠룇鍮섊뙼???リ턁筌???嶺뚮∥???꾨뎨??꾩룇瑗??
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
	System->m_Graphics->m_Direct3D->qtD3dWidget->release();
	QTime dieTime = QTime::currentTime().addMSecs(500);
	while (QTime::currentTime() < dieTime)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

	event->accept();
}