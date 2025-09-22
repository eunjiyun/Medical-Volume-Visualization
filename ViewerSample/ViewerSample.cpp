#include "ViewerSample.h"

#include "GraphicsClass.h"
#include "D3DClass.h"

//?è¢â‘¹ì¡???ï§ë‚†êµ???£ë‰???¨ëº¤ì¶??ê³¸ì·…
//?•ê³Œ??? ç­Œï½‹?¾å¯ƒëº ì³¸ï§ã…»ë®? ???„ì¾¿?¥â–²?‡è€????«ê¿¸????è¢â‘¹ì¡??ç­Œë¤´ë«å ‰??ç­ŒìšŠ????Ÿë°¸ì±???????±ë²‰
#include <QStyle>

//??ºì– ?”ç¹¹?ç­Œë¡«???…ì?? ?ê³—ë®†??
#include <QDebug>

//?è¢â‘¹????ë³?¢ ?¶ì›??ï§ê¾©ê¶ç–«?
#include <QTime>

//Qt???????ºìš©ë®????????¨Â€???ï§ï½‹ê¶???¶ì›??ï§ê¾©ê¶æ¤°ê¾§í€¡æ´???ë½?„ ?????????ë¡«ë®‰ ???³ì­
#include <QScreen>

//Qt???????ë°¸ì”œ ç­Œë¡«???…ì?? ç­?????ºìš§ë§?? ?è¢â‘¹?????????ë¡«ë®‰ ???³ì­
#include <QMessageBox>

//Qt?????ç­Œâ‰ª???????????„ì†ë®‡æº??ë¡«ë®‰ ???æº?ï§? ç­Œï½Œê¼????¾â”› ?è¢ã‚‹???????ë¡«ë®‰ ???³ì­
#include <QCloseEvent>

//Qt???????ºì–‡????ºìš©ë®???????ï§ï½‹ê¶????¾ë¡«????¾â”› ?è¢ã‚‹???????ë¡«ì² ???³ì­
#include <QDesktopWidget>

ViewerSample::ViewerSample(QWidget * parent)
	: QMainWindow(parent)
	, ui(new Ui::ViewerSampleClass)


	, m_WindowSize(QSize(1280, 800))

	//ç­Œï½‹?¾å¯ƒëº ì³¸ï§ã…»ë®??è¢â‘¹ì¡???¶ì›??ê·ë—ª????????
	, m_pCbxDoFrames(new QCheckBox(this))
{
	//setupUi(this)??.ui ???ëµ???ï§ã…¼ë²??ç­Œë¤´ë«€è«??è¢â‘¹ì¡??this (ex: QMainWindow)???ë¸ëŠ¿?ì¡¿Â€??Î»?ƒç”±????¾ë???
	ui->setupUi(this);

	//// System ?¶ì†ë¹˜çŒ¿???ë°´ì‰
	//System = new SystemClass;
	//
	//if (!System)
	//{
	//	return;
	//}

	//System->qtD3dWidget = ui->view;

	//// System ?¶ì†ë¹˜çŒ¿??Î»?ƒç”±???????ˆë»¬
	//if (System->Initialize())
	//{
	//	
	//	System->Run();
	//}

	//// System ?¶ì†ë¹˜çŒ¿???«êµï§???ç­Œë¡«?€??„ëµ³??„ì†ê¼??
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