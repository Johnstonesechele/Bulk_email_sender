#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include <QStandardPaths>
#include <QDir>
#include "mainwindow.h"
#include "appsettings.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Bulk Email Manager Professional");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("BulkEmailManager");
    app.setApplicationDisplayName("Bulk Email Manager - Professional Edition");
    
    // Initialize application directories
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    QDir().mkpath(appDataPath + "/logs");
    QDir().mkpath(appDataPath + "/templates");
    QDir().mkpath(appDataPath + "/backups");
    
    // Initialize logger
    Logger *logger = Logger::instance();
    logger->setLogLevel(LogLevel::Info);
    logger->info("Application", "Starting Bulk Email Manager Professional v2.0.0");
    
    // Initialize settings
    AppSettings *settings = AppSettings::instance();
    settings->load();
    
    // Apply theme from settings
    QString theme = settings->getTheme();
    if (theme == "System") {
        // Use system theme
        app.setStyle(QStyleFactory::create("windowsvista"));
    } else if (theme == "Light") {
        // Apply light theme
        app.setStyle(QStyleFactory::create("Fusion"));
        QPalette lightPalette;
        lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
        lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
        lightPalette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
        lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
        lightPalette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
        lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));
        lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
        lightPalette.setColor(QPalette::BrightText, QColor(255, 0, 0));
        lightPalette.setColor(QPalette::Link, QColor(0, 100, 200));
        lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
        lightPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        app.setPalette(lightPalette);
    } else {
        // Apply dark theme (default)
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
    }
    
    // Apply font from settings
    QFont appFont = settings->getApplicationFont();
    app.setFont(appFont);
    
    logger->info("Application", "Creating main window");
    MainWindow window;
    
    // Restore window settings
    if (settings->getWindowMaximized()) {
        window.showMaximized();
    } else {
        window.resize(settings->getWindowSize());
        window.move(settings->getWindowPosition());
        window.show();
    }
    
    logger->info("Application", "Application started successfully");
    
    int result = app.exec();
    
    logger->info("Application", "Application shutting down");
    return result;
}