#include "ImageViewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWheelEvent>
#include <QScrollBar>
#include <QApplication>
#include <QFileInfo>
#include <QPixmap>
#include <QImageReader>
#include <QMessageBox>
#include <QPainter>
#include "utils/ImageUtils.h"

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_imageLabel(nullptr)
    , m_infoLabel(nullptr)
    , m_scaleFactor(1.0)
    , m_selectionMode(false)
    , m_rubberBand(nullptr)
{
    setupUI();
}

void ImageViewer::setupUI()
{
    setWindowTitle("Image Viewer");
    
    // Create image label
    m_imageLabel = new QLabel;
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("QLabel { background-color: #2b2b2b; }");
    m_imageLabel->setMinimumSize(200, 200);
    m_imageLabel->setText("No image loaded");
    
    // Create scroll area
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidget(m_imageLabel);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    
    // Create toolbar
    QHBoxLayout *toolbarLayout = new QHBoxLayout;
    
    m_zoomInButton = new QPushButton("Zoom In");
    m_zoomOutButton = new QPushButton("Zoom Out");
    m_zoomFitButton = new QPushButton("Fit");
    m_zoomActualButton = new QPushButton("100%");
    m_selectToggleButton = new QPushButton("Select");
    m_selectToggleButton->setCheckable(true);
    
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setRange(10, 500); // 10% to 500%
    m_zoomSlider->setValue(100);
    m_zoomSlider->setToolTip("Zoom level");
    
    m_zoomLabel = new QLabel("100%");
    m_zoomLabel->setMinimumWidth(50);
    
    toolbarLayout->addWidget(m_zoomInButton);
    toolbarLayout->addWidget(m_zoomOutButton);
    toolbarLayout->addWidget(m_zoomFitButton);
    toolbarLayout->addWidget(m_zoomActualButton);
    toolbarLayout->addWidget(m_selectToggleButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(new QLabel("Zoom:"));
    toolbarLayout->addWidget(m_zoomSlider);
    toolbarLayout->addWidget(m_zoomLabel);
    
    // Info label
    m_infoLabel = new QLabel("Ready");
    m_infoLabel->setStyleSheet("QLabel { color: #888; }");
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addWidget(m_scrollArea);
    mainLayout->addWidget(m_infoLabel);
    setLayout(mainLayout);
    
    // Connect signals
    connect(m_zoomInButton, &QPushButton::clicked, this, &ImageViewer::zoomIn);
    connect(m_zoomOutButton, &QPushButton::clicked, this, &ImageViewer::zoomOut);
    connect(m_zoomFitButton, &QPushButton::clicked, this, &ImageViewer::zoomFit);
    connect(m_zoomActualButton, &QPushButton::clicked, this, &ImageViewer::zoomActual);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &ImageViewer::onZoomSliderChanged);
    connect(m_selectToggleButton, &QPushButton::toggled, this, &ImageViewer::setSelectionMode);
}

void ImageViewer::loadImage(const QString &filename)
{
    if (filename.isEmpty() || !QFile::exists(filename)) {
        clearImage();
        return;
    }
    
    m_currentFilename = filename;
    
    // Load preview using ImageUtils (uses LibRaw if available)
    QImage preview = ImageUtils::loadPreview(filename, 2048, 2048);
    if (preview.isNull()) {
        // Fallback placeholder
        QPixmap placeholder(400, 300);
        placeholder.fill(QColor(64, 64, 64));
        QPainter painter(&placeholder);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 12));
        painter.drawText(placeholder.rect(), Qt::AlignCenter,
                         QString("No preview available\n%1").arg(QFileInfo(filename).fileName()));
        m_originalPixmap = placeholder;
    } else {
        m_originalPixmap = QPixmap::fromImage(preview);
    }
    updateImage();
    
    // Update info
    QFileInfo fileInfo(filename);
    m_infoLabel->setText(QString("File: %1 | Size: %2")
                        .arg(fileInfo.fileName())
                        .arg(formatFileSize(fileInfo.size())));
}

void ImageViewer::clearImage()
{
    m_originalPixmap = QPixmap();
    m_imageLabel->clear();
    m_imageLabel->setText("No image loaded");
    m_currentFilename.clear();
    m_infoLabel->setText("Ready");
}

void ImageViewer::setSelectionMode(bool enabled)
{
    m_selectionMode = enabled;
    if (!enabled && m_rubberBand) {
        m_rubberBand->hide();
    }
}

QRect ImageViewer::getSelection() const
{
    return m_selection;
}

QRect ImageViewer::getSelectionInImagePixels() const
{
    if (m_originalPixmap.isNull() || m_selection.isEmpty()) return QRect();
    // Map widget selection to label coords, then to image pixels
    QPoint topLeft = m_selection.topLeft() - m_imageLabel->pos();
    QPoint bottomRight = m_selection.bottomRight() - m_imageLabel->pos();
    QRect onLabel = QRect(topLeft, bottomRight).normalized();
    if (onLabel.isEmpty()) return QRect();
    double invScale = (m_scaleFactor > 0.0) ? (1.0 / m_scaleFactor) : 1.0;
    QRect imgRect = QRect(
        qMax(0, qRound(onLabel.x() * invScale)),
        qMax(0, qRound(onLabel.y() * invScale)),
        qMax(0, qRound(onLabel.width() * invScale)),
        qMax(0, qRound(onLabel.height() * invScale))
    ).intersected(QRect(QPoint(0,0), m_originalPixmap.size()));
    return imgRect;
}

void ImageViewer::zoomIn()
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

void ImageViewer::zoomFit()
{
    if (m_originalPixmap.isNull()) return;
    
    QSize scrollSize = m_scrollArea->viewport()->size();
    QSize pixmapSize = m_originalPixmap.size();
    
    double scaleX = (double)scrollSize.width() / pixmapSize.width();
    double scaleY = (double)scrollSize.height() / pixmapSize.height();
    double scale = qMin(scaleX, scaleY);
    
    setZoom(scale * 100);
}

void ImageViewer::zoomActual()
{
    setZoom(100);
}

void ImageViewer::onZoomSliderChanged(int value)
{
    setZoom(value);
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (m_selectionMode && event->button() == Qt::LeftButton) {
        m_selectionStart = event->pos();
        if (!m_rubberBand) {
            m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
        }
        m_rubberBand->setGeometry(QRect(m_selectionStart, QSize()));
        m_rubberBand->show();
    }
    
    emit imageClicked(event->pos());
    QWidget::mousePressEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_selectionMode && m_rubberBand && m_rubberBand->isVisible()) {
        QRect selection = QRect(m_selectionStart, event->pos()).normalized();
        m_rubberBand->setGeometry(selection);
    }
    
    QWidget::mouseMoveEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_selectionMode && m_rubberBand && m_rubberBand->isVisible()) {
        m_selection = QRect(m_selectionStart, event->pos()).normalized();
        m_rubberBand->hide();
        emit selectionChanged(m_selection);
    }
    
    QWidget::mouseReleaseEvent(event);
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom with Ctrl+Wheel
        const double scaleFactor = 1.15;
        if (event->angleDelta().y() > 0) {
            scaleImage(scaleFactor);
        } else {
            scaleImage(1.0 / scaleFactor);
        }
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

void ImageViewer::updateImage()
{
    if (m_originalPixmap.isNull()) return;
    
    m_scaledPixmap = m_originalPixmap.scaled(
        m_originalPixmap.size() * m_scaleFactor,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    
    m_imageLabel->setPixmap(m_scaledPixmap);
    m_imageLabel->resize(m_scaledPixmap.size());
}

void ImageViewer::scaleImage(double factor)
{
    m_scaleFactor *= factor;
    m_scaleFactor = qBound(0.1, m_scaleFactor, 5.0);
    
    updateImage();
    
    // Update zoom controls
    int zoomPercent = qRound(m_scaleFactor * 100);
    m_zoomSlider->setValue(zoomPercent);
    m_zoomLabel->setText(QString("%1%").arg(zoomPercent));
    
    // Adjust scroll bars
    if (m_scrollArea) {
        QScrollBar *hBar = m_scrollArea->horizontalScrollBar();
        QScrollBar *vBar = m_scrollArea->verticalScrollBar();
        
        int hValue = qRound(hBar->value() * factor);
        int vValue = qRound(vBar->value() * factor);
        
        hBar->setValue(hValue);
        vBar->setValue(vValue);
    }
}

void ImageViewer::setZoom(double zoom)
{
    double newScale = zoom / 100.0;
    m_scaleFactor = qBound(0.1, newScale, 5.0);
    
    updateImage();
    
    // Update controls
    int zoomPercent = qRound(m_scaleFactor * 100);
    m_zoomSlider->blockSignals(true);
    m_zoomSlider->setValue(zoomPercent);
    m_zoomSlider->blockSignals(false);
    m_zoomLabel->setText(QString("%1%").arg(zoomPercent));
}

QString ImageViewer::formatFileSize(qint64 size) const
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
