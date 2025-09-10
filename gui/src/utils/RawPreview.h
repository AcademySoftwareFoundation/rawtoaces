#pragma once

#include <QImage>
#include <QString>

namespace RawPreview {

// Try to extract an embedded thumbnail/preview via LibRaw (if available).
// Returns null QImage on failure.
QImage extractEmbeddedPreview(const QString &filepath);

}
