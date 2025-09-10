#pragma once

#include <QImage>
#include <QPixmap>
#include <QString>
#include <QColor>

namespace ImageUtils {

// Load a preview image for a given file (RAW files get a placeholder)
QImage loadPreview(const QString &filepath, int maxWidth = 1024, int maxHeight = 1024);

// Load a preview and render into a framed thumbnail of target size
QImage loadFramedThumbnail(const QString &filepath, int targetWidth = 96, int targetHeight = 72,
						   const QColor &background = QColor(40,40,40),
						   const QColor &border = QColor(80,80,80));

// Convert QImage to QPixmap with device pixel ratio handling
QPixmap toPixmap(const QImage &image);

// Create a placeholder preview for non-decodable files
QImage createPlaceholder(const QString &label, int width = 400, int height = 300);

// Draw an image centered on a background with a subtle border
QImage frameImage(const QImage &src, int targetWidth, int targetHeight,
				  const QColor &background = QColor(40,40,40),
				  const QColor &border = QColor(80,80,80));

}
