#include "../../include/utils/FileUtils.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

namespace FileUtils {

bool isRawFile(const QString &filename)
{
    static const QStringList rawExtensions = {
        "cr2", "cr3", "nef", "arw", "dng", "raf", "orf", "rw2", "pef", "srw",
        "x3f", "bay", "bmq", "cs1", "dc2", "dcr", "fff", "hdr", "k25", "kdc",
        "mdc", "mos", "mrw", "raw", "rwl", "sr2", "srf", "sti", "3fr", "ari",
        "cap", "iiq", "eip", "dcs", "dcr", "drf", "erf", "gpr", "iiq", "mef",
        "mfw", "nrw", "ptx", "pxn", "r3d", "rwz", "srf", "srw2"
    };
    
    QFileInfo fileInfo(filename);
    QString extension = fileInfo.suffix().toLower();
    return rawExtensions.contains(extension);
}

bool isImageFile(const QString &filename)
{
    static const QStringList imageExtensions = {
        "jpg", "jpeg", "png", "tiff", "tif", "bmp", "gif", "webp", "exr", "hdr"
    };
    
    QFileInfo fileInfo(filename);
    QString extension = fileInfo.suffix().toLower();
    return imageExtensions.contains(extension) || isRawFile(filename);
}

QString getFileExtension(const QString &filename)
{
    QFileInfo fileInfo(filename);
    return fileInfo.suffix().toLower();
}

QString getBaseName(const QString &filename)
{
    QFileInfo fileInfo(filename);
    return fileInfo.baseName();
}

QString getDirectory(const QString &filename)
{
    QFileInfo fileInfo(filename);
    return fileInfo.absolutePath();
}

qint64 getFileSize(const QString &filename)
{
    QFileInfo fileInfo(filename);
    return fileInfo.size();
}

QString formatFileSize(qint64 size)
{
    const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double sizeDouble = size;
    
    while (sizeDouble >= 1024 && unitIndex < units.size() - 1) {
        sizeDouble /= 1024;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(sizeDouble, 0, 'f', 1).arg(units[unitIndex]);
}

QDateTime getFileModificationTime(const QString &filename)
{
    QFileInfo fileInfo(filename);
    return fileInfo.lastModified();
}

bool fileExists(const QString &filename)
{
    return QFile::exists(filename);
}

bool createDirectory(const QString &path)
{
    QDir dir;
    return dir.mkpath(path);
}

QString getUniqueFilename(const QString &filename)
{
    if (!QFile::exists(filename)) {
        return filename;
    }
    
    QFileInfo fileInfo(filename);
    QString baseName = fileInfo.baseName();
    QString extension = fileInfo.suffix();
    QString directory = fileInfo.absolutePath();
    
    int counter = 1;
    QString newFilename;
    
    do {
        newFilename = QString("%1/%2_%3.%4")
                     .arg(directory)
                     .arg(baseName)
                     .arg(counter)
                     .arg(extension);
        counter++;
    } while (QFile::exists(newFilename));
    
    return newFilename;
}

QString getApplicationDataPath()
{
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "RAWtoACES_GUI";
    }
    
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + 
               "/.config/" + appName;
    }
    
    // Ensure the directory exists
    QDir dir;
    if (!dir.exists(path)) {
        dir.mkpath(path);
    }
    
    return path;
}

QString getTemporaryPath()
{
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString appTempPath = tempPath + "/RAWtoACES_GUI";
    
    QDir dir;
    if (!dir.exists(appTempPath)) {
        dir.mkpath(appTempPath);
    }
    
    return appTempPath;
}

QStringList getFilesInDirectory(const QString &directory, const QStringList &filters, bool recursive)
{
    QStringList files;
    QDir dir(directory);
    
    if (!dir.exists()) {
        return files;
    }
    
    QDir::Filters dirFilters = QDir::Files | QDir::Readable;
    if (recursive) {
        dirFilters |= QDir::AllDirs | QDir::NoDotAndDotDot;
    }
    
    QFileInfoList fileInfoList = dir.entryInfoList(filters, dirFilters);
    
    for (const QFileInfo &fileInfo : fileInfoList) {
        if (fileInfo.isFile()) {
            files.append(fileInfo.absoluteFilePath());
        } else if (fileInfo.isDir() && recursive) {
            files.append(getFilesInDirectory(fileInfo.absoluteFilePath(), filters, recursive));
        }
    }
    
    return files;
}

QStringList getRawFilesInDirectory(const QString &directory, bool recursive)
{
    QStringList rawFilters;
    QStringList rawExtensions = {
        "*.cr2", "*.cr3", "*.nef", "*.arw", "*.dng", "*.raf", "*.orf", "*.rw2", 
        "*.pef", "*.srw", "*.x3f", "*.bay", "*.bmq", "*.cs1", "*.dc2", "*.dcr", 
        "*.fff", "*.hdr", "*.k25", "*.kdc", "*.mdc", "*.mos", "*.mrw", "*.raw", 
        "*.rwl", "*.sr2", "*.srf", "*.sti", "*.3fr", "*.ari", "*.cap", "*.iiq", 
        "*.eip", "*.dcs", "*.drf", "*.erf", "*.gpr", "*.mef", "*.mfw", "*.nrw", 
        "*.ptx", "*.pxn", "*.r3d", "*.rwz", "*.srw2"
    };
    
    // Add uppercase versions
    for (const QString &ext : rawExtensions) {
        rawFilters.append(ext.toUpper());
    }
    rawFilters.append(rawExtensions);
    
    return getFilesInDirectory(directory, rawFilters, recursive);
}

bool copyFile(const QString &source, const QString &destination)
{
    // Remove destination if it exists
    if (QFile::exists(destination)) {
        QFile::remove(destination);
    }
    
    return QFile::copy(source, destination);
}

bool moveFile(const QString &source, const QString &destination)
{
    // Remove destination if it exists
    if (QFile::exists(destination)) {
        QFile::remove(destination);
    }
    
    return QFile::rename(source, destination);
}

bool deleteFile(const QString &filename)
{
    return QFile::remove(filename);
}

QString makeRelativePath(const QString &fromPath, const QString &toPath)
{
    QDir fromDir(fromPath);
    return fromDir.relativeFilePath(toPath);
}

QString makeAbsolutePath(const QString &basePath, const QString &relativePath)
{
    QDir baseDir(basePath);
    return baseDir.absoluteFilePath(relativePath);
}

bool validatePath(const QString &path)
{
    QFileInfo fileInfo(path);
    return fileInfo.exists() && fileInfo.isReadable();
}

} // namespace FileUtils
