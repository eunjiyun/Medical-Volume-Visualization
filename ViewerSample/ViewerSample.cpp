#include <QStyle>
#include <QDebug>
#include <QTime>
#include <QScreen>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopWidget>
#include "ViewerSample.h"
#include"FileReader.h"

ViewerSample::ViewerSample(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::ViewerSampleClass)
	, m_WindowSize(QSize(1280, 800))
{
	ui->setupUi(this);
	m_pScene = ui->view;
	adjustWindowSize();
	connectSlots();
}

ViewerSample::~ViewerSample() = default;

void ViewerSample::adjustWindowSize()
{
	resize(m_WindowSize.width(), m_WindowSize.height());
	setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(),
		qApp->screens().first()->availableGeometry()));
}



void ViewerSample::connectSlots()
{
	connect(m_pScene, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);
	connect(m_pScene, &QDirect3D11Widget::ticked, this, &ViewerSample::tick);
	connect(m_pScene, &QDirect3D11Widget::rendered, this, &ViewerSample::render);
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

	QString name = QString::fromLocal8Bit(m_pScene->fileReader->patientName.c_str());
	QString label = QString::fromLocal8Bit("  ") + name;
	ui->label_name->setText(label.toUtf8().constData());

	QString patientMF = QString::fromLocal8Bit(m_pScene->fileReader->patientMF.c_str());
	QString patientMFLabel = QString::fromLocal8Bit("    ") + patientMF;
	ui->label_gender->setText(patientMFLabel.toUtf8().constData());



	QString patientID = QString::fromLocal8Bit(m_pScene->fileReader->patientID.c_str());
	QString patientIDLabel = ui->label_id->text(); // 기존 텍스트

	QString richTextpatientID = "&nbsp;&nbsp;" + patientIDLabel +
		"&nbsp;&nbsp;&nbsp;" +
		"<b>" + patientID + "</b>";
	ui->label_id->setTextFormat(Qt::RichText);
	ui->label_id->setText(richTextpatientID);


	QString patientBirth = QString::fromLocal8Bit(m_pScene->fileReader->birthDate.c_str());
	QString labelText = ui->label_age->text(); // 기존 텍스트

	QString richText = "&nbsp;&nbsp;" + labelText +
		"&nbsp;&nbsp;&nbsp;" +
		"<b>" + patientBirth + "</b>";
	ui->label_age->setTextFormat(Qt::RichText);
	ui->label_age->setText(richText);



	QString studyDate = QString::fromLocal8Bit(m_pScene->fileReader->studyDate.c_str());
	QString studyDateLabel = ui->label_examDate->text(); // 기존 텍스트

	QString richTextstudyDate = "&nbsp;&nbsp;" + studyDateLabel +
		"&nbsp;&nbsp;&nbsp;" +
		"<b>" + studyDate + "</b>";
	ui->label_examDate->setTextFormat(Qt::RichText);
	ui->label_examDate->setText(richTextstudyDate);


	disconnect(m_pScene, &QDirect3D11Widget::deviceInitialized, this, &ViewerSample::init);
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
	m_pScene->release();
	QTime dieTime = QTime::currentTime().addMSecs(500);
	while (QTime::currentTime() < dieTime)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

	event->accept();
}
