#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTimer>
#include "MainWindow.h"
#include "SettingsManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("RAWtoACES GUI");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Academy Software Foundation");
    app.setOrganizationDomain("aswf.io");
    
    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Apply dark theme
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    // Initialize settings
    SettingsManager::instance().loadSettings();
    
    // Create and show main window
    MainWindow window;
    window.show();
    // Optional smoke-test: if env RAWTOACES_GUI_SMOKE_TEST is set, quit shortly after launch
    if (qEnvironmentVariableIsSet("RAWTOACES_GUI_SMOKE_TEST")) {
        QTimer::singleShot(1200, &app, &QCoreApplication::quit);
    }
    
    // Handle command line arguments
    QStringList args = app.arguments();
    if (args.size() > 1) {
        QStringList files;
        for (int i = 1; i < args.size(); ++i) {
            if (QFile::exists(args[i])) {
                files << args[i];
            }
        }
        if (!files.isEmpty()) {
            window.addFiles(files);
        }
    }
    
    return app.exec();
}
