#include <QApplication>
#include "MainWindow.h"

static QPalette createDarkPalette()
{
    QPalette palette;

    QColor darkBlue(24, 26, 32); // almost black blue
    QColor darkGold(210, 161, 0);
    QColor lighterBlue(40, 44, 52);

    palette.setColor(QPalette::Window, darkBlue);
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, lighterBlue);
    palette.setColor(QPalette::AlternateBase, darkBlue);
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, darkBlue);
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, darkGold);
    palette.setColor(QPalette::Highlight, darkGold);
    palette.setColor(QPalette::HighlightedText, Qt::black);

    return palette;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Bulk Email Sender");

    app.setPalette(createDarkPalette());

    MainWindow w;
    w.show();

    return app.exec();
}