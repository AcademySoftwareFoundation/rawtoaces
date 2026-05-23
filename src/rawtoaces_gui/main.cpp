// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "main_window.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIcon>

#include <cstdlib>

int main( int argc, char *argv[] )
{
#if defined( _WIN32 )
    _putenv( const_cast<char *>( "TZ=UTC" ) );
#else
    setenv( "TZ", "UTC", 1 );
#endif

    QApplication application( argc, argv );
    QApplication::setApplicationName( QStringLiteral( "rawtoaces" ) );
    // macOS menu bar uses the display name (not the .app / executable basename).
    QApplication::setApplicationDisplayName( QStringLiteral( "Raw to ACES" ) );
    QApplication::setOrganizationName( QStringLiteral( "rawtoaces" ) );

#if defined( Q_OS_MACOS )
    // Qt paints the Dock tile from this pixmap (no system squircle); the .icns carries
    // transparent corners (squircle)
    const QString icnsPath = QDir{ QCoreApplication::applicationDirPath() }.filePath(
        QStringLiteral( "../Resources/rawtoaces_gui.icns" ) );
    if ( QFile::exists( icnsPath ) )
    {
        QApplication::setWindowIcon( QIcon( icnsPath ) );
    }
#endif

    MainWindow mainWindow;
    mainWindow.show();
    return application.exec();
}
