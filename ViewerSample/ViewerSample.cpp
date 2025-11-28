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
	connect(ui->contrastSlider, &QSlider::valueChanged, this, &ViewerSample::contrastWidthChanged);

	//connect(ui->contrastSpinBox, &QDoubleSpinBox::valueChanged,
	//	this, [this](double value) {
	//		contrastWidthChanged(value);
	//	});

	//connect(ui->contrastSpinBox,
	//	static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
	//	this, [this](double value) {
	//		contrastWidthChanged(value);
	//	});


	//connect(ui->brightnessSlider, &QSlider::valueChanged, this, &ViewerSample::brightnessCenterChanged);

	//connect(ui->brightnessSlider,
	//	static_cast<void(QSlider::*)(double)>(&QDoubleSpinBox::valueChanged),
	//	this, [this](double value) {
	//		brightnessCenterChanged(value);
	//	});

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
	// 여기에 원하는 동작을 구현
	qDebug() << "볼륨 전환 버튼이 클릭되었습니다!";
	// 예: dx 값을 변경하거나 뷰 업데이트

	if (m_pScene->isPlaster)
		m_pScene->isPlaster = false;
	else
		m_pScene->isPlaster = true;

	update();
}

//void ViewerSample::huValueChanged(int value)
//{
////	// value: 0 ~ 1000 범위
////	// HU 중심값 계산: -1024 ~ 2927
////	float huCenter = -1024.0f + (value * 4.024f);
////	ui->huValueLabel->setText(QString::number((int)huCenter));
////
////	if (!m_pScene || !m_pScene->fileReader || !m_pScene->GetTransferFunction()) {
////		return;
////	}
////
////	// ⭐ WC 적용
////	m_pScene->fileReader->volWC = huCenter;
////
////	// ⭐ WW도 HU 값에 따라 자동 조절
////	float normalizedValue = value / 1000.0f;  // 0.0 ~ 1.0
////	float minWW = 400.0f;   // 연조직용 최소 폭
////	float maxWW = 2000.0f;  // 뼈용 최대 폭
////	float windowWidth = minWW + (normalizedValue * (maxWW - minWW));
////
////	m_pScene->fileReader->volWW = windowWidth;
////
////	qDebug() << "HU adjusted - WC:" << huCenter << "WW:" << windowWidth;
////
////	// ⭐ Transfer Function을 새로운 WC/WW로 재초기화
////m_pScene->GetTransferFunction()->Initialize(huCenter, windowWidth, m_pScene->m_pDevice);
////
////	// 또는 SetHUWindow 사용 (고정 TF 유지하려면)
////	// m_pScene->GetTransferFunction()->SetHUWindow(huCenter, windowWidth, m_pScene->m_pDevice);
////
////	update();
//
//	// ⭐ 구현 추가!
//	float huCenter = -1024.0f + (value * 4.024f);//2927
//
//	ui->huValueLabel->setText(QString::number((int)huCenter));
//
//	// ⭐ 3. Null 체크
//	if (!m_pScene || !m_pScene->fileReader || !m_pScene->GetTransferFunction()) {
//		return;
//	}
//
//	//if (m_pScene->fileReader && m_pScene->GetTransferFunction()) {
//	//	m_pScene->GetTransferFunction()->SetHUWindow(
//	//		huCenter, m_pScene->fileReader->windowWidth, m_pScene->m_pDevice
//	//	);
//	//}
//
//
//	// ⭐ 3. FileReader에 저장 (다음 렌더링 때 반영됨)
//	if (m_pScene && m_pScene->fileReader) {
//		m_pScene->fileReader->volWC = huCenter;
//		// windowWidth는 고정 또는 다른 슬라이더로 조절
//		// m_pScene->fileReader->windowWidth = 2000.0f;
//	}
//
//
//	//cb.HuParams.x = fileReader->m_rescaleSlope;
//	//cb.HuParams.y = fileReader->m_rescaleIntercept;
//	//cb.HuParams.z = fileReader->windowCenter - fileReader->windowWidth / 2.0;
//	//cb.HuParams.w = fileReader->windowCenter + fileReader->windowWidth / 2.0;
//
//
//	//// ⭐ Constant Buffer에 center/width 전달
//	//VolumeParams params;
//	//params.HuParams.z = huCenter;
//	//params.HuParams.w = 2000.0f;  // width
//
//	//m_pScene->m_pImmediateContext->UpdateSubresource(
//	//	m_constantBuffer, 0, nullptr, &params, 0, 0
//	//);
//
//
//
//
//	//m_pScene->cb.HuParams.z = huCenter - fileReader->windowWidth / 2.0;
//	//m_pScene->cb.HuParams.w = huCenter + fileReader->windowWidth / 2.0;
//
//
//	//m_pScene->m_pDeviceContext->UpdateSubresource(
//	//	m_constantBuffer, 0, nullptr, &params, 0, 0
//	//);
//
//	update();
//}


void ViewerSample::huValueChanged(int value)
{
	// ✅ 올바른 변환: 0~4000 → -1000~3000 HU
	//float huCenter = -1000.0f + (value * 1.0f);

	float huCenter = (float)value;
	ui->huValueLabel->setText(QString::number((int)huCenter));

	if (!m_pScene || !m_pScene->fileReader) return;

	float windowWidth = 2000.0f;  // 고정 또는 별도 슬라이더

	// TF 재초기화
	m_pScene->GetTransferFunction()->Initialize(
		huCenter, windowWidth, m_pScene->m_pDevice
	);

	m_pScene->fileReader->volWC = huCenter;

	//// CB 업데이트
	//float minHU = huCenter - windowWidth / 2.0f;
	//float maxHU = huCenter + windowWidth / 2.0f;

	//m_pScene->cb.HuParams.x = m_pScene->fileReader->m_rescaleSlope;
	//m_pScene->cb.HuParams.y = m_pScene->fileReader->m_rescaleIntercept;
	//m_pScene->cb.HuParams.z = minHU;
	//m_pScene->cb.HuParams.w = maxHU;

	//qDebug() << "HU Center:" << huCenter
	//	<< "Range:" << minHU << "~" << maxHU;

	update();
}
void ViewerSample::brightnessCenterChanged(double brightness)
{// Window Center


	if (!m_pScene || !m_pScene->fileReader) return;
	if (-1 == m_initialWindowCenter) {
		m_initialWindowCenter = m_pScene->fileReader->windowCenter;
		brightness = 0;
		//return;
	}

	qDebug() << "brightness:" << brightness;
	qDebug() << "m_initialWindowWidth:" << m_initialWindowWidth;
	qDebug() << "m_initialWindowCenter:" << m_initialWindowCenter;


	//float offset = brightness * m_initialWindowWidth * 0.5f; // WW의 절반 범위로 조절
	//float newWindowCenter = m_initialWindowCenter + offset;

	//// 실제 적용
	//m_pScene->fileReader->windowCenter = newWindowCenter;
	//ui->brightnessValueLabel->setText(QString::number(newWindowCenter));

	//////m_pScene->UpdateVolumeMatrix();
	//////m_pScene->UpdateSlicePlanePositions();  // ✅ 추가
	//////m_pScene->FullScreenPassSet();

	////m_pScene->update();
	//////update();





		// brightness: -0.5 ~ 0.5
	// WC를 ±WW의 절반 범위로 조절 (±2000)
	float offset = brightness/1000.0  * m_initialWindowWidth;  // -2000 ~ +2000
	float newWC = m_initialWindowCenter + offset;      // -1000 ~ 3000

	m_pScene->fileReader->windowCenter = newWC;

	ui->brightnessValueLabel->setText(QString::number(newWC));



	for (int i{ 1 }; i <= 3; ++i) {
		//	m_pScene->fileReader->views.centerPatientCoord[i] = m_pScene->patientCoord;
		//	m_pScene->fileReader->currentIndex[i] = m_pScene->ComputeSliceIndexForView(patientCoord, i);


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
			//sliceInfoAxial->adjustSize();
		  //  sliceInfoAxial->repaint();  // 강제로 다시 그리기
			m_pScene->sliceInfoAxial->show();


			//onAxialScroll(fileReader->currentIndex[1]);
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
			//sliceInfoCoronal->adjustSize();
		  //  sliceInfoCoronal->repaint();  // 강제로 다시 그리기
			m_pScene->sliceInfoCoronal->show();



			//onCoronalScroll(fileReader->currentIndex[2]);
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
			//sliceInfoSagittal->adjustSize();
			//sliceInfoSagittal->repaint();  // 강제로 다시 그리기
			m_pScene->sliceInfoSagittal->show();

			//onSagittalScroll(fileReader->currentIndex[3]);
			break;
		}
		//}

	}

	m_pScene->UpdateSlicePlanePositions();

	//RenderVolumeView(); // 강제 호출로 확인


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




	//// contrast: 0.0 ~ 2.0, 초기값 1.0
	//// 초기 WW에 비율 곱하기
	//float newWindowWidth = m_initialWindowWidth * (contrast/1000.0);

	//m_pScene->fileReader->windowWidth = newWindowWidth/2;

	//ui->contrastValueLabel->setText(QString::number((double)newWindowWidth/4000.0));



	//qDebug() << "=== Contrast Changed ===";
	//qDebug() << "contrast (slider value):" << contrast;  // 0.0 ~ 2.0
	//qDebug() << "m_initialWindowWidth:" << m_initialWindowWidth;
	//qDebug() << "newWindowWidth:" << newWindowWidth;


	//////m_pScene->UpdateVolumeMatrix();
	//////m_pScene->UpdateSlicePlanePositions();  // ✅ 추가
	//////m_pScene->FullScreenPassSet();

	////m_pScene->update();
	////ui->centralWidget->updateGeometry();

	//////update();



	 // contrast: 0.0 ~ 2.0, 초기값 1.0
	// WW를 배율로 조절
	float newWW = m_initialWindowWidth * (contrast/1000.0);  // 0 ~ 8000

	m_pScene->fileReader->windowWidth = newWW;

	ui->contrastValueLabel->setText(QString::number((double)newWW ));



	for (int i{ 1 }; i <= 3; ++i) {
	//	m_pScene->fileReader->views.centerPatientCoord[i] = m_pScene->patientCoord;
	//	m_pScene->fileReader->currentIndex[i] = m_pScene->ComputeSliceIndexForView(patientCoord, i);


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
				//sliceInfoAxial->adjustSize();
			  //  sliceInfoAxial->repaint();  // 강제로 다시 그리기
				m_pScene->sliceInfoAxial->show();


				//onAxialScroll(fileReader->currentIndex[1]);
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
				//sliceInfoCoronal->adjustSize();
			  //  sliceInfoCoronal->repaint();  // 강제로 다시 그리기
				m_pScene->sliceInfoCoronal->show();



				//onCoronalScroll(fileReader->currentIndex[2]);
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
				//sliceInfoSagittal->adjustSize();
				//sliceInfoSagittal->repaint();  // 강제로 다시 그리기
				m_pScene->sliceInfoSagittal->show();

				//onSagittalScroll(fileReader->currentIndex[3]);
				break;
			}
		//}

	}

	m_pScene->UpdateSlicePlanePositions();

	//RenderVolumeView(); // 강제 호출로 확인


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

	//loadDicomData();

	ui->huSlider->setMinimum(0);
	ui->huSlider->setMaximum(4000);
	ui->huSlider->setValue(2000);//2114
	// HU 슬라이더 초기 설정
	//ui->huSlider->setInvertedAppearance(true);  // ⭐ UI 방향 반대로
	//ui->huSlider->setInvertedControls(true);

//	m_pScene->GetTransferFunction()->SetHUWindow(0, 0, m_pScene->m_pDevice);
	//ui->huSlider->setMinimum(-1024);  // 최소 HU (공기)
	//ui->huSlider->setMaximum(3000);   // 최대 HU (치아/금속)

	//ui->huSlider->setMinimum(-1024);  // 최소 HU (공기)
	//ui->huSlider->setMaximum(6000);   // 최대 HU (치아/금속)
	//ui->huSlider->setValue(2000);     // 초기값: 디폴트 WC


	//brightness
	ui->brightnessSlider->setMinimum(-500);
	ui->brightnessSlider->setMaximum(500);
	ui->brightnessSlider->setValue(0);
	//ui->brightnessSlider->setSingleStep(0.01);
	ui->brightnessSlider->setInvertedAppearance(true);  // ⭐ UI 방향 반대로
	ui->brightnessSlider->setInvertedControls(true);


	//contrast
	ui->contrastSlider->setMinimum(1);
	ui->contrastSlider->setMaximum(2000);
	ui->contrastSlider->setValue(1000);
	//ui->contrastSlider->setSingleStep(0.02);
	ui->contrastSlider->setInvertedAppearance(true);  // ⭐ UI 방향 반대로
	ui->contrastSlider->setInvertedControls(true);


	//sharpness
	//ui->sharpnessSlider->setMinimum(0);
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
