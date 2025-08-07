#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Bulk Email Manager");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("EmailPro");
    
    // Apply custom dark theme
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(25, 35, 45));           // Dark blue background
    darkPalette.setColor(QPalette::WindowText, QColor(255, 255, 255));    // White text
    darkPalette.setColor(QPalette::Base, QColor(35, 45, 55));             // Input background
    darkPalette.setColor(QPalette::AlternateBase, QColor(45, 55, 65));    // Alternate rows
    darkPalette.setColor(QPalette::ToolTipBase, QColor(0, 0, 0));         // Tooltip background
    darkPalette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));   // Tooltip text
    darkPalette.setColor(QPalette::Text, QColor(255, 255, 255));          // Text color
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));           // Button background
    darkPalette.setColor(QPalette::ButtonText, QColor(255, 255, 255));    // Button text
    darkPalette.setColor(QPalette::BrightText, QColor(255, 0, 0));        // Bright text
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));           // Link color
    darkPalette.setColor(QPalette::Highlight, QColor(184, 134, 11));      // Dark gold highlight
    darkPalette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));     // Highlighted text
    
    app.setPalette(darkPalette);
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}