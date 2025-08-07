#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Bulk Email Manager");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("EmailCorp");
    
    // Apply dark theme with dark blue and dark gold colors
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(25, 35, 45));
    darkPalette.setColor(QPalette::WindowText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Base, QColor(35, 45, 55));
    darkPalette.setColor(QPalette::AlternateBase, QColor(45, 55, 65));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(25, 35, 45));
    darkPalette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Text, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Button, QColor(45, 55, 65));
    darkPalette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::BrightText, QColor(255, 215, 0)); // Dark gold
    darkPalette.setColor(QPalette::Link, QColor(100, 150, 255));
    darkPalette.setColor(QPalette::Highlight, QColor(255, 215, 0)); // Dark gold
    darkPalette.setColor(QPalette::HighlightedText, QColor(25, 35, 45));
    
    app.setPalette(darkPalette);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}