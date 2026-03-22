// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <atomic>

#include <QThread>
#include <QStringList>

#include <rawtoaces/image_converter.h>

class ConversionThread final : public QThread
{
    Q_OBJECT

public:
    explicit ConversionThread( QObject *parent = nullptr );

    void
    setJob( rta::util::ImageConverter::Settings settings, QStringList paths );

    void requestCancel();

signals:
    void fileStarted( int index, QString path );
    void fileFinished( int index, bool ok, QString message );
    void progress( int done, int total );
    void batchFinished();

protected:
    void run() override;

private:
    rta::util::ImageConverter::Settings m_settings{};
    QStringList                         m_paths;
    std::atomic_bool                    m_cancel{ false };
};
