#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>

struct ConversionParameters {
    // White balance
    QString wbMethod = "metadata";
    QString illuminant = "D55";
    QPoint wbBoxOrigin = QPoint(0, 0);
    QSize wbBoxSize = QSize(0, 0);
    QVector<double> customWb = {1.0, 1.0, 1.0, 1.0};
    
    // Matrix calculation
    QString matMethod = "spectral";
    QVector<double> customMatrix;
    QString customCameraMake;
    QString customCameraModel;
    
    // Processing options
    double headroom = 6.0;
    double scale = 1.0;
    bool autobrightEnabled = false;
    double adjustMaxThreshold = 0.75;
    int blackLevel = -1;
    int saturationLevel = 0;
    QPointF chromaticAberration = QPointF(1.0, 1.0);
    bool halfSize = false;
    int highlightMode = 0;
    QRect cropBox = QRect(0, 0, 0, 0);
    QString cropMode = "soft";
    int flip = 0;
    double denoiseThreshold = 0.0;
    QString demosaicAlgorithm = "AHD";
    
    // Output options
    bool overwrite = false;
    QString outputDir;
    bool createDirs = false;
    bool useCache = true;
    
    // Debug options
    bool verbose = false;
    bool useTiming = false;
};

class ParameterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterWidget(QWidget *parent = nullptr);
    
    ConversionParameters getParameters() const;
    void setParameters(const ConversionParameters &params);
    void resetToDefaults();
    // Apply rectangles from viewer (image pixel coords)
    void setWbBoxFromSelection(const QRect &rect);
    void setCropBoxFromSelection(const QRect &rect);

signals:
    void parametersChanged();

private slots:
    void onWbMethodChanged();
    void onMatMethodChanged();
    void onParameterChanged();
    void chooseOutputDirectory();
    void showAdvancedOptions();

private:
    void setupUI();
    void setupWhiteBalanceGroup();
    void setupMatrixGroup();
    void setupProcessingGroup();
    void setupOutputGroup();
    void setupAdvancedGroup();
    void updateWbControls();
    void updateMatControls();
    
    // UI Groups
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QGroupBox *m_wbGroup;
    QGroupBox *m_matrixGroup;
    QGroupBox *m_processingGroup;
    QGroupBox *m_outputGroup;
    QGroupBox *m_advancedGroup;
    
    // White balance controls
    QComboBox *m_wbMethodCombo;
    QComboBox *m_illuminantCombo;
    QLineEdit *m_customIlluminantEdit;
    QSpinBox *m_wbBoxXSpin;
    QSpinBox *m_wbBoxYSpin;
    QSpinBox *m_wbBoxWSpin;
    QSpinBox *m_wbBoxHSpin;
    QDoubleSpinBox *m_customWbR;
    QDoubleSpinBox *m_customWbG1;
    QDoubleSpinBox *m_customWbB;
    QDoubleSpinBox *m_customWbG2;
    
    // Matrix controls
    QComboBox *m_matMethodCombo;
    QLineEdit *m_customMakeEdit;
    QLineEdit *m_customModelEdit;
    QWidget *m_customMatrixWidget;
    QVector<QDoubleSpinBox*> m_matrixSpins;
    
    // Processing controls
    QDoubleSpinBox *m_headroomSpin;
    QDoubleSpinBox *m_scaleSpin;
    QCheckBox *m_autobrightCheck;
    QDoubleSpinBox *m_adjustMaxSpin;
    QSpinBox *m_blackLevelSpin;
    QSpinBox *m_saturationSpin;
    QDoubleSpinBox *m_chromAberrationR;
    QDoubleSpinBox *m_chromAberrationB;
    QCheckBox *m_halfSizeCheck;
    QComboBox *m_highlightModeCombo;
    QComboBox *m_demosaicCombo;
    
    // Output controls
    QLineEdit *m_outputDirEdit;
    QPushButton *m_browseOutputButton;
    QCheckBox *m_overwriteCheck;
    QCheckBox *m_createDirsCheck;
    QCheckBox *m_disableCacheCheck;
    
    // Advanced controls
    QWidget *m_advancedWidget;
    bool m_advancedVisible;
    QPushButton *m_advancedToggleButton;
    QSpinBox *m_cropXSpin;
    QSpinBox *m_cropYSpin;
    QSpinBox *m_cropWSpin;
    QSpinBox *m_cropHSpin;
    QComboBox *m_cropModeCombo;
    QComboBox *m_flipCombo;
    QDoubleSpinBox *m_denoiseSpin;
    QCheckBox *m_verboseCheck;
    QCheckBox *m_timingCheck;
};
