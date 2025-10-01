#include "ViewerSample.h"

//Qt 애플리케이션의 시작점
#include <QtWidgets/QApplication>

//기능이 많은 기본 창 클래스
#include <QtWidgets/QMainWindow>

//모든 Qt위젯의 기본 클래스
#include <QtWidgets/QWidget>

//위젯을 격자 형태로 배치하는 레이아웃 클래스
#include <QtWidgets/QGridLayout>

//텍스트, 이미지, 또는 HTML을 표시할 수 있는 정적인 표시용 위젯
#include <QtWidgets/QLabel>


class FourViewWindow :public QMainWindow {
public:
	FourViewWindow(QWidget* parent = nullptr) :QMainWindow(parent) {
		QWidget* central = new QWidget(this);

		//QGridLayout은 Qt에서 위젯을 격자 형태로 배치하는 레이아웃 시스템
		//central이라는 부모 위젯에 붙이는 작업
		QGridLayout* layout = new QGridLayout(central);


		//4개의 분할 영역 생성
		for (int i{}; i < 4; ++i) {
			QWidget* view = new QWidget();
			view->setStyleSheet("background-color: lightgray; border: 1px solid black;");
			QStringList viewNames = { "Volume", "Axial", "Coronal", "Sagittal" };

			//label은 텍스트나 이미지를 표시하는 위젯
			//view는 QWidget 객체 => 직접 만든 회색 배경의 컨테이너
			//view를 label의 부모로 지정
			QLabel* label = new QLabel(view);
			label->setText(viewNames[i]);

			//label 위치 정렬
			label->setAlignment(Qt::AlignCenter);

			//Grid 위치 계산
			//세로 : row
			int row = i / 2;

			//가로 : col
			int col = i % 2;
			layout->addWidget(view, row, col);
		}

		setCentralWidget(central);
		setWindowTitle("Viewer Sample");
		resize(800, 600);
	}
};


//Qt의 이벤트 루프를 시작하는 함수
//마우스 클릭, 키 입력, 윈도우 리사이즈 같은 이벤트를 처리하면서 앱이 계속 실행됨
//a.exec()이 끝나면 앱이 종료됨=>main() 함수가 반환됨
//QApplication 생성 => FourViewWindow 생성 => show() 호출->창 띄움
//=> a.exec() -> 이벤트 루프 시작 => 사용자 이벤트 처리 => 창 닫힘->exec() 종료 
//=> return->프로그램 종료
int main(int argc, char *argv[])
{
	//qt gui 애플리케이션을 초기화하는 객체
	//이벤트 루프 관리 및 모든 위젯의 생명주기를 통제함
    QApplication a(argc, argv);

    ViewerSample w;
    w.show();

	////QMainWindow를 상송한 클래스
	//FourViewWindow w;
	//w.show();

    return a.exec();
}
