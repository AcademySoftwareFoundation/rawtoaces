#pragma once

#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QImage>
#include <QSize>

/**
 * @brief Utility functions for image operations
 */
namespace ImageUtils {

/**
 * @brief Load an image from file with automatic format detection
 * @param filename The image file path
 * @return QPixmap containing the loaded image
 */
QPixmap loadImage(const QString &filename);

/**
 * @brief Load a RAW file thumbnail/preview
 * @param filename The RAW file path
 * @return QPixmap containing the preview image
 */
QPixmap loadRawPreview(const QString &filename);

/**
 * @brief Generate a placeholder image for unsupported formats
 * @param filename The file path (for display in placeholder)
 * @param size The size of the placeholder
 * @return QPixmap placeholder image
 */
QPixmap generatePlaceholder(const QString &filename, const QSize &size = QSize(400, 300));

/**
 * @brief Scale an image to fit within given dimensions while maintaining aspect ratio
 * @param pixmap The source pixmap
 * @param maxSize Maximum dimensions
 * @return Scaled pixmap
 */
QPixmap scaleToFit(const QPixmap &pixmap, const QSize &maxSize);

/**
 * @brief Scale an image by a factor
 * @param pixmap The source pixmap
 * @param scale Scale factor (1.0 = original size)
 * @return Scaled pixmap
 */
QPixmap scaleImage(const QPixmap &pixmap, double scale);

/**
 * @brief Get image dimensions without fully loading the image
 * @param filename The image file path
 * @return Image size, or invalid size if cannot be determined
 */
QSize getImageSize(const QString &filename);

/**
 * @brief Check if an image format is supported by Qt
 * @param filename The image file path
 * @return true if the format is supported
 */
bool isFormatSupported(const QString &filename);

/**
 * @brief Get a list of supported image formats
 * @return List of supported file extensions
 */
QStringList getSupportedFormats();

/**
 * @brief Convert an image to a different format
 * @param sourceFile Source image file
 * @param targetFile Target file path
 * @param format Target format (e.g., "PNG", "JPEG")
 * @param quality Quality for lossy formats (0-100)
 * @return true if conversion was successful
 */
bool convertImage(const QString &sourceFile, const QString &targetFile, 
                  const QString &format, int quality = 90);

/**
 * @brief Extract EXIF data from an image file
 * @param filename The image file path
 * @return Map of EXIF tag names to values
 */
QMap<QString, QString> extractExifData(const QString &filename);

/**
 * @brief Generate a histogram for an image
 * @param image The source image
 * @return Histogram data (256 values for each RGB channel)
 */
struct HistogramData {
    QVector<int> red;
    QVector<int> green;
    QVector<int> blue;
    QVector<int> luminance;
};

HistogramData generateHistogram(const QImage &image);

/**
 * @brief Apply basic color corrections to an image
 * @param image Source image
 * @param brightness Brightness adjustment (-100 to 100)
 * @param contrast Contrast adjustment (-100 to 100)
 * @param saturation Saturation adjustment (-100 to 100)
 * @return Adjusted image
 */
QImage adjustColors(const QImage &image, int brightness = 0, int contrast = 0, int saturation = 0);

/**
 * @brief Create a thumbnail image
 * @param sourceFile Source image file
 * @param thumbnailSize Maximum thumbnail dimensions
 * @return Thumbnail pixmap
 */
QPixmap createThumbnail(const QString &sourceFile, const QSize &thumbnailSize = QSize(128, 128));

} // namespace ImageUtils
