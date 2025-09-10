#include "ConversionWorker.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>

ConversionWorker::ConversionWorker(const QStringList &files, 
                                 const ConversionParameters &parameters,
                                 QObject *parent)
    : QThread(parent)
    , m_files(files)
    , m_parameters(parameters)
    , m_stopRequested(false)
    , m_currentFile(0)
    , m_totalFiles(files.size())
{
}

void ConversionWorker::stop()
{
    QMutexLocker locker(&m_stopMutex);
    m_stopRequested = true;
}

void ConversionWorker::run()
{
    emit logMessage("Starting RAWtoACES conversion...");
    
    int successCount = 0;
    int failureCount = 0;
    QStringList failedFiles;
    
    for (int i = 0; i < m_files.size(); ++i) {
        // Check if stop was requested
        {
            QMutexLocker locker(&m_stopMutex);
            if (m_stopRequested) {
                emit logMessage("Conversion stopped by user");
                emit finished(false, "Conversion stopped by user");
                return;
            }
        }
        
        const QString &inputFile = m_files[i];
        m_currentFile = i + 1;
        
        emit progress(m_currentFile, m_totalFiles, inputFile);
        emit logMessage(QString("Processing file %1/%2: %3")
                       .arg(m_currentFile)
                       .arg(m_totalFiles)
                       .arg(QFileInfo(inputFile).fileName()));
        
        // Check if input file exists
        if (!QFile::exists(inputFile)) {
            QString error = QString("Input file does not exist: %1").arg(inputFile);
            emit logMessage(error);
            emit fileCompleted(inputFile, false, error);
            failedFiles << inputFile;
            failureCount++;
            continue;
        }
        
        // Generate output filename
        QString outputFile = getOutputFilename(inputFile);
        
        // Check if output directory exists and create if needed
        QFileInfo outputInfo(outputFile);
        QDir outputDir = outputInfo.absoluteDir();
        if (!outputDir.exists()) {
            if (m_parameters.createDirs) {
                if (!outputDir.mkpath(".")) {
                    QString error = QString("Failed to create output directory: %1")
                                   .arg(outputDir.absolutePath());
                    emit logMessage(error);
                    emit fileCompleted(inputFile, false, error);
                    failedFiles << inputFile;
                    failureCount++;
                    continue;
                }
            } else {
                QString error = QString("Output directory does not exist: %1")
                               .arg(outputDir.absolutePath());
                emit logMessage(error);
                emit fileCompleted(inputFile, false, error);
                failedFiles << inputFile;
                failureCount++;
                continue;
            }
        }
        
        // Check if output file exists and handle overwrite
        if (QFile::exists(outputFile) && !m_parameters.overwrite) {
            QString error = QString("Output file exists and overwrite is disabled: %1")
                           .arg(outputFile);
            emit logMessage(error);
            emit fileCompleted(inputFile, false, error);
            failedFiles << inputFile;
            failureCount++;
            continue;
        }
        
    // Build and execute process safely
    QString program = resolveRawtoacesProgram();
    QStringList args = buildArguments(inputFile, outputFile);
    emit logMessage(QString("Executing: %1 %2").arg(program, args.join(" ")));        
    bool success = executeProcess(program, args, inputFile);
        
        if (success) {
            successCount++;
            emit logMessage(QString("Successfully converted: %1").arg(QFileInfo(inputFile).fileName()));
            emit fileCompleted(inputFile, true, "Conversion successful");
        } else {
            failureCount++;
            failedFiles << inputFile;
            QString error = QString("Failed to convert: %1").arg(QFileInfo(inputFile).fileName());
            emit logMessage(error);
            emit fileCompleted(inputFile, false, error);
        }
        
        // Small delay to prevent overwhelming the system
        msleep(10);
    }
    
    // Emit final results
    QString finalMessage;
    bool overallSuccess = failureCount == 0;
    
    if (overallSuccess) {
        finalMessage = QString("All %1 files converted successfully").arg(successCount);
    } else {
        finalMessage = QString("Conversion completed: %1 successful, %2 failed")
                      .arg(successCount)
                      .arg(failureCount);
        if (!failedFiles.isEmpty()) {
            finalMessage += QString("\nFailed files:\n%1").arg(failedFiles.join("\n"));
        }
    }
    
    emit logMessage(finalMessage);
    emit finished(overallSuccess, finalMessage);
}

QStringList ConversionWorker::buildArguments(const QString &inputFile, const QString &outputFile) const
{
    QStringList args;
    
    // White balance method
    args << "--wb-method" << m_parameters.wbMethod;
    
    // Illuminant (if needed)
    if (m_parameters.wbMethod == "illuminant") {
        args << "--illuminant" << m_parameters.illuminant;
    }
    
    // White balance box (if needed)
    if (m_parameters.wbMethod == "box" && 
        (m_parameters.wbBoxSize.width() > 0 || m_parameters.wbBoxSize.height() > 0)) {
        args << "--wb-box" 
             << QString::number(m_parameters.wbBoxOrigin.x())
             << QString::number(m_parameters.wbBoxOrigin.y())
             << QString::number(m_parameters.wbBoxSize.width())
             << QString::number(m_parameters.wbBoxSize.height());
    }
    
    // Custom white balance (if needed)
    if (m_parameters.wbMethod == "custom") {
        args << "--custom-wb"
             << QString::number(m_parameters.customWb[0])
             << QString::number(m_parameters.customWb[1])
             << QString::number(m_parameters.customWb[2])
             << QString::number(m_parameters.customWb[3]);
    }
    
    // Matrix method
    args << "--mat-method" << m_parameters.matMethod;
    
    // Custom matrix (if needed)
    if (m_parameters.matMethod == "custom" && m_parameters.customMatrix.size() == 9) {
        args << "--custom-mat";
        for (double value : m_parameters.customMatrix) {
            args << QString::number(value);
        }
    }
    
    // Custom camera make/model (if needed)
    if (!m_parameters.customCameraMake.isEmpty()) {
        args << "--custom-camera-make" << m_parameters.customCameraMake;
    }
    if (!m_parameters.customCameraModel.isEmpty()) {
        args << "--custom-camera-model" << m_parameters.customCameraModel;
    }
    
    // Processing parameters
    if (m_parameters.headroom != 6.0) {
        args << "--headroom" << QString::number(m_parameters.headroom);
    }
    if (m_parameters.scale != 1.0) {
        args << "--scale" << QString::number(m_parameters.scale);
    }
    if (m_parameters.autobrightEnabled) {
        args << "--auto-bright";
    }
    if (m_parameters.adjustMaxThreshold != 0.75) {
        args << "--adjust-maximum-threshold" << QString::number(m_parameters.adjustMaxThreshold);
    }
    if (m_parameters.blackLevel >= 0) {
        args << "--black-level" << QString::number(m_parameters.blackLevel);
    }
    if (m_parameters.saturationLevel > 0) {
        args << "--saturation-level" << QString::number(m_parameters.saturationLevel);
    }
    if (m_parameters.chromaticAberration.x() != 1.0 || m_parameters.chromaticAberration.y() != 1.0) {
        args << "--chromatic-aberration" 
             << QString::number(m_parameters.chromaticAberration.x())
             << QString::number(m_parameters.chromaticAberration.y());
    }
    if (m_parameters.halfSize) {
        args << "--half-size";
    }
    if (m_parameters.highlightMode != 0) {
        args << "--highlight-mode" << QString::number(m_parameters.highlightMode);
    }
    if (m_parameters.cropBox.width() > 0 || m_parameters.cropBox.height() > 0) {
        args << "--crop-box"
             << QString::number(m_parameters.cropBox.x())
             << QString::number(m_parameters.cropBox.y())
             << QString::number(m_parameters.cropBox.width())
             << QString::number(m_parameters.cropBox.height());
    }
    if (m_parameters.cropMode != "soft") {
        args << "--crop-mode" << m_parameters.cropMode;
    }
    if (m_parameters.flip != 0) {
        args << "--flip" << QString::number(m_parameters.flip);
    }
    if (m_parameters.denoiseThreshold > 0.0) {
        args << "--denoise-threshold" << QString::number(m_parameters.denoiseThreshold);
    }
    if (m_parameters.demosaicAlgorithm != "AHD") {
        args << "--demosaic" << m_parameters.demosaicAlgorithm;
    }
    
    // Output options
    if (m_parameters.overwrite) {
        args << "--overwrite";
    }
    if (!m_parameters.outputDir.isEmpty()) {
        args << "--output-dir" << m_parameters.outputDir;
    }
    if (m_parameters.createDirs) {
        args << "--create-dirs";
    }
    // Note: rawtoaces does not support a cache control flag currently.
    
    // Debug options
    if (m_parameters.verbose) {
        args << "--verbose";
    }
    if (m_parameters.useTiming) {
        args << "--use-timing";
    }
    
    // Input file
    args << inputFile;
    return args;
}

QString ConversionWorker::getOutputFilename(const QString &inputFile) const
{
    QFileInfo inputInfo(inputFile);
    QString baseName = inputInfo.completeBaseName();
    
    QString outputDir;
    if (!m_parameters.outputDir.isEmpty()) {
        outputDir = m_parameters.outputDir;
    } else {
        outputDir = inputInfo.absolutePath();
    }
    
    return QDir(outputDir).absoluteFilePath(baseName + ".exr");
}

QString ConversionWorker::resolveRawtoacesProgram() const
{
    // Prefer env var if provided
    QString fromEnv = qEnvironmentVariable("RAWTOACES_BIN");
    if (!fromEnv.isEmpty() && QFileInfo::exists(fromEnv)) return fromEnv;
    // Common names; rely on PATH
    return QStringLiteral("rawtoaces");
}

bool ConversionWorker::executeProcess(const QString &program, const QStringList &args, const QString &filename)
{
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    // Inherit environment and pass through RAWTOACES_DATA_PATH if set
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    process.setProcessEnvironment(env);

    // Stream output incrementally for progress logs
    connect(&process, &QProcess::readyReadStandardOutput, [&]() {
        QByteArray out = process.readAllStandardOutput();
        if (!out.isEmpty()) emit logMessage(QString::fromUtf8(out));
    });

    // Start
    process.start(program, args, QIODevice::ReadOnly);
    if (!process.waitForStarted(10000)) {
        emit logMessage(QString("Failed to start conversion process for %1").arg(filename));
        return false;
    }

    // Timeout: 10 minutes per file
    const int timeoutMs = 10 * 60 * 1000;
    bool finished = process.waitForFinished(timeoutMs);
    if (!finished) {
        emit logMessage(QString("Conversion timeout for %1").arg(filename));
        process.kill();
        process.waitForFinished(5000);
        return false;
    }

    // Drain remaining output
    QByteArray output = process.readAllStandardOutput();
    if (!output.isEmpty()) emit logMessage(QString::fromUtf8(output));

    int exitCode = process.exitCode();
    if (exitCode != 0) {
        emit logMessage(QString("rawtoaces exited with code %1 for %2").arg(exitCode).arg(filename));
        return false;
    }
    return true;
}
