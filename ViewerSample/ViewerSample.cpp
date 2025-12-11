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

	connect(ui->btnViewHead, &QPushButton::clicked, this, &ViewerSample::volumeShowHide);


	// ✅ 시그널 연결

	connect(ui->huSlider, &QSlider::valueChanged, this, &ViewerSample::huValueChanged);
	connect(ui->contrastSlider, &QSlider::valueChanged, this, &ViewerSample::contrastWidthChanged);


	// 시그널 연결
	connect(ui->brightnessSlider, &QSlider::valueChanged,
		this, [this](int value) {
			//double brightness = value/1000.0 ;  // -500~500 → -0.5~0.5

			qDebug() << "Slider moved:" << value;
			double brightness = value;
			brightnessCenterChanged(brightness);
		});
	connect(ui->sharpnessSlider, &QSlider::valueChanged, this, &ViewerSample::sharpnessChanged);



}


// 예: MainWindow.cpp
void ViewerSample::onBtnColorInvertClicked() {
	//// 여기에 원하는 동작을 구현
	//qDebug() << "볼륨 전환 버튼이 클릭되었습니다!";
	//// 예: dx 값을 변경하거나 뷰 업데이트

	if (m_pScene->isPlaster)
		m_pScene->isPlaster = false;
	else
		m_pScene->isPlaster = true;

	update();
}

void ViewerSample::volumeShowHide()
{

	if (1.0f==m_pScene->cb.CameraPosAndAlpha.w)
		m_pScene->cb.CameraPosAndAlpha.w = 0.0f;
	else
		m_pScene->cb.CameraPosAndAlpha.w = 1.0f;

	//// ⭐ 2. GPU로 전송!
	//m_pScene->m_pDeviceContext->UpdateSubresource(
	//	m_pScene->cbRay.Get(), 0, nullptr,
	//	&m_pScene->cb, 0, 0
	//);



	std::cout << "clicked!!!!!!!!" << endl;

	update();
}


void ViewerSample::huValueChanged(int value)
{

	float huCenter = (float)value;
	ui->huValueLabel->setText(QString::number((int)huCenter));

	if (!m_pScene || !m_pScene->fileReader) return;

	// ⭐ Width를 늘림
	m_pScene->fileReader->volWC = huCenter;
	// ⭐ HU 값을 0~1로 정규화
	float t = (huCenter + 1000.0f) / 4000.0f;  // -1000~3000 → 0~1
	// ⭐ Window Width를 역으로 조정 (HU 높을수록 좁게)
	float windowWidth = 4000.0f - t * 3000.0f;  // 4000 → 1000

	m_pScene->fileReader->volWW = windowWidth;  // 1500 → 3000


	float sliderNorm = (huCenter - (-1000.0f)) / (3000.0f - (-1000.0f));
	// 결과: HU=-3600 → 0.0
	//       HU=-1000 → 1.0
	sliderNorm = std::clamp(sliderNorm, 0.0f, 1.0f);

	float minBoost = 3.0f;   // HU 최소 → soft tissue 3배 진하게
	float maxBoost = 0.4f;   // HU 최대 → soft tissue 40%만 남김

	//m_pScene->cb.alphaScale = minBoost * (1.0f - sliderNorm) + maxBoost * sliderNorm;

	update();
}

void ViewerSample::brightnessCenterChanged(double brightness)
{

	if (!m_pScene || !m_pScene->fileReader) return;
	if (-1 == m_initialWindowCenter) {
		m_initialWindowCenter = m_pScene->fileReader->windowCenter;
		brightness = 0;
		//return;
	}

	qDebug() << "brightness:" << brightness;
	qDebug() << "m_initialWindowWidth:" << m_initialWindowWidth;
	qDebug() << "m_initialWindowCenter:" << m_initialWindowCenter;


	// brightness: -0.5 ~ 0.5
// WC를 ±WW의 절반 범위로 조절 (±2000)
	float offset = brightness / 1000.0  * m_initialWindowWidth;  // -2000 ~ +2000
	float newWC = m_initialWindowCenter + offset;      // -1000 ~ 3000

	m_pScene->fileReader->windowCenter = newWC;

	ui->brightnessValueLabel->setText(QString::number(newWC));



	for (int i{ 1 }; i <= 3; ++i) {

		ID3D11RenderTargetView* rtvA, *rtvC, *rtvS;
		ID3D11ShaderResourceView* srvA, *srvC, *srvS;
		ID3D11Texture2D* texA, *texC, *texS;


		//if (clickedViewIndex != i) {

		switch (i) {
		case 1:
			m_pScene->fileReader->UpdateAxialTexture(m_pScene->fileReader->currentIndex[1]);
			texA = m_pScene->fileReader->axialTextureCache[m_pScene->fileReader->currentIndex[1]];
			srvA = m_pScene->getSRVForTexture(texA);
			m_pScene->m_SRViews.slices[1] = srvA;

			rtvA = m_pScene->getRTVForTexture(texA);
			m_pScene->m_RTViews.slices[1] = rtvA;


			m_pScene->sliceInfoAxial->hide();
			m_pScene->sliceInfoAxial->setText(QString("Image %1/%2").arg(m_pScene->fileReader->m_depth - m_pScene->fileReader->currentIndex[1] + 1).arg(m_pScene->fileReader->m_depth));

			m_pScene->sliceInfoAxial->show();

			break;
		case 2:
			m_pScene->fileReader->UpdateCoronalTexture(m_pScene->fileReader->currentIndex[2]);
			texC = m_pScene->fileReader->coronalTextureCache[m_pScene->fileReader->currentIndex[2]];
			srvC = m_pScene->getSRVForTexture(texC);
			m_pScene->m_SRViews.slices[2] = srvC;

			rtvC = m_pScene->getRTVForTexture(texC);
			m_pScene->m_RTViews.slices[2] = rtvC;


			m_pScene->sliceInfoCoronal->hide();
			m_pScene->sliceInfoCoronal->setText(QString("Image %1/%2").arg(m_pScene->fileReader->currentIndex[2] + 1).arg(m_pScene->fileReader->m_height));

			m_pScene->sliceInfoCoronal->show();


			break;
		case 3:
			m_pScene->fileReader->UpdateSagittalTexture(m_pScene->fileReader->currentIndex[3]);
			texS = m_pScene->fileReader->sagittalTextureCache[m_pScene->fileReader->currentIndex[3]];
			srvS = m_pScene->getSRVForTexture(texS);
			m_pScene->m_SRViews.slices[3] = srvS;

			rtvS = m_pScene->getRTVForTexture(texS);
			m_pScene->m_RTViews.slices[3] = rtvS;

			m_pScene->sliceInfoSagittal->hide();
			m_pScene->sliceInfoSagittal->setText(QString("Image %1/%2").arg(m_pScene->fileReader->currentIndex[3] + 1).arg(m_pScene->fileReader->m_width));

			m_pScene->sliceInfoSagittal->show();

			break;
		}

	}

	m_pScene->UpdateSlicePlanePositions();


	// 렌더링 업데이트
	update();
}
void ViewerSample::contrastWidthChanged(double contrast)
{// Window Width


	if (!m_pScene || !m_pScene->fileReader) return;
	if (-1 == m_initialWindowWidth) {
		m_initialWindowWidth = m_pScene->fileReader->windowWidth;
		contrast = 1000;
		//return;
	}


	// contrast: 0.0 ~ 2.0, 초기값 1.0
   // WW를 배율로 조절
	float newWW = m_initialWindowWidth * (contrast / 1000.0);  // 0 ~ 8000

	m_pScene->fileReader->windowWidth = newWW;

	ui->contrastValueLabel->setText(QString::number((double)newWW));



	for (int i{ 1 }; i <= 3; ++i) {

		ID3D11RenderTargetView* rtvA, *rtvC, *rtvS;
		ID3D11ShaderResourceView* srvA, *srvC, *srvS;
		ID3D11Texture2D* texA, *texC, *texS;

		switch (i) {
		case 1:
			m_pScene->fileReader->UpdateAxialTexture(m_pScene->fileReader->currentIndex[1]);
			texA = m_pScene->fileReader->axialTextureCache[m_pScene->fileReader->currentIndex[1]];
			srvA = m_pScene->getSRVForTexture(texA);
			m_pScene->m_SRViews.slices[1] = srvA;

			rtvA = m_pScene->getRTVForTexture(texA);
			m_pScene->m_RTViews.slices[1] = rtvA;


			m_pScene->sliceInfoAxial->hide();
			m_pScene->sliceInfoAxial->setText(QString("Image %1/%2").arg(m_pScene->fileReader->m_depth - m_pScene->fileReader->currentIndex[1] + 1).arg(m_pScene->fileReader->m_depth));

			m_pScene->sliceInfoAxial->show();

			break;
		case 2:
			m_pScene->fileReader->UpdateCoronalTexture(m_pScene->fileReader->currentIndex[2]);
			texC = m_pScene->fileReader->coronalTextureCache[m_pScene->fileReader->currentIndex[2]];
			srvC = m_pScene->getSRVForTexture(texC);
			m_pScene->m_SRViews.slices[2] = srvC;

			rtvC = m_pScene->getRTVForTexture(texC);
			m_pScene->m_RTViews.slices[2] = rtvC;


			m_pScene->sliceInfoCoronal->hide();
			m_pScene->sliceInfoCoronal->setText(QString("Image %1/%2").arg(m_pScene->fileReader->currentIndex[2] + 1).arg(m_pScene->fileReader->m_height));

			m_pScene->sliceInfoCoronal->show();

			break;
		case 3:
			m_pScene->fileReader->UpdateSagittalTexture(m_pScene->fileReader->currentIndex[3]);
			texS = m_pScene->fileReader->sagittalTextureCache[m_pScene->fileReader->currentIndex[3]];
			srvS = m_pScene->getSRVForTexture(texS);
			m_pScene->m_SRViews.slices[3] = srvS;

			rtvS = m_pScene->getRTVForTexture(texS);
			m_pScene->m_RTViews.slices[3] = rtvS;

			m_pScene->sliceInfoSagittal->hide();
			m_pScene->sliceInfoSagittal->setText(QString("Image %1/%2").arg(m_pScene->fileReader->currentIndex[3] + 1).arg(m_pScene->fileReader->m_width));

			m_pScene->sliceInfoSagittal->show();

			break;
		}

	}

	m_pScene->UpdateSlicePlanePositions();


	// 렌더링 업데이트
	update();
}
void ViewerSample::sharpnessChanged(int value)
{
	float sharpness = value / 100.0f;  // 0~200 → 0.0~2.0

	ui->sharpnessValueLabel->setText(QString::number(sharpness, 'f', 2));


	qDebug() << "Sharpness value:" << sharpness;  // ⭐ 이게 출력되는지 확인

	if (!m_pScene) return;

	m_pScene->SetSharpness(sharpness);  // ⭐ 하나만 호출
	m_pScene->update();
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


	// ViewerSample 초기화
	ui->huSlider->setMinimum(-500);
	ui->huSlider->setMaximum(3000);
	ui->huSlider->setValue(1000);  // 뼈 중심


	//brightness
	ui->brightnessSlider->setMinimum(-500);
	ui->brightnessSlider->setMaximum(500);
	ui->brightnessSlider->setValue(0);
	ui->brightnessSlider->setInvertedAppearance(true);  // ⭐ UI 방향 반대로
	ui->brightnessSlider->setInvertedControls(true);


	//contrast
	ui->contrastSlider->setMinimum(1);
	ui->contrastSlider->setMaximum(2000);
	ui->contrastSlider->setValue(1000);
	ui->contrastSlider->setInvertedAppearance(true);  // ⭐ UI 방향 반대로
	ui->contrastSlider->setInvertedControls(true);


	//sharpness
	// 슬라이더를 50 정도로 설정해서 테스트
	ui->sharpnessSlider->setMinimum(0);      // 0.0
	ui->sharpnessSlider->setMaximum(300);    // 3.0 (더 넓은 범위)
	ui->sharpnessSlider->setValue(0);

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
