#include "ParameterWidget.h"
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>

ParameterWidget::ParameterWidget(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_wbGroup(nullptr)
    , m_matrixGroup(nullptr)
    , m_processingGroup(nullptr)
    , m_outputGroup(nullptr)
    , m_advancedGroup(nullptr)
    , m_advancedVisible(false)
{
    setupUI();
}

void ParameterWidget::setupUI()
{
    setWindowTitle("Conversion Parameters");
    
    // Create scroll area for parameters
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Create content widget
    m_contentWidget = new QWidget;
    
    // Setup parameter groups
    setupWhiteBalanceGroup();
    setupMatrixGroup();
    setupProcessingGroup();
    setupOutputGroup();
    setupAdvancedGroup();
    
    // Layout content widget
    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->addWidget(m_wbGroup);
    contentLayout->addWidget(m_matrixGroup);
    contentLayout->addWidget(m_processingGroup);
    contentLayout->addWidget(m_outputGroup);
    contentLayout->addWidget(m_advancedGroup);
    contentLayout->addStretch();
    
    m_contentWidget->setLayout(contentLayout);
    m_scrollArea->setWidget(m_contentWidget);
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_scrollArea);
    setLayout(mainLayout);
    
    // Connect signals
    connect(m_wbMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ParameterWidget::onWbMethodChanged);
    connect(m_matMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ParameterWidget::onMatMethodChanged);
    
    // Initialize UI state
    onWbMethodChanged();
    onMatMethodChanged();
}

void ParameterWidget::setupWhiteBalanceGroup()
{
    m_wbGroup = new QGroupBox("White Balance");
    QGridLayout *layout = new QGridLayout;
    
    // White balance method
    layout->addWidget(new QLabel("Method:"), 0, 0);
    m_wbMethodCombo = new QComboBox;
    m_wbMethodCombo->addItems({"metadata", "illuminant", "box", "custom"});
    m_wbMethodCombo->setToolTip("White balance calculation method");
    layout->addWidget(m_wbMethodCombo, 0, 1);
    
    // Illuminant
    layout->addWidget(new QLabel("Illuminant:"), 1, 0);
    m_illuminantCombo = new QComboBox;
    m_illuminantCombo->addItems({"D50", "D55", "D60", "D65", "D75"});
    m_illuminantCombo->setCurrentText("D55");
    layout->addWidget(m_illuminantCombo, 1, 1);
    
    // Custom illuminant
    m_customIlluminantEdit = new QLineEdit;
    m_customIlluminantEdit->setPlaceholderText("e.g., 3200K, D60");
    layout->addWidget(m_customIlluminantEdit, 1, 2);
    
    // WB Box controls
    layout->addWidget(new QLabel("WB Box:"), 2, 0);
    QHBoxLayout *boxLayout = new QHBoxLayout;
    
    m_wbBoxXSpin = new QSpinBox;
    m_wbBoxXSpin->setRange(0, 9999);
    m_wbBoxXSpin->setSuffix(" x");
    boxLayout->addWidget(m_wbBoxXSpin);
    
    m_wbBoxYSpin = new QSpinBox;
    m_wbBoxYSpin->setRange(0, 9999);
    m_wbBoxYSpin->setSuffix(" y");
    boxLayout->addWidget(m_wbBoxYSpin);
    
    m_wbBoxWSpin = new QSpinBox;
    m_wbBoxWSpin->setRange(0, 9999);
    m_wbBoxWSpin->setSuffix(" w");
    boxLayout->addWidget(m_wbBoxWSpin);
    
    m_wbBoxHSpin = new QSpinBox;
    m_wbBoxHSpin->setRange(0, 9999);
    m_wbBoxHSpin->setSuffix(" h");
    boxLayout->addWidget(m_wbBoxHSpin);
    
    QWidget *boxWidget = new QWidget;
    boxWidget->setLayout(boxLayout);
    layout->addWidget(boxWidget, 2, 1, 1, 2);
    
    // Custom WB multipliers
    layout->addWidget(new QLabel("Custom WB:"), 3, 0);
    QHBoxLayout *wbLayout = new QHBoxLayout;
    
    m_customWbR = new QDoubleSpinBox;
    m_customWbR->setRange(0.1, 10.0);
    m_customWbR->setValue(1.0);
    m_customWbR->setDecimals(3);
    m_customWbR->setSuffix(" R");
    wbLayout->addWidget(m_customWbR);
    
    m_customWbG1 = new QDoubleSpinBox;
    m_customWbG1->setRange(0.1, 10.0);
    m_customWbG1->setValue(1.0);
    m_customWbG1->setDecimals(3);
    m_customWbG1->setSuffix(" G1");
    wbLayout->addWidget(m_customWbG1);
    
    m_customWbB = new QDoubleSpinBox;
    m_customWbB->setRange(0.1, 10.0);
    m_customWbB->setValue(1.0);
    m_customWbB->setDecimals(3);
    m_customWbB->setSuffix(" B");
    wbLayout->addWidget(m_customWbB);
    
    m_customWbG2 = new QDoubleSpinBox;
    m_customWbG2->setRange(0.1, 10.0);
    m_customWbG2->setValue(1.0);
    m_customWbG2->setDecimals(3);
    m_customWbG2->setSuffix(" G2");
    wbLayout->addWidget(m_customWbG2);
    
    QWidget *wbWidget = new QWidget;
    wbWidget->setLayout(wbLayout);
    layout->addWidget(wbWidget, 3, 1, 1, 2);
    
    m_wbGroup->setLayout(layout);
}

void ParameterWidget::setupMatrixGroup()
{
    m_matrixGroup = new QGroupBox("Matrix Calculation");
    QGridLayout *layout = new QGridLayout;
    
    // Matrix method
    layout->addWidget(new QLabel("Method:"), 0, 0);
    m_matMethodCombo = new QComboBox;
    m_matMethodCombo->addItems({"spectral", "metadata", "Adobe", "custom"});
    layout->addWidget(m_matMethodCombo, 0, 1);
    
    // Custom camera make/model
    layout->addWidget(new QLabel("Camera Make:"), 1, 0);
    m_customMakeEdit = new QLineEdit;
    m_customMakeEdit->setPlaceholderText("e.g., Canon, Nikon");
    layout->addWidget(m_customMakeEdit, 1, 1);
    
    layout->addWidget(new QLabel("Camera Model:"), 2, 0);
    m_customModelEdit = new QLineEdit;
    m_customModelEdit->setPlaceholderText("e.g., EOS 5D Mark IV");
    layout->addWidget(m_customModelEdit, 2, 1);
    
    // Custom matrix (will be shown/hidden based on method)
    m_customMatrixWidget = new QWidget;
    QGridLayout *matrixLayout = new QGridLayout;
    m_matrixSpins.resize(9);
    
    for (int i = 0; i < 9; ++i) {
        m_matrixSpins[i] = new QDoubleSpinBox;
        m_matrixSpins[i]->setRange(-10.0, 10.0);
        m_matrixSpins[i]->setValue(i % 4 == 0 ? 1.0 : 0.0); // Identity matrix default
        m_matrixSpins[i]->setDecimals(6);
        m_matrixSpins[i]->setSingleStep(0.1);
        matrixLayout->addWidget(m_matrixSpins[i], i / 3, i % 3);
    }
    
    m_customMatrixWidget->setLayout(matrixLayout);
    layout->addWidget(new QLabel("Custom Matrix:"), 3, 0);
    layout->addWidget(m_customMatrixWidget, 3, 1);
    
    m_matrixGroup->setLayout(layout);
}

void ParameterWidget::setupProcessingGroup()
{
    m_processingGroup = new QGroupBox("Processing Options");
    QGridLayout *layout = new QGridLayout;
    
    // Headroom
    layout->addWidget(new QLabel("Headroom:"), 0, 0);
    m_headroomSpin = new QDoubleSpinBox;
    m_headroomSpin->setRange(1.0, 20.0);
    m_headroomSpin->setValue(6.0);
    m_headroomSpin->setDecimals(1);
    m_headroomSpin->setToolTip("Highlight headroom factor");
    layout->addWidget(m_headroomSpin, 0, 1);
    
    // Scale
    layout->addWidget(new QLabel("Scale:"), 0, 2);
    m_scaleSpin = new QDoubleSpinBox;
    m_scaleSpin->setRange(0.1, 10.0);
    m_scaleSpin->setValue(1.0);
    m_scaleSpin->setDecimals(2);
    m_scaleSpin->setToolTip("Additional scaling factor");
    layout->addWidget(m_scaleSpin, 0, 3);
    
    // Auto brightness
    m_autobrightCheck = new QCheckBox("Auto Brightness");
    m_autobrightCheck->setToolTip("Enable automatic exposure adjustment");
    layout->addWidget(m_autobrightCheck, 1, 0);
    
    // Half size
    m_halfSizeCheck = new QCheckBox("Half Size");
    m_halfSizeCheck->setToolTip("Decode image at half size resolution");
    layout->addWidget(m_halfSizeCheck, 1, 1);
    
    // Demosaic algorithm
    layout->addWidget(new QLabel("Demosaic:"), 2, 0);
    m_demosaicCombo = new QComboBox;
    m_demosaicCombo->addItems({"AHD", "VNG", "PPG", "DCB", "AHD-Mod", "AFD", "VCD", "Mixed", "LMMSE", "AMaZE", "DHT", "AAHD"});
    layout->addWidget(m_demosaicCombo, 2, 1);
    
    // Highlight mode
    layout->addWidget(new QLabel("Highlight Mode:"), 2, 2);
    m_highlightModeCombo = new QComboBox;
    m_highlightModeCombo->addItems({"0 - Clip", "1 - Unclip", "2 - Blend", "3 - Rebuild", "4 - Rebuild", "5 - Rebuild", "6 - Rebuild", "7 - Rebuild", "8 - Rebuild", "9 - Rebuild"});
    layout->addWidget(m_highlightModeCombo, 2, 3);
    
    m_processingGroup->setLayout(layout);
}

void ParameterWidget::setupOutputGroup()
{
    m_outputGroup = new QGroupBox("Output Options");
    QGridLayout *layout = new QGridLayout;
    
    // Output directory
    layout->addWidget(new QLabel("Output Directory:"), 0, 0);
    m_outputDirEdit = new QLineEdit;
    m_outputDirEdit->setPlaceholderText("Leave empty to use input directory");
    layout->addWidget(m_outputDirEdit, 0, 1);
    
    m_browseOutputButton = new QPushButton("Browse...");
    connect(m_browseOutputButton, &QPushButton::clicked, this, &ParameterWidget::chooseOutputDirectory);
    layout->addWidget(m_browseOutputButton, 0, 2);
    
    // Checkboxes
    m_overwriteCheck = new QCheckBox("Overwrite existing files");
    layout->addWidget(m_overwriteCheck, 1, 0);
    
    m_createDirsCheck = new QCheckBox("Create output directories");
    layout->addWidget(m_createDirsCheck, 1, 1);
    
    m_disableCacheCheck = new QCheckBox("Disable cache");
    layout->addWidget(m_disableCacheCheck, 1, 2);
    
    m_outputGroup->setLayout(layout);
}

void ParameterWidget::setupAdvancedGroup()
{
    m_advancedGroup = new QGroupBox("Advanced Options");
    
    // Advanced toggle button
    m_advancedToggleButton = new QPushButton("Show Advanced Options");
    connect(m_advancedToggleButton, &QPushButton::clicked, this, &ParameterWidget::showAdvancedOptions);
    
    // Advanced widget (initially hidden)
    m_advancedWidget = new QWidget;
    QGridLayout *layout = new QGridLayout;
    
    // Black level
    layout->addWidget(new QLabel("Black Level:"), 0, 0);
    m_blackLevelSpin = new QSpinBox;
    m_blackLevelSpin->setRange(-1, 65535);
    m_blackLevelSpin->setValue(-1);
    m_blackLevelSpin->setSpecialValueText("Auto");
    layout->addWidget(m_blackLevelSpin, 0, 1);
    
    // Saturation level
    layout->addWidget(new QLabel("Saturation Level:"), 0, 2);
    m_saturationSpin = new QSpinBox;
    m_saturationSpin->setRange(0, 65535);
    m_saturationSpin->setValue(0);
    m_saturationSpin->setSpecialValueText("Auto");
    layout->addWidget(m_saturationSpin, 0, 3);
    
    // Chromatic aberration
    layout->addWidget(new QLabel("Chromatic Aberration:"), 1, 0);
    QHBoxLayout *chromLayout = new QHBoxLayout;
    
    m_chromAberrationR = new QDoubleSpinBox;
    m_chromAberrationR->setRange(0.5, 2.0);
    m_chromAberrationR->setValue(1.0);
    m_chromAberrationR->setDecimals(3);
    m_chromAberrationR->setSuffix(" R");
    chromLayout->addWidget(m_chromAberrationR);
    
    m_chromAberrationB = new QDoubleSpinBox;
    m_chromAberrationB->setRange(0.5, 2.0);
    m_chromAberrationB->setValue(1.0);
    m_chromAberrationB->setDecimals(3);
    m_chromAberrationB->setSuffix(" B");
    chromLayout->addWidget(m_chromAberrationB);
    
    QWidget *chromWidget = new QWidget;
    chromWidget->setLayout(chromLayout);
    layout->addWidget(chromWidget, 1, 1, 1, 2);
    
    // Debug options
    m_verboseCheck = new QCheckBox("Verbose output");
    layout->addWidget(m_verboseCheck, 2, 0);
    
    m_timingCheck = new QCheckBox("Show timing");
    layout->addWidget(m_timingCheck, 2, 1);
    
    m_advancedWidget->setLayout(layout);
    m_advancedWidget->setVisible(false);
    
    // Group layout
    QVBoxLayout *groupLayout = new QVBoxLayout;
    groupLayout->addWidget(m_advancedToggleButton);
    groupLayout->addWidget(m_advancedWidget);
    m_advancedGroup->setLayout(groupLayout);
}

void ParameterWidget::onWbMethodChanged()
{
    QString method = m_wbMethodCombo->currentText();
    
    // Show/hide controls based on method
    m_illuminantCombo->setVisible(method == "illuminant");
    m_customIlluminantEdit->setVisible(method == "illuminant");
    
    // Enable/disable WB box controls
    bool boxMode = (method == "box");
    m_wbBoxXSpin->setEnabled(boxMode);
    m_wbBoxYSpin->setEnabled(boxMode);
    m_wbBoxWSpin->setEnabled(boxMode);
    m_wbBoxHSpin->setEnabled(boxMode);
    
    // Enable/disable custom WB controls
    bool customMode = (method == "custom");
    m_customWbR->setEnabled(customMode);
    m_customWbG1->setEnabled(customMode);
    m_customWbB->setEnabled(customMode);
    m_customWbG2->setEnabled(customMode);
    
    emit parametersChanged();
}

void ParameterWidget::onMatMethodChanged()
{
    QString method = m_matMethodCombo->currentText();
    
    // Show/hide custom matrix controls
    m_customMatrixWidget->setVisible(method == "custom");
    
    emit parametersChanged();
}

void ParameterWidget::onParameterChanged()
{
    emit parametersChanged();
}

void ParameterWidget::chooseOutputDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Choose Output Directory", m_outputDirEdit->text());
    if (!dir.isEmpty()) {
        m_outputDirEdit->setText(dir);
        emit parametersChanged();
    }
}

void ParameterWidget::showAdvancedOptions()
{
    m_advancedVisible = !m_advancedVisible;
    m_advancedWidget->setVisible(m_advancedVisible);
    m_advancedToggleButton->setText(m_advancedVisible ? "Hide Advanced Options" : "Show Advanced Options");
}

ConversionParameters ParameterWidget::getParameters() const
{
    ConversionParameters params;
    
    // White balance
    params.wbMethod = m_wbMethodCombo->currentText();
    params.illuminant = m_illuminantCombo->currentText();
    if (!m_customIlluminantEdit->text().isEmpty()) {
        params.illuminant = m_customIlluminantEdit->text();
    }
    params.wbBoxOrigin = QPoint(m_wbBoxXSpin->value(), m_wbBoxYSpin->value());
    params.wbBoxSize = QSize(m_wbBoxWSpin->value(), m_wbBoxHSpin->value());
    params.customWb = {m_customWbR->value(), m_customWbG1->value(), m_customWbB->value(), m_customWbG2->value()};
    
    // Matrix
    params.matMethod = m_matMethodCombo->currentText();
    params.customCameraMake = m_customMakeEdit->text();
    params.customCameraModel = m_customModelEdit->text();
    
    if (params.matMethod == "custom") {
        params.customMatrix.resize(9);
        for (int i = 0; i < 9; ++i) {
            params.customMatrix[i] = m_matrixSpins[i]->value();
        }
    }
    
    // Processing
    params.headroom = m_headroomSpin->value();
    params.scale = m_scaleSpin->value();
    params.autobrightEnabled = m_autobrightCheck->isChecked();
    params.halfSize = m_halfSizeCheck->isChecked();
    params.demosaicAlgorithm = m_demosaicCombo->currentText();
    params.highlightMode = m_highlightModeCombo->currentIndex();
    
    // Advanced
    params.blackLevel = m_blackLevelSpin->value();
    params.saturationLevel = m_saturationSpin->value();
    params.chromaticAberration = QPointF(m_chromAberrationR->value(), m_chromAberrationB->value());
    params.verbose = m_verboseCheck->isChecked();
    params.useTiming = m_timingCheck->isChecked();
    
    // Output
    params.outputDir = m_outputDirEdit->text();
    params.overwrite = m_overwriteCheck->isChecked();
    params.createDirs = m_createDirsCheck->isChecked();
    params.useCache = !m_disableCacheCheck->isChecked();
    
    return params;
}

void ParameterWidget::setParameters(const ConversionParameters &params)
{
    // White balance
    m_wbMethodCombo->setCurrentText(params.wbMethod);
    m_illuminantCombo->setCurrentText(params.illuminant);
    m_wbBoxXSpin->setValue(params.wbBoxOrigin.x());
    m_wbBoxYSpin->setValue(params.wbBoxOrigin.y());
    m_wbBoxWSpin->setValue(params.wbBoxSize.width());
    m_wbBoxHSpin->setValue(params.wbBoxSize.height());
    
    if (params.customWb.size() >= 4) {
        m_customWbR->setValue(params.customWb[0]);
        m_customWbG1->setValue(params.customWb[1]);
        m_customWbB->setValue(params.customWb[2]);
        m_customWbG2->setValue(params.customWb[3]);
    }
    
    // Matrix
    m_matMethodCombo->setCurrentText(params.matMethod);
    m_customMakeEdit->setText(params.customCameraMake);
    m_customModelEdit->setText(params.customCameraModel);
    
    if (params.customMatrix.size() == 9) {
        for (int i = 0; i < 9; ++i) {
            m_matrixSpins[i]->setValue(params.customMatrix[i]);
        }
    }
    
    // Processing
    m_headroomSpin->setValue(params.headroom);
    m_scaleSpin->setValue(params.scale);
    m_autobrightCheck->setChecked(params.autobrightEnabled);
    m_halfSizeCheck->setChecked(params.halfSize);
    m_demosaicCombo->setCurrentText(params.demosaicAlgorithm);
    m_highlightModeCombo->setCurrentIndex(params.highlightMode);
    
    // Advanced
    m_blackLevelSpin->setValue(params.blackLevel);
    m_saturationSpin->setValue(params.saturationLevel);
    m_chromAberrationR->setValue(params.chromaticAberration.x());
    m_chromAberrationB->setValue(params.chromaticAberration.y());
    m_verboseCheck->setChecked(params.verbose);
    m_timingCheck->setChecked(params.useTiming);
    
    // Output
    m_outputDirEdit->setText(params.outputDir);
    m_overwriteCheck->setChecked(params.overwrite);
    m_createDirsCheck->setChecked(params.createDirs);
    m_disableCacheCheck->setChecked(!params.useCache);
    
    // Update UI state
    onWbMethodChanged();
    onMatMethodChanged();
}

void ParameterWidget::resetToDefaults()
{
    setParameters(ConversionParameters());
}

void ParameterWidget::setWbBoxFromSelection(const QRect &rect)
{
    if (!rect.isValid()) return;
    m_wbBoxXSpin->setValue(rect.x());
    m_wbBoxYSpin->setValue(rect.y());
    m_wbBoxWSpin->setValue(rect.width());
    m_wbBoxHSpin->setValue(rect.height());
    if (m_wbMethodCombo->currentText() != "box") {
        m_wbMethodCombo->setCurrentText("box");
        onWbMethodChanged();
    }
    emit parametersChanged();
}

void ParameterWidget::setCropBoxFromSelection(const QRect &rect)
{
    if (!rect.isValid()) return;
    if (!m_advancedVisible) {
        showAdvancedOptions();
    }
    m_cropXSpin->setValue(rect.x());
    m_cropYSpin->setValue(rect.y());
    m_cropWSpin->setValue(rect.width());
    m_cropHSpin->setValue(rect.height());
    if (m_cropModeCombo->currentText() == "none") {
        m_cropModeCombo->setCurrentText("soft");
    }
    emit parametersChanged();
}
