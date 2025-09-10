#include "RawPreview.h"

#ifdef USE_LIBRAW
#include <libraw/libraw.h>
#endif

#include <QImage>
#include <QByteArray>

namespace RawPreview {

QImage extractEmbeddedPreview(const QString &filepath)
{
#ifdef USE_LIBRAW
    libraw_data_t proc;
    if (libraw_open_file(&proc, filepath.toUtf8().constData()) != LIBRAW_SUCCESS) {
        libraw_close(&proc);
        return QImage();
    }

    if (libraw_unpack_thumb(&proc) != LIBRAW_SUCCESS) {
        libraw_close(&proc);
        return QImage();
    }

    QImage img;
    if (proc.thumbnail.tformat == LIBRAW_THUMBNAIL_JPEG
#ifdef LIBRAW_THUMBNAIL_JPEG2000
    || proc.thumbnail.tformat == LIBRAW_THUMBNAIL_JPEG2000
#endif
    ) {
        QByteArray bytes(reinterpret_cast<const char*>(proc.thumbnail.thumb), proc.thumbnail.tlength);
        img = QImage::fromData(bytes);
    } else if (proc.thumbnail.tformat == LIBRAW_THUMBNAIL_BITMAP) {
        // 8-bit RGB
        int w = proc.thumbnail.twidth;
        int h = proc.thumbnail.theight;
        int stride = w * 3;
        const uchar* data = reinterpret_cast<const uchar*>(proc.thumbnail.thumb);
        QImage temp(data, w, h, stride, QImage::Format_RGB888);
        img = temp.copy(); // copy away from LibRaw buffer
    }

    libraw_close(&proc);
    return img;
#else
    Q_UNUSED(filepath);
    return QImage();
#endif
}

} // namespace RawPreview
