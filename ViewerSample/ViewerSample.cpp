#include "ViewerSample.h"

#include <QStyle>

#include <QDebug>

#include <QTime>

#include <QScreen>

#include <QMessageBox>

#include <QCloseEvent>

#include <QDesktopWidget>

ViewerSample::ViewerSample(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::ViewerSampleClass)


    , m_WindowSize(QSize(1280, 800))

    , m_pCbxDoFrames(new QCheckBox(this))
{

    ui->setupUi(this);


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

    m_pCbxDoFrames->setText("Do Frames");
    m_pCbxDoFrames->setChecked(true);
    connect(m_pCbxDoFrames, &QCheckBox::stateChanged, [&] {
        if (m_pCbxDoFrames->isChecked())
            m_pScene->continueFrames();
        else
            m_pScene->pauseFrames();
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
