// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "conversion_thread.h"

ConversionThread::ConversionThread( QObject *parent ) : QThread( parent )
{}

void ConversionThread::setJob(
    rta::util::ImageConverter::Settings settings, QStringList paths )
{
    m_settings = std::move( settings );
    m_paths    = std::move( paths );
    m_cancel.store( false );
}

void ConversionThread::requestCancel()
{
    m_cancel.store( true );
}

void ConversionThread::run()
{
    const int total = static_cast<int>( m_paths.size() );
    for ( int i = 0; i < total; ++i )
    {
        if ( m_cancel.load() )
        {
            emit batchFinished();
            return;
        }

        const QString path = m_paths.at( i );
        emit          fileStarted( i, path );

        rta::util::ImageConverter converter;
        converter.settings = m_settings;
        const bool ok      = converter.process_image( path.toStdString() );
        emit       fileFinished(
            i, ok, QString::fromStdString( converter.last_error_message ) );
        emit progress( i + 1, total );
    }
    emit batchFinished();
}
