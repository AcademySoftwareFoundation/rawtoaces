#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QFile>

/**
 * @brief Utility functions for file operations
 */
namespace FileUtils {

/**
 * @brief Check if a file is a RAW image file based on its extension
 * @param filename The filename to check
 * @return true if the file is a RAW file
 */
bool isRawFile(const QString &filename);

/**
 * @brief Check if a file is an image file (including RAW files)
 * @param filename The filename to check
 * @return true if the file is an image file
 */
bool isImageFile(const QString &filename);

/**
 * @brief Get the file extension (without the dot)
 * @param filename The filename
 * @return The file extension in lowercase
 */
QString getFileExtension(const QString &filename);

/**
 * @brief Get the base name of a file (without path and extension)
 * @param filename The filename
 * @return The base name
 */
QString getBaseName(const QString &filename);

/**
 * @brief Get the directory path of a file
 * @param filename The filename
 * @return The directory path
 */
QString getDirectory(const QString &filename);

/**
 * @brief Get the size of a file in bytes
 * @param filename The filename
 * @return The file size in bytes
 */
qint64 getFileSize(const QString &filename);

/**
 * @brief Format a file size in human-readable format
 * @param size The size in bytes
 * @return Formatted size string (e.g., "1.5 MB")
 */
QString formatFileSize(qint64 size);

/**
 * @brief Get the last modification time of a file
 * @param filename The filename
 * @return The modification time
 */
QDateTime getFileModificationTime(const QString &filename);

/**
 * @brief Check if a file exists
 * @param filename The filename to check
 * @return true if the file exists
 */
bool fileExists(const QString &filename);

/**
 * @brief Create a directory (including parent directories)
 * @param path The directory path to create
 * @return true if successful
 */
bool createDirectory(const QString &path);

/**
 * @brief Generate a unique filename by appending a number if the file exists
 * @param filename The original filename
 * @return A unique filename
 */
QString getUniqueFilename(const QString &filename);

/**
 * @brief Get the application data directory
 * @return Path to the application data directory
 */
QString getApplicationDataPath();

/**
 * @brief Get a temporary directory for the application
 * @return Path to the temporary directory
 */
QString getTemporaryPath();

/**
 * @brief Get all files in a directory matching the given filters
 * @param directory The directory to search
 * @param filters File name filters (e.g., {"*.jpg", "*.png"})
 * @param recursive Whether to search recursively
 * @return List of matching file paths
 */
QStringList getFilesInDirectory(const QString &directory, 
                               const QStringList &filters = QStringList(), 
                               bool recursive = false);

/**
 * @brief Get all RAW files in a directory
 * @param directory The directory to search
 * @param recursive Whether to search recursively
 * @return List of RAW file paths
 */
QStringList getRawFilesInDirectory(const QString &directory, bool recursive = false);

/**
 * @brief Copy a file
 * @param source Source file path
 * @param destination Destination file path
 * @return true if successful
 */
bool copyFile(const QString &source, const QString &destination);

/**
 * @brief Move a file
 * @param source Source file path
 * @param destination Destination file path
 * @return true if successful
 */
bool moveFile(const QString &source, const QString &destination);

/**
 * @brief Delete a file
 * @param filename The file to delete
 * @return true if successful
 */
bool deleteFile(const QString &filename);

/**
 * @brief Make a relative path from one path to another
 * @param fromPath The base path
 * @param toPath The target path
 * @return Relative path
 */
QString makeRelativePath(const QString &fromPath, const QString &toPath);

/**
 * @brief Make an absolute path from a base path and relative path
 * @param basePath The base path
 * @param relativePath The relative path
 * @return Absolute path
 */
QString makeAbsolutePath(const QString &basePath, const QString &relativePath);

/**
 * @brief Validate if a path exists and is readable
 * @param path The path to validate
 * @return true if the path is valid
 */
bool validatePath(const QString &path);

} // namespace FileUtils
