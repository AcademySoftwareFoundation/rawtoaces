// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <QMainWindow>
#include <QPointer>

#include <rawtoaces/image_converter.h>

class QListWidget;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QProgressBar;
class QTextEdit;
class QTabWidget;
class QCloseEvent;
class QLabel;
class ConversionThread;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = nullptr );

private slots:
    void onAddFiles();
    void onAddFolder();
    void onRemoveSelected();
    void onClearFiles();
    void onBrowseOutput();
    void onAddSpectralDataFolder();
    void onRemoveSpectralDataSelected();
    void onMoveSpectralDataUp();
    void onMoveSpectralDataDown();
    void onConvert();
    void onCancel();
    void onConversionFileStarted( int index, QString path );
    void onConversionFileFinished( int index, bool ok, QString message );
    void onConversionProgress( int done, int total );
    void onBatchFinished();
    void onAbout();
    void updateMatrixMethodDependentUi();
    void updateWbMethodDependentUi();
    void updateBlackSaturationUi();
    void updateLensMetadataOverrideUi();

protected:
    void closeEvent( QCloseEvent *event ) override;

private:
    rta::util::ImageConverter::Settings buildSettingsFromUi() const;
    void                                appendLog( const QString &line );
    void                                setUiBusy( bool busy );
    void                                loadPreferences();
    void                                savePreferences() const;

    QListWidget  *m_fileList      = nullptr;
    QLineEdit    *m_outputDir     = nullptr;
    QListWidget  *m_dataDirList   = nullptr;
    QPushButton  *m_convertButton = nullptr;
    QPushButton  *m_cancelButton  = nullptr;
    QProgressBar *m_progress      = nullptr;
    QTextEdit    *m_log           = nullptr;

    QTabWidget *m_settingsTabs = nullptr;

    QComboBox      *m_wbMethod     = nullptr;
    QComboBox      *m_matrixMethod = nullptr;
    QLineEdit      *m_illuminant   = nullptr;
    QSpinBox       *m_wbBox[4]{};
    QDoubleSpinBox *m_customWb[4]{};
    QDoubleSpinBox *m_customMat[3][3]{};
    QLineEdit      *m_customCameraMake  = nullptr;
    QLineEdit      *m_customCameraModel = nullptr;
    QDoubleSpinBox *m_headroom          = nullptr;
    QDoubleSpinBox *m_scale             = nullptr;

    QCheckBox      *m_autoBright             = nullptr;
    QDoubleSpinBox *m_adjustMaximum          = nullptr;
    QCheckBox      *m_blackLevelFromMetadata = nullptr;
    QSpinBox       *m_blackLevel             = nullptr;
    QCheckBox      *m_saturationFromMetadata = nullptr;
    QSpinBox       *m_saturationLevel        = nullptr;
    QDoubleSpinBox *m_chromaR                = nullptr;
    QDoubleSpinBox *m_chromaB                = nullptr;
    QCheckBox      *m_halfSize               = nullptr;
    QComboBox      *m_highlightMode          = nullptr;
    QSpinBox       *m_cropBox[4]{};
    QComboBox      *m_cropMode = nullptr;
    QComboBox      *m_flip     = nullptr;
    QDoubleSpinBox *m_denoise  = nullptr;
    QComboBox      *m_demosaic = nullptr;

    QCheckBox *m_overwrite  = nullptr;
    QCheckBox *m_createDirs = nullptr;

    QCheckBox      *m_lensCorrAberration   = nullptr;
    QCheckBox      *m_lensCorrDistortion   = nullptr;
    QCheckBox      *m_lensCorrVignetting   = nullptr;
    QCheckBox      *m_requireLens          = nullptr;
    QCheckBox      *m_lensMetadataOverride = nullptr;
    QLineEdit      *m_lensMake             = nullptr;
    QLineEdit      *m_lensModel            = nullptr;
    QDoubleSpinBox *m_lensAperture         = nullptr;
    QDoubleSpinBox *m_lensFocal            = nullptr;
    QDoubleSpinBox *m_lensFocus            = nullptr;

    QCheckBox *m_useTiming       = nullptr;
    QCheckBox *m_disableCache    = nullptr;
    QCheckBox *m_disableExiftool = nullptr;
    QComboBox *m_verbosity       = nullptr;

    QPointer<ConversionThread> m_worker;

    /// Shown only when matrix method is Custom (matches `ImageConverter` usage).
    QWidget *m_customMatrixWrap  = nullptr;
    QLabel  *m_customMatrixLabel = nullptr;

    QWidget *m_wbIlluminantWrap   = nullptr;
    QLabel  *m_wbIlluminantLabel  = nullptr;
    QWidget *m_wbBoxRegionWrap    = nullptr;
    QLabel  *m_wbBoxRegionLabel   = nullptr;
    QWidget *m_wbCustomGainsWrap  = nullptr;
    QLabel  *m_wbCustomGainsLabel = nullptr;
};
