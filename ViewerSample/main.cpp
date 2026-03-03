//Qt 애플리케이션의 시작점
#include <QtWidgets/QApplication>
#include "ViewerSample.h"

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

	return a.exec();
}
