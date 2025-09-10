#include "ImageUtils.h"
#include <QImageReader>
#include <QPainter>
#include <QFileInfo>
#include <QGuiApplication>
#include <QScreen>
#include <QPen>
#include <QBrush>
#include "FileUtils.h"
#include "RawPreview.h"

namespace ImageUtils {

QImage createPlaceholder(const QString &label, int width, int height)
{
    QImage img(width, height, QImage::Format_ARGB32_Premultiplied);
    img.fill(QColor(64, 64, 64));

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 12));

    QRect r = QRect(0, 0, width, height);
    p.drawText(r, Qt::AlignCenter, label);

    return img;
}

QImage loadPreview(const QString &filepath, int maxWidth, int maxHeight)
{
    QFileInfo fi(filepath);
    if (!fi.exists()) {
        return createPlaceholder("File not found", maxWidth, maxHeight);
    }

    // Try RAW embedded preview first if it's a RAW file
    if (FileUtils::isRawFile(filepath)) {
        QImage rawThumb = RawPreview::extractEmbeddedPreview(filepath);
        if (!rawThumb.isNull()) {
            return rawThumb.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    // Try Qt-supported formats
    QImageReader reader(filepath);
    if (reader.canRead()) {
        reader.setAutoTransform(true);
        QImage img = reader.read();
        if (!img.isNull()) {
            img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            img = img.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            return img;
        }
    }

    // Fallback placeholder
    return createPlaceholder(QString("Preview not available\n%1").arg(fi.fileName()), maxWidth, maxHeight);
}

QImage frameImage(const QImage &src, int targetWidth, int targetHeight,
                  const QColor &background, const QColor &border)
{
    QImage canvas(targetWidth, targetHeight, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(background);

    if (!src.isNull()) {
        QSize target(targetWidth - 4, targetHeight - 4); // padding for border
        QImage scaled = src.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPoint topLeft((targetWidth - scaled.width())/2, (targetHeight - scaled.height())/2);
        QPainter p(&canvas);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawImage(topLeft, scaled);
        // border
        p.setPen(QPen(border));
        p.drawRect(0, 0, targetWidth-1, targetHeight-1);
    }
    return canvas;
}

QImage loadFramedThumbnail(const QString &filepath, int targetWidth, int targetHeight,
                           const QColor &background, const QColor &border)
{
    QImage preview = loadPreview(filepath, targetWidth, targetHeight);
    return frameImage(preview, targetWidth, targetHeight, background, border);
}

QPixmap toPixmap(const QImage &image)
{
    QPixmap pm = QPixmap::fromImage(image);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    if (QGuiApplication::primaryScreen()) {
        pm.setDevicePixelRatio(QGuiApplication::primaryScreen()->devicePixelRatio());
    }
#endif
    return pm;
}

} // namespace ImageUtils
