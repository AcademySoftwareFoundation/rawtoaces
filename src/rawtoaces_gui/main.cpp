// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "main_window.h"

#include <QApplication>

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
    QApplication::setOrganizationName( QStringLiteral( "rawtoaces" ) );

    MainWindow mainWindow;
    mainWindow.show();
    return application.exec();
}
