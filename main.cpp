#include <QApplication>
#include "AppWindow.h"

int main(int argc, char ** argv)
{
    QApplication app(argc,argv);
    AppWindow * window;
    
    if (argc >= 2)
        window = new AppWindow(argv[1]);
    else
        window = new AppWindow();

    window->show();
    return app.exec();
}

