// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "main_window.h"

#include <QApplication>

#ifndef WIN32
#    include <cstdlib>
#else
#    include <cstdlib>
#endif

int main( int argc, char *argv[] )
{
#ifndef WIN32
    setenv( "TZ", "UTC", 1 );
#else
    _putenv( const_cast<char *>( "TZ=UTC" ) );
#endif

    QApplication application( argc, argv );
    QApplication::setApplicationName( QStringLiteral( "rawtoaces" ) );
    QApplication::setOrganizationName( QStringLiteral( "rawtoaces" ) );

    MainWindow mainWindow;
    mainWindow.show();
    return application.exec();
}
