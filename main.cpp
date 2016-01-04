#include <QApplication>
#include "AppWindow.h"

int main(int argc, char ** argv)
{
    QApplication app(argc,argv);
    AppWindow * window = new AppWindow();
    
    if (argc >= 2) window->open(argv[1]);
    window->show();
    
    return app.exec();
}

