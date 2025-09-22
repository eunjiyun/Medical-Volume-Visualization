#include "ViewerSample.h"

#include "GraphicsClass.h"
#include "D3DClass.h"

//?袁⑹졐???紐낆굨 ?뚣끉??怨뺤춳??곸췅
//甕곌쑵?? 筌ｋ똾寃뺠쳸類ㅻ뮞, ??쎄쾿嚥▲끇而???疫꿸퀡???袁⑹졐??筌뤴뫁堉??筌욊낯??域밸챶??????됱벉
#include <QStyle>

//?遺얠쒔繹?筌롫뗄?놅쭪? ?곗뮆??
#include <QDebug>

//?袁⑹삺 ??볦퍢 揶쎛?紐꾩궎疫?
#include <QTime>

//Qt?癒?퐣 ?遺용뮞???쟿???온???類ｋ궖??揶쎛?紐꾩궎椰꾧퀡援???뽯선?????????롫뮉 ??삳쐭
#include <QScreen>

//Qt?癒?퐣 ??밸씜 筌롫뗄?놅쭪? 筌????遺욧맒?? ?袁⑹뒻 ???????롫뮉 ??삳쐭
#include <QMessageBox>

//Qt?癒?퐣 筌≪럩?????쁽 ??獄쏆뮇源??롫뮉 ??源?紐? 筌ｌ꼶???띾┛ ?袁る퉸 ?????롫뮉 ??삳쐭
#include <QCloseEvent>

//Qt?癒?퐣 ?遺얇늺(?遺용뮞???쟿???類ｋ궖???臾롫젏??띾┛ ?袁る퉸 ?????롫쐲 ??삳쐭
#include <QDesktopWidget>

ViewerSample::ViewerSample(QWidget * parent)
	: QMainWindow(parent)
	, ui(new Ui::ViewerSampleClass)


	, m_WindowSize(QSize(1280, 800))

	//筌ｋ똾寃뺠쳸類ㅻ뮞 ?袁⑹졐??揶쎛?귐뗪텕???????
	, m_pCbxDoFrames(new QCheckBox(this))
{
	//setupUi(this)??.ui ???뵬???類ㅼ벥??筌뤴뫀諭??袁⑹졐??this (ex: QMainWindow)???븐늿?졿??λ뜃由???臾믩씜
	ui->setupUi(this);

	//// System 揶쏆빘猿???밴쉐
	//System = new SystemClass;
	//
	//if (!System)
	//{
	//	return;
	//}

	//System->qtD3dWidget = ui->view;

	//// System 揶쏆빘猿??λ뜃由??獄???쎈뻬
	//if (System->Initialize())
	//{
	//	
	//	System->Run();
	//}

	//// System 揶쏆빘猿??ル굝利?獄?筌롫뗀?덄뵳?獄쏆꼹??
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