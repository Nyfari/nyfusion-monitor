#include <QtWidgets/QApplication>

#include "qt/MainWindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    ny::ui::qt::MainWindow window;
    window.show();

    return app.exec();
}
