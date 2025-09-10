#pragma once

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QPixmap>
#include <QMouseEvent>
#include <QRubberBand>

class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    
    void loadImage(const QString &filename);
    void clearImage();
    void setSelectionMode(bool enabled);
    QRect getSelection() const;
    QRect getSelectionInImagePixels() const;

signals:
    void selectionChanged(const QRect &selection);
    void imageClicked(const QPoint &position);

private slots:
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomActual();
    void onZoomSliderChanged(int value);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void setupUI();
    void updateImage();
    void scaleImage(double factor);
    void setZoom(double zoom);
    QString formatFileSize(qint64 size) const; // declaration added
    QPixmap generateThumbnail(const QString &filename);
    
    QScrollArea *m_scrollArea;
    QLabel *m_imageLabel;
    QLabel *m_infoLabel;
    QPushButton *m_zoomInButton;
    QPushButton *m_zoomOutButton;
    QPushButton *m_zoomFitButton;
    QPushButton *m_zoomActualButton;
    QPushButton *m_selectToggleButton;
    QSlider *m_zoomSlider;
    QLabel *m_zoomLabel;
    
    QPixmap m_originalPixmap;
    QPixmap m_scaledPixmap;
    double m_scaleFactor;
    bool m_selectionMode;
    QRubberBand *m_rubberBand;
    QPoint m_selectionStart;
    QRect m_selection;
    
    QString m_currentFilename;
};
