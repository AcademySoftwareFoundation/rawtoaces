#pragma once

#include <QThread>
#include <QProcess>
#include <QStringList>
#include <QMutex>
#include <QTimer>
#include "ParameterWidget.h"

class ConversionWorker : public QThread
{
    Q_OBJECT

public:
    explicit ConversionWorker(const QStringList &files, 
                             const ConversionParameters &parameters,
                             QObject *parent = nullptr);
    
    void stop();

signals:
    void progress(int current, int total, const QString &filename);
    void fileCompleted(const QString &filename, bool success, const QString &message);
    void finished(bool success, const QString &message);
    void logMessage(const QString &message);

protected:
    void run() override;

private:
    QStringList buildArguments(const QString &inputFile, const QString &outputFile) const;
    QString resolveRawtoacesProgram() const;
    QString getOutputFilename(const QString &inputFile) const;
    bool executeProcess(const QString &program, const QStringList &args, const QString &filename);
    
    QStringList m_files;
    ConversionParameters m_parameters;
    QMutex m_stopMutex;
    bool m_stopRequested;
    int m_currentFile;
    int m_totalFiles;
};
