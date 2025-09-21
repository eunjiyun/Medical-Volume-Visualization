#include "ViewerSample.h"

#include "GraphicsClass.h"
#include "D3DClass.h"

//위젯의 외형 커스터마이징
//버튼, 체크박스, 스크롤바 등 기본 위젯의 모양을 직접 그릴 수 있음
#include <QStyle>

//디버깅 메시지 출력
#include <QDebug>

//현재 시간 가져오기
#include <QTime>

//Qt에서 디스플레이 관련 정보를 가져오거나 제어할 때 사용하는 헤더
#include <QScreen>

//Qt에서 팝업 메시지 창(대화상자) 띄울 때 사용하는 헤더
#include <QMessageBox>

//Qt에서 창이 닫힐 때 발생하는 이벤트를 처리하기 위해 사용하는 헤더
#include <QCloseEvent>

//Qt에서 화면(디스플레이)정보에 접근하기 위해 사용되던 헤더
#include <QDesktopWidget>

ViewerSample::ViewerSample(QWidget * parent)
	: QMainWindow(parent)
	, ui(new Ui::ViewerSampleClass)


	, m_WindowSize(QSize(1280, 800))

	//체크박스 위젯을 가리키는 포인터
	, m_pCbxDoFrames(new QCheckBox(this))
{
	//setupUi(this)는 .ui 파일에 정의된 모든 위젯을 this (ex: QMainWindow)에 붙이고 초기화 작업
	ui->setupUi(this);

	// System 객체 생성
	System = new SystemClass;
	
	if (!System)
	{
		return;
	}

	// System 객체 초기화 및 실행
	if (System->Initialize())
	{
		System->m_Graphics->m_Direct3D->qtD3dWidget = ui->view;
		System->Run();
	}

	// System 객체 종료 및 메모리 반환
	System->Shutdown();
	delete System;
	System = nullptr;

	/*m_pScene = ui->view;
	m_pScene = ui->view;*/

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
	/*connect(m_pScene, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);
	connect(m_pScene, &QDirect3D11Widget::ticked, this, &ViewerSample::tick);
	connect(m_pScene, &QDirect3D11Widget::rendered, this, &ViewerSample::render);*/

	connect(System->m_Graphics->m_Direct3D->qtD3dWidget, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);
	connect(System->m_Graphics->m_Direct3D->qtD3dWidget, &QDirect3D11Widget::ticked, this, &ViewerSample::tick);
	connect(System->m_Graphics->m_Direct3D->qtD3dWidget, &QDirect3D11Widget::rendered, this, &ViewerSample::render);

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


	//QTimer::singleShot(500, this, [&] { m_pScene->run(); });
	QTimer::singleShot(500, this, [&] { System->m_Graphics->m_Direct3D->qtD3dWidget->run(); });
	//disconnect(m_pScene, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);
	disconnect(System->m_Graphics->m_Direct3D->qtD3dWidget, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);
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