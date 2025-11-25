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

	connect(ui->btnColorInvert, &QPushButton::clicked, this, &ViewerSample::onBtnColorInvertClicked);
	// ✅ 시그널 연결

	connect(ui->huSlider, &QSlider::valueChanged, this, &ViewerSample::huValueChanged);

}


// 예: MainWindow.cpp
void ViewerSample::onBtnColorInvertClicked() {
	// 여기에 원하는 동작을 구현
	qDebug() << "볼륨 전환 버튼이 클릭되었습니다!";
	// 예: dx 값을 변경하거나 뷰 업데이트

	if (m_pScene->isPlaster)
		m_pScene->isPlaster = false;
	else
		m_pScene->isPlaster = true;

	update();
}

void ViewerSample::huValueChanged(int value)
{
	// ⭐ 구현 추가!
	float huCenter = -1024.0f + (value * 4.024f);//2927

	ui->labelValue1->setText(QString::number((int)huCenter));

	// ⭐ 3. Null 체크
	if (!m_pScene || !m_pScene->fileReader || !m_pScene->GetTransferFunction()) {
		return;
	}

	//if (m_pScene->fileReader && m_pScene->GetTransferFunction()) {
	//	m_pScene->GetTransferFunction()->SetHUWindow(
	//		huCenter, m_pScene->fileReader->windowWidth, m_pScene->m_pDevice
	//	);
	//}


	// ⭐ 3. FileReader에 저장 (다음 렌더링 때 반영됨)
	if (m_pScene && m_pScene->fileReader) {
		m_pScene->fileReader->windowCenter = huCenter;
		// windowWidth는 고정 또는 다른 슬라이더로 조절
		// m_pScene->fileReader->windowWidth = 2000.0f;
	}


	//cb.HuParams.x = fileReader->m_rescaleSlope;
	//cb.HuParams.y = fileReader->m_rescaleIntercept;
	//cb.HuParams.z = fileReader->windowCenter - fileReader->windowWidth / 2.0;
	//cb.HuParams.w = fileReader->windowCenter + fileReader->windowWidth / 2.0;



	//// ⭐ Constant Buffer에 center/width 전달
	//VolumeParams params;
	//params.HuParams.z = huCenter;
	//params.HuParams.w = 2000.0f;  // width

	//m_pScene->m_pImmediateContext->UpdateSubresource(
	//	m_constantBuffer, 0, nullptr, &params, 0, 0
	//);




	//m_pScene->cb.HuParams.z = huCenter - fileReader->windowWidth / 2.0;
	//m_pScene->cb.HuParams.w = huCenter + fileReader->windowWidth / 2.0;


	//m_pScene->m_pDeviceContext->UpdateSubresource(
	//	m_constantBuffer, 0, nullptr, &params, 0, 0
	//);

	update();
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



	ui->huSlider->setMinimum(114);
	ui->huSlider->setMaximum(3200);
	ui->huSlider->setValue(1751);


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
