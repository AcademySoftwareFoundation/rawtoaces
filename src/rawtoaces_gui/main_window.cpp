// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "main_window.h"
#include "conversion_thread.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QFrame>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSettings>
#include <QSizePolicy>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QCloseEvent>

#include <rawtoaces/image_converter.h>

#include <algorithm>
#include <vector>

namespace
{
using S = rta::util::ImageConverter::Settings;

/// `QFormLayout` often leaves a bare `QCheckBox` top-aligned in a tall label row
/// (style-dependent). Hosting it in an expanding row with `AlignVCenter`
/// matches the label column’s vertical center without per-platform pixel tweaks.
QWidget *wrapCheckBoxForFormRow( QCheckBox *checkBox )
{
    if ( checkBox == nullptr )
    {
        return nullptr;
    }
    auto *host = new QWidget;
    auto *lay  = new QHBoxLayout( host );
    lay->setContentsMargins( 0, 0, 0, 0 );
    lay->setSpacing( 0 );
    lay->setAlignment( Qt::AlignVCenter );
    lay->addWidget( checkBox, 0, Qt::AlignVCenter );
    lay->addStretch( 1 );
    host->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    return host;
}

/// Fixed width for numeric fields so similar controls align across the window.
constexpr int kStdNumericFieldWidth = 112;

/// Inset from the scroll viewport edges on settings tabs (Raw, Colour, Lens,
/// Output). Vertical gaps between sections do **not** use `QVBoxLayout::spacing`
/// because each `QGroupBox` carries style-dependent chrome; instead each
/// **logical** block (including a vertical stack of group boxes) sits in
/// `wrapSettingsSectionTail` with a fixed bottom margin (`kSettingsSectionTailGap`).
constexpr int kSettingsTabPageMargin  = 6;
constexpr int kSettingsSectionTailGap = 6;

/// Vertical gap between collapsible blocks on Basic / Advanced tabs.
constexpr int kCollapsibleTabSectionSpacing = 12;

/// Inset around the main tab widget and the bottom Convert / progress row.
constexpr int kCentralChromeMargin = 10;

void applySettingsTabPageMarginsOnly( QVBoxLayout *outerColumn )
{
    if ( outerColumn == nullptr )
    {
        return;
    }
    outerColumn->setContentsMargins(
        kSettingsTabPageMargin,
        kSettingsTabPageMargin,
        kSettingsTabPageMargin,
        kSettingsTabPageMargin );
}

void applySettingsTabPageChrome( QVBoxLayout *outerColumn )
{
    applySettingsTabPageMarginsOnly( outerColumn );
    outerColumn->setSpacing( 0 );
}

/// Uniform space **below** a settings “section” (one full-width group or a
/// stacked block of groups). Style engine margins on `QGroupBox` differ;
/// this wrapper is the single place that defines rhythm.
/// `contentStretchInTail` > 0 lets the content fill extra height inside the
/// wrapper (e.g. Input files on the Inputs tab).
QWidget *wrapSettingsSectionTail( QWidget *content, int contentStretchInTail = 0 )
{
    if ( content == nullptr )
    {
        return nullptr;
    }
    auto *wrap = new QWidget;
    auto *lay  = new QVBoxLayout( wrap );
    lay->setContentsMargins( 0, 0, 0, kSettingsSectionTailGap );
    lay->setSpacing( 0 );
    lay->addWidget( content, contentStretchInTail );
    return wrap;
}

QWidget *wrapScroll( QWidget *inner )
{
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable( true );
    scroll->setFrameShape( QFrame::NoFrame );
    scroll->setWidget( inner );
    return scroll;
}

/// Qt Widgets has no collapsible `QGroupBox`; a checkable `QToolButton` header
/// with a disclosure arrow is the usual pattern. `body` holds the form fields
/// (no inner `QGroupBox` frame — the header is the section chrome).
QWidget *wrapCollapsibleSection(
    QWidget *body, const QString &title, bool expandedByDefault = true )
{
    if ( body == nullptr )
    {
        return nullptr;
    }

    auto *section = new QWidget;
    auto *vlay    = new QVBoxLayout( section );
    vlay->setContentsMargins( 0, 0, 0, 0 );
    vlay->setSpacing( 4 );

    auto *toggle = new QToolButton;
    toggle->setText( title );
    toggle->setCheckable( true );
    toggle->setChecked( expandedByDefault );
    toggle->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    toggle->setArrowType(
        expandedByDefault ? Qt::DownArrow : Qt::RightArrow );
    body->setVisible( expandedByDefault );
    toggle->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    toggle->setAutoRaise( true );
    toggle->setFocusPolicy( Qt::StrongFocus );
    {
        QFont headerFont = toggle->font();
        headerFont.setBold( true );
        toggle->setFont( headerFont );
    }
    toggle->setAccessibleName( title );

    QObject::connect(
        toggle,
        &QToolButton::toggled,
        section,
        [toggle, body]( bool expanded ) {
            body->setVisible( expanded );
            toggle->setArrowType( expanded ? Qt::DownArrow : Qt::RightArrow );
        } );

    vlay->addWidget( toggle );
    vlay->addWidget( body );
    return section;
}

/// Fixed pixel width for compact numeric fields. Uses setFixedWidth (not only
/// setMaximumWidth) so e.g. QDoubleSpinBox widgets match despite different decimals.
void setFieldMaxWidth( QWidget *widget, int widthPixels )
{
    if ( widget == nullptr || widthPixels <= 0 )
    {
        return;
    }
    widget->setFixedWidth( widthPixels );
    widget->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

/// macOS (`QMacStyle`) and Fusion differ on default `QFormLayout` label alignment.
/// Use a single policy: leading form, **right-aligned** labels, **vertically
/// centered** next to fields. Standalone checkboxes use `wrapCheckBoxForFormRow`.
void polishFormLayout( QFormLayout *form )
{
    if ( form == nullptr )
    {
        return;
    }
    form->setFormAlignment( Qt::AlignLeading | Qt::AlignTop );
    form->setLabelAlignment( Qt::AlignRight | Qt::AlignVCenter );
}

/// Rows whose field is a nested `QFormLayout` need the *outer* label top-aligned
/// so it lines up with the first inner row (baseline / default center looks wrong).
void alignFormLabelTopForField( QFormLayout *form, QWidget *field )
{
    if ( form == nullptr || field == nullptr )
    {
        return;
    }
    for ( int r = 0; r < form->rowCount(); ++r )
    {
        QLayoutItem *const fieldItem =
            form->itemAt( r, QFormLayout::FieldRole );
        if ( fieldItem == nullptr || fieldItem->widget() != field )
        {
            continue;
        }
        QLayoutItem *const labelItem =
            form->itemAt( r, QFormLayout::LabelRole );
        if ( labelItem != nullptr )
        {
            labelItem->setAlignment( Qt::AlignRight | Qt::AlignTop );
        }
        break;
    }
}

/// Pair with `alignFormLabelTopForField` for tall / stacked fields so the field
/// column stays top-aligned with the outer label.
void alignFormFieldTopForField( QFormLayout *form, QWidget *field )
{
    if ( form == nullptr || field == nullptr )
    {
        return;
    }
    for ( int r = 0; r < form->rowCount(); ++r )
    {
        QLayoutItem *const fieldItem =
            form->itemAt( r, QFormLayout::FieldRole );
        if ( fieldItem == nullptr || fieldItem->widget() != field )
        {
            continue;
        }
        fieldItem->setAlignment( Qt::AlignLeft | Qt::AlignTop );
        break;
    }
}

/// macOS often stretches a `QFormLayout` that is the direct layout of a wide
/// Form in a plain host with narrow field column (Maximum width), anchored
/// top-leading — same layout pattern as former `QGroupBox` forms without frame.
void mountFormInWidget( QWidget *host, QFormLayout **outForm )
{
    if ( host == nullptr || outForm == nullptr )
    {
        return;
    }
    auto *outer = new QVBoxLayout( host );
    outer->setContentsMargins( 0, 0, 0, 0 );
    auto *inner = new QWidget;
    *outForm    = new QFormLayout( inner );
    polishFormLayout( *outForm );
    inner->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
    outer->addWidget( inner, 0, Qt::AlignLeft | Qt::AlignTop );
}

/// Full-width form host (paths / wide rows); same layout as a full-width
/// `QGroupBox` form without the group frame.
void mountFormInWidgetFullWidth( QWidget *host, QFormLayout **outForm )
{
    if ( host == nullptr || outForm == nullptr )
    {
        return;
    }
    auto *outer = new QVBoxLayout( host );
    outer->setContentsMargins( 0, 0, 0, 0 );
    auto *inner = new QWidget;
    *outForm    = new QFormLayout( inner );
    polishFormLayout( *outForm );
    inner->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    outer->addWidget( inner );
}

void addLabeledSpinRows(
    QFormLayout       *form,
    QSpinBox          *boxes[4],
    const QStringList &rowLabels,
    int                maxFieldWidth )
{
    for ( int i = 0; i < 4; ++i )
    {
        if ( maxFieldWidth > 0 )
        {
            setFieldMaxWidth( boxes[i], maxFieldWidth );
        }
        form->addRow( rowLabels.at( i ), boxes[i] );
    }
}

QStringList
flattenBatches( const std::vector<std::vector<std::string>> &batches )
{
    QStringList out;
    for ( const auto &batch: batches )
    {
        for ( const auto &p: batch )
        {
            out.push_back( QString::fromStdString( p ) );
        }
    }
    return out;
}

/// Override paths from the spectral data line edit (`;` / `:` separated).
/// Empty field → empty vector so the library fills from env / defaults (same as
/// not overriding spectral data paths in the converter).
std::vector<std::string> spectralDatabaseDirsFromLineEdit( const QString &text )
{
    const QString trimmed = text.trimmed();
    if ( trimmed.isEmpty() )
    {
        return {};
    }
    std::vector<std::string> out;
    const QStringList        parts = trimmed.split(
        QRegularExpression( QStringLiteral( "[;:]" ) ), Qt::SkipEmptyParts );
    for ( const QString &part: parts )
    {
        const QString one = part.trimmed();
        if ( one.isEmpty() )
        {
            continue;
        }
        out.push_back( one.toStdString() );
    }
    return out;
}

} // namespace

MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent )
{
    setObjectName( QStringLiteral( "rawtoacesMainWindow" ) );
    setWindowTitle( QApplication::applicationDisplayName() );

    auto *inputGroup = new QGroupBox( tr( "Input files" ) );
    auto *inputLay   = new QVBoxLayout( inputGroup );
    inputLay->setSpacing( 6 );
    inputGroup->setSizePolicy(
        QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

    auto *fileRow = new QHBoxLayout;
    m_fileList    = new QListWidget;
    m_fileList->setObjectName( QStringLiteral( "guiFileList" ) );
    m_fileList->setSelectionMode( QAbstractItemView::ExtendedSelection );
    m_fileList->setMinimumHeight( 48 );
    m_fileList->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    fileRow->addWidget( m_fileList, 1 );
    auto *fileBtnCol = new QVBoxLayout;
    fileBtnCol->setSpacing( 6 );
    auto *addFiles  = new QPushButton( tr( "Add files…" ) );
    auto *addFolder = new QPushButton( tr( "Add folder…" ) );
    auto *removeBtn = new QPushButton( tr( "Remove" ) );
    auto *clearBtn  = new QPushButton( tr( "Clear" ) );
    fileBtnCol->addWidget( addFiles );
    fileBtnCol->addWidget( addFolder );
    fileBtnCol->addWidget( removeBtn );
    fileBtnCol->addWidget( clearBtn );
    fileBtnCol->addStretch();
    fileRow->addLayout( fileBtnCol );
    inputLay->addLayout( fileRow, 1 );

    connect( addFiles, &QPushButton::clicked, this, &MainWindow::onAddFiles );
    connect( addFolder, &QPushButton::clicked, this, &MainWindow::onAddFolder );
    connect(
        removeBtn, &QPushButton::clicked, this, &MainWindow::onRemoveSelected );
    connect( clearBtn, &QPushButton::clicked, this, &MainWindow::onClearFiles );

    auto        *pathGroup = new QWidget;
    QFormLayout *pathForm  = nullptr;
    mountFormInWidgetFullWidth( pathGroup, &pathForm );
    pathForm->setHorizontalSpacing( 12 );
    pathForm->setVerticalSpacing( 8 );
    pathForm->setFieldGrowthPolicy( QFormLayout::ExpandingFieldsGrow );

    m_outputDir = new QLineEdit;
    m_outputDir->setObjectName( QStringLiteral( "guiOutputDir" ) );
    m_outputDir->setPlaceholderText(
        tr( "Empty = same folder as each RAW; or set a subfolder / path" ) );
    m_outputDir->setToolTip( tr(
        "Leave empty to write .exr next to the source file. "
        "If set, output paths are resolved under each input file’s directory." ) );
    auto *outBrowse = new QPushButton( tr( "Browse…" ) );
    auto *outWrap   = new QWidget;
    outWrap->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    auto *outHBox = new QHBoxLayout( outWrap );
    outHBox->setContentsMargins( 0, 0, 0, 0 );
    outHBox->setSpacing( 8 );
    outHBox->addWidget( m_outputDir, 1 );
    outHBox->addWidget( outBrowse );
    pathForm->addRow( tr( "Output directory:" ), outWrap );
    connect(
        outBrowse, &QPushButton::clicked, this, &MainWindow::onBrowseOutput );

    m_overwrite  = new QCheckBox;
    m_createDirs = new QCheckBox;
    m_overwrite->setToolTip(
        tr( "Replace existing output files instead of skipping them." ) );
    m_createDirs->setToolTip(
        tr( "When an output directory is set, create missing parent folders "
            "if they do not exist." ) );
    pathForm->addRow(
        tr( "Overwrite existing files:" ),
        wrapCheckBoxForFormRow( m_overwrite ) );
    pathForm->addRow(
        tr( "Create missing directories:" ),
        wrapCheckBoxForFormRow( m_createDirs ) );

    m_log = new QTextEdit;
    m_log->setObjectName( QStringLiteral( "guiLog" ) );
    m_log->setReadOnly( true );
    m_log->setPlaceholderText(
        tr( "Conversion progress and messages will appear here." ) );
    m_log->setMinimumHeight( 72 );

    m_useTiming       = new QCheckBox;
    m_disableCache    = new QCheckBox;
    m_disableExiftool = new QCheckBox;
    m_verbosity       = new QComboBox;
    m_verbosity->setObjectName( QStringLiteral( "guiVerbosity" ) );
    m_verbosity->addItem( tr( "Quiet" ), 0 );
    m_verbosity->addItem( tr( "Progress" ), 1 );
    m_verbosity->addItem( tr( "Detailed" ), 2 );
    m_verbosity->addItem( tr( "Solver report" ), 3 );
    m_verbosity->addItem( tr( "Solver trace" ), 4 );
    m_verbosity->setCurrentIndex( 0 );
    m_verbosity->setMaximumWidth( kStdNumericFieldWidth * 2 );
    m_verbosity->setToolTip( tr(
        "How much is printed to the log and terminal. "
        "Progress: per-step messages. Detailed: adds configuration summary. "
        "Solver report: Ceres summary and IDT matrix. "
        "Solver trace: also Ceres minimizer progress." ) );

    auto        *logsOptionsGroup = new QWidget;
    QFormLayout *logsOptionsForm  = nullptr;
    mountFormInWidget( logsOptionsGroup, &logsOptionsForm );
    logsOptionsForm->setHorizontalSpacing( 12 );
    logsOptionsForm->setVerticalSpacing( 8 );
    logsOptionsForm->setFieldGrowthPolicy( QFormLayout::ExpandingFieldsGrow );
    logsOptionsForm->addRow( tr( "Verbosity:" ), m_verbosity );
    logsOptionsForm->addRow(
        tr( "Log timing:" ), wrapCheckBoxForFormRow( m_useTiming ) );

    auto        *logCacheGroup = new QWidget;
    QFormLayout *cacheDiagLay  = nullptr;
    mountFormInWidget( logCacheGroup, &cacheDiagLay );
    cacheDiagLay->setHorizontalSpacing( 12 );
    cacheDiagLay->setVerticalSpacing( 8 );
    cacheDiagLay->setFieldGrowthPolicy( QFormLayout::ExpandingFieldsGrow );
    cacheDiagLay->addRow(
        tr( "Disable cache:" ), wrapCheckBoxForFormRow( m_disableCache ) );
    cacheDiagLay->addRow(
        tr( "Disable exiftool:" ),
        wrapCheckBoxForFormRow( m_disableExiftool ) );

    m_settingsTabs = new QTabWidget;
    m_settingsTabs->setObjectName( QStringLiteral( "guiSettingsTabs" ) );
    m_settingsTabs->setDocumentMode( true );

    // --- Raw decode groups (used on Basic / Advanced tabs) ---

    auto        *rawLevels = new QWidget;
    QFormLayout *rawLay    = nullptr;
    mountFormInWidget( rawLevels, &rawLay );
    rawLay->setHorizontalSpacing( 12 );
    rawLay->setVerticalSpacing( 8 );
    // `FieldsStayAtSizeHint` keeps checkbox hosts at a tiny preferred width and
    // clips macOS indicators; expanding policy still respects fixed-width spins.
    rawLay->setFieldGrowthPolicy( QFormLayout::ExpandingFieldsGrow );
    m_autoBright = new QCheckBox;
    m_autoBright->setToolTip( tr(
        "LibRaw automatic brightening; other level options still apply." ) );
    m_adjustMaximum = new QDoubleSpinBox;
    m_adjustMaximum->setRange( 0.0, 1.0 );
    m_adjustMaximum->setDecimals( 4 );
    m_adjustMaximum->setValue( 0.75 );
    setFieldMaxWidth( m_adjustMaximum, kStdNumericFieldWidth );

    m_blackLevelFromMetadata = new QCheckBox( tr( "Take from metadata" ) );
    m_blackLevelFromMetadata->setChecked( true );
    m_blackLevelFromMetadata->setToolTip(
        tr( "Use the black level from RAW metadata; when off, use the value "
            "below." ) );
    m_blackLevel = new QSpinBox;
    m_blackLevel->setRange( 0, 2147483647 );
    m_blackLevel->setValue( 0 );
    m_blackLevel->setEnabled( false );
    m_blackLevel->setToolTip(
        tr( "Override sensor black level when metadata is not used." ) );
    setFieldMaxWidth( m_blackLevel, kStdNumericFieldWidth );

    m_saturationFromMetadata = new QCheckBox( tr( "Take from metadata" ) );
    m_saturationFromMetadata->setChecked( true );
    m_saturationFromMetadata->setToolTip( tr(
        "Use the saturation (clip / white) level from RAW metadata; when off, "
        "use the value below." ) );
    m_saturationLevel = new QSpinBox;
    m_saturationLevel->setRange( 1, 2147483647 );
    m_saturationLevel->setValue( 16383 );
    m_saturationLevel->setEnabled( false );
    m_saturationLevel->setToolTip(
        tr( "Raw value treated as saturated when not using metadata." ) );
    setFieldMaxWidth( m_saturationLevel, kStdNumericFieldWidth );

    auto *blackLevelBlock = new QWidget;
    auto *blackLevelVBox  = new QVBoxLayout( blackLevelBlock );
    blackLevelVBox->setContentsMargins( 0, 0, 0, 0 );
    blackLevelVBox->setSpacing( 8 );
    blackLevelVBox->addWidget( m_blackLevelFromMetadata );
    blackLevelVBox->addWidget( m_blackLevel );

    auto *saturationBlock = new QWidget;
    auto *saturationVBox  = new QVBoxLayout( saturationBlock );
    saturationVBox->setContentsMargins( 0, 0, 0, 0 );
    saturationVBox->setSpacing( 8 );
    saturationVBox->addWidget( m_saturationFromMetadata );
    saturationVBox->addWidget( m_saturationLevel );

    rawLay->addRow(
        tr( "Auto bright:" ), wrapCheckBoxForFormRow( m_autoBright ) );
    rawLay->addRow( tr( "Adjust maximum threshold:" ), m_adjustMaximum );
    rawLay->addRow( tr( "Black level:" ), blackLevelBlock );
    rawLay->addRow( tr( "Saturation level:" ), saturationBlock );
    alignFormLabelTopForField( rawLay, blackLevelBlock );
    alignFormLabelTopForField( rawLay, saturationBlock );
    connect(
        m_blackLevelFromMetadata,
        &QCheckBox::toggled,
        this,
        &MainWindow::updateBlackSaturationUi );
    connect(
        m_saturationFromMetadata,
        &QCheckBox::toggled,
        this,
        &MainWindow::updateBlackSaturationUi );
    updateBlackSaturationUi();

    auto        *rawChroma = new QWidget;
    QFormLayout *chForm    = nullptr;
    mountFormInWidget( rawChroma, &chForm );
    chForm->setHorizontalSpacing( 12 );
    chForm->setVerticalSpacing( 8 );
    chForm->setFieldGrowthPolicy( QFormLayout::ExpandingFieldsGrow );
    m_chromaR = new QDoubleSpinBox;
    m_chromaB = new QDoubleSpinBox;
    m_chromaR->setRange( 0.0, 1.0e6 );
    m_chromaB->setRange( 0.0, 1.0e6 );
    m_chromaR->setDecimals( 4 );
    m_chromaB->setDecimals( 4 );
    m_chromaR->setValue( 1.0 );
    m_chromaB->setValue( 1.0 );
    m_halfSize      = new QCheckBox;
    m_highlightMode = new QComboBox;
    m_highlightMode->setObjectName( QStringLiteral( "guiHighlightMode" ) );
    m_highlightMode->addItem( tr( "0 — Clip" ), 0 );
    m_highlightMode->addItem( tr( "1 — Unclip" ), 1 );
    m_highlightMode->addItem( tr( "2 — Blend" ), 2 );
    for ( int h = 3; h <= 9; ++h )
    {
        m_highlightMode->addItem(
            tr( "%1 — Rebuild (level %2)" ).arg( h ).arg( h ), h );
    }
    m_highlightMode->setMaximumWidth( kStdNumericFieldWidth * 3 );
    m_highlightMode->setToolTip(
        tr( "LibRaw highlight recovery: 0 clip, 1 unclip, 2 blend; 3–9 are "
            "rebuild levels with increasing strength." ) );
    setFieldMaxWidth( m_chromaR, kStdNumericFieldWidth );
    setFieldMaxWidth( m_chromaB, kStdNumericFieldWidth );
    chForm->addRow( tr( "Red channel multiplier:" ), m_chromaR );
    chForm->addRow( tr( "Blue channel multiplier:" ), m_chromaB );
    chForm->addRow(
        tr( "Half-size decode:" ), wrapCheckBoxForFormRow( m_halfSize ) );
    chForm->addRow( tr( "Highlight mode:" ), m_highlightMode );

    auto        *rawCrop = new QWidget;
    QFormLayout *crForm  = nullptr;
    mountFormInWidget( rawCrop, &crForm );
    crForm->setHorizontalSpacing( 12 );
    crForm->setVerticalSpacing( 8 );
    crForm->setFieldGrowthPolicy( QFormLayout::FieldsStayAtSizeHint );
    for ( int i = 0; i < 4; ++i )
    {
        m_cropBox[i] = new QSpinBox;
        m_cropBox[i]->setRange( -1000000000, 1000000000 );
    }
    auto *cropRegionInner = new QWidget;
    auto *cropRegionForm  = new QFormLayout( cropRegionInner );
    polishFormLayout( cropRegionForm );
    cropRegionForm->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
    cropRegionForm->setContentsMargins( 0, 0, 0, 0 );
    cropRegionForm->setHorizontalSpacing( 12 );
    cropRegionForm->setFieldGrowthPolicy( QFormLayout::FieldsStayAtSizeHint );
    addLabeledSpinRows(
        cropRegionForm,
        m_cropBox,
        { tr( "X:" ), tr( "Y:" ), tr( "Width:" ), tr( "Height:" ) },
        kStdNumericFieldWidth );
    m_cropMode = new QComboBox;
    m_cropMode->addItems( { tr( "off" ), tr( "soft" ), tr( "hard" ) } );
    m_cropMode->setCurrentIndex( 1 );
    m_cropMode->setMaximumWidth( kStdNumericFieldWidth * 2 );
    m_flip = new QComboBox;
    m_flip->setObjectName( QStringLiteral( "guiFlip" ) );
    m_flip->addItem( tr( "0 — No override (use file metadata)" ), 0 );
    m_flip->addItem( tr( "1 — Normal (0°)" ), 1 );
    m_flip->addItem( tr( "2 — Mirror horizontal" ), 2 );
    m_flip->addItem( tr( "3 — Rotate 180°" ), 3 );
    m_flip->addItem( tr( "4 — Mirror vertical" ), 4 );
    m_flip->addItem( tr( "5 — Mirror horizontal, rotate 270° CW" ), 5 );
    m_flip->addItem( tr( "6 — Rotate 90° CCW" ), 6 );
    m_flip->addItem( tr( "7 — Mirror horizontal, rotate 90° CW" ), 7 );
    m_flip->addItem( tr( "8 — Rotate 90° CW" ), 8 );
    m_flip->setMaximumWidth( kStdNumericFieldWidth * 4 );
    m_flip->setToolTip(
        tr( "Override orientation. 0 uses metadata; 1–8 are EXIF orientation "
            "codes (e.g. 3 = 180°, 6 = 90° CCW, 8 = 90° CW)." ) );
    m_denoise = new QDoubleSpinBox;
    m_denoise->setRange( 0.0, 1.0e9 );
    m_denoise->setDecimals( 4 );
    m_denoise->setValue( 0.0 );
    crForm->addRow( tr( "Crop region (pixels):" ), cropRegionInner );
    alignFormLabelTopForField( crForm, cropRegionInner );
    crForm->addRow( tr( "Crop mode:" ), m_cropMode );
    crForm->addRow( tr( "Flip:" ), m_flip );
    crForm->addRow( tr( "Denoise threshold:" ), m_denoise );
    setFieldMaxWidth( m_denoise, kStdNumericFieldWidth );

    auto        *rawDemo = new QWidget;
    QFormLayout *dmForm  = nullptr;
    mountFormInWidget( rawDemo, &dmForm );
    dmForm->setHorizontalSpacing( 12 );
    dmForm->setFieldGrowthPolicy( QFormLayout::FieldsStayAtSizeHint );
    m_demosaic                      = new QComboBox;
    const QStringList demosaicNames = {
        QStringLiteral( "linear" ), QStringLiteral( "VNG" ),
        QStringLiteral( "PPG" ),    QStringLiteral( "AHD" ),
        QStringLiteral( "DCB" ),    QStringLiteral( "AHD-Mod" ),
        QStringLiteral( "AFD" ),    QStringLiteral( "VCD" ),
        QStringLiteral( "Mixed" ),  QStringLiteral( "LMMSE" ),
        QStringLiteral( "AMaZE" ),  QStringLiteral( "DHT" ),
        QStringLiteral( "AAHD" ),   QStringLiteral( "AHD" )
    };
    m_demosaic->addItems( demosaicNames );
    m_demosaic->setMaximumWidth( kStdNumericFieldWidth * 2 );
    dmForm->addRow( tr( "Algorithm:" ), m_demosaic );

    // --- Colour groups (used on Basic / Advanced tabs) ---
    auto        *grpSpectral  = new QWidget;
    QFormLayout *spectralForm = nullptr;
    mountFormInWidgetFullWidth( grpSpectral, &spectralForm );
    spectralForm->setHorizontalSpacing( 12 );
    spectralForm->setVerticalSpacing( 8 );
    spectralForm->setFieldGrowthPolicy( QFormLayout::ExpandingFieldsGrow );
    m_dataDir = new QLineEdit;
    m_dataDir->setObjectName( QStringLiteral( "guiDataDir" ) );
    m_dataDir->setPlaceholderText( tr( "Empty = default search paths" ) );
    m_dataDir->setToolTip( tr(
        "Override directories for camera / illuminant spectral data. "
        "Separate multiple paths with ';' or ':'. Empty uses library defaults "
        "and environment." ) );
    auto *dataBrowse = new QPushButton( tr( "Browse…" ) );
    auto *dataWrap   = new QWidget;
    dataWrap->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    auto *dataHBox = new QHBoxLayout( dataWrap );
    dataHBox->setContentsMargins( 0, 0, 0, 0 );
    dataHBox->setSpacing( 8 );
    dataHBox->addWidget( m_dataDir, 1 );
    dataHBox->addWidget( dataBrowse );
    spectralForm->addRow( tr( "Data directory:" ), dataWrap );
    connect(
        dataBrowse, &QPushButton::clicked, this, &MainWindow::onBrowseDataDir );

    auto        *grpWb  = new QWidget;
    QFormLayout *wbForm = nullptr;
    mountFormInWidget( grpWb, &wbForm );
    wbForm->setHorizontalSpacing( 12 );
    wbForm->setVerticalSpacing( 8 );
    wbForm->setFieldGrowthPolicy( QFormLayout::FieldsStayAtSizeHint );
    m_wbMethod = new QComboBox;
    m_wbMethod->setObjectName( QStringLiteral( "guiWbMethod" ) );
    m_wbMethod->addItems(
        { tr( "metadata" ), tr( "illuminant" ), tr( "box" ), tr( "custom" ) } );
    m_wbMethod->setMaximumWidth( kStdNumericFieldWidth * 2 );
    m_illuminant = new QLineEdit;
    m_illuminant->setPlaceholderText( tr( "e.g. D55, 3200K" ) );
    m_wbIlluminantWrap = new QWidget;
    auto *illumHBox    = new QHBoxLayout( m_wbIlluminantWrap );
    illumHBox->setContentsMargins( 0, 0, 0, 0 );
    illumHBox->addWidget( m_illuminant, 1 );
    for ( int i = 0; i < 4; ++i )
    {
        m_wbBox[i] = new QSpinBox;
        m_wbBox[i]->setRange( -1000000000, 1000000000 );
    }
    m_wbBoxRegionWrap = new QWidget;
    auto *wbBoxForm   = new QFormLayout( m_wbBoxRegionWrap );
    polishFormLayout( wbBoxForm );
    wbBoxForm->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
    wbBoxForm->setContentsMargins( 0, 0, 0, 0 );
    wbBoxForm->setHorizontalSpacing( 12 );
    wbBoxForm->setFieldGrowthPolicy( QFormLayout::FieldsStayAtSizeHint );
    addLabeledSpinRows(
        wbBoxForm,
        m_wbBox,
        { tr( "X:" ), tr( "Y:" ), tr( "Width:" ), tr( "Height:" ) },
        kStdNumericFieldWidth );

    m_wbCustomGainsWrap = new QWidget;
    auto *cwbForm       = new QFormLayout( m_wbCustomGainsWrap );
    polishFormLayout( cwbForm );
    cwbForm->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
    cwbForm->setContentsMargins( 0, 0, 0, 0 );
    cwbForm->setHorizontalSpacing( 12 );
    cwbForm->setFieldGrowthPolicy( QFormLayout::FieldsStayAtSizeHint );
    for ( int i = 0; i < 4; ++i )
    {
        m_customWb[i] = new QDoubleSpinBox;
        m_customWb[i]->setRange( 0.0, 1.0e6 );
        m_customWb[i]->setDecimals( 6 );
        m_customWb[i]->setValue( 1.0 );
        setFieldMaxWidth( m_customWb[i], kStdNumericFieldWidth );
    }
    cwbForm->addRow( tr( "Red:" ), m_customWb[0] );
    cwbForm->addRow( tr( "Green 1:" ), m_customWb[1] );
    cwbForm->addRow( tr( "Green 2:" ), m_customWb[2] );
    cwbForm->addRow( tr( "Blue:" ), m_customWb[3] );

    wbForm->addRow( tr( "Method:" ), m_wbMethod );
    wbForm->addRow( tr( "Illuminant:" ), m_wbIlluminantWrap );
    wbForm->addRow( tr( "Region for 'box' mode:" ), m_wbBoxRegionWrap );
    wbForm->addRow( tr( "Custom gains:" ), m_wbCustomGainsWrap );
    alignFormLabelTopForField( wbForm, m_wbBoxRegionWrap );
    alignFormLabelTopForField( wbForm, m_wbCustomGainsWrap );
    m_wbIlluminantLabel =
        qobject_cast<QLabel *>( wbForm->labelForField( m_wbIlluminantWrap ) );
    m_wbBoxRegionLabel =
        qobject_cast<QLabel *>( wbForm->labelForField( m_wbBoxRegionWrap ) );
    m_wbCustomGainsLabel =
        qobject_cast<QLabel *>( wbForm->labelForField( m_wbCustomGainsWrap ) );
    connect(
        m_wbMethod,
        &QComboBox::currentIndexChanged,
        this,
        &MainWindow::updateWbMethodDependentUi );
    updateWbMethodDependentUi();

    auto        *grpMat  = new QWidget;
    QFormLayout *matForm = nullptr;
    mountFormInWidget( grpMat, &matForm );
    matForm->setHorizontalSpacing( 12 );
    matForm->setVerticalSpacing( 8 );
    matForm->setFieldGrowthPolicy( QFormLayout::FieldsStayAtSizeHint );
    m_matrixMethod = new QComboBox;
    m_matrixMethod->addItems( { tr( "auto" ),
                                tr( "spectral" ),
                                tr( "metadata" ),
                                tr( "Adobe" ),
                                tr( "custom" ) } );
    m_matrixMethod->setMaximumWidth( kStdNumericFieldWidth * 2 );
    matForm->addRow( tr( "Matrix method:" ), m_matrixMethod );

    m_customMatrixWrap = new QWidget;
    auto *matGrid      = new QGridLayout( m_customMatrixWrap );
    matGrid->setContentsMargins( 0, 0, 0, 0 );
    for ( int r = 0; r < 3; ++r )
    {
        for ( int c = 0; c < 3; ++c )
        {
            m_customMat[r][c] = new QDoubleSpinBox;
            m_customMat[r][c]->setRange( -1.0e6, 1.0e6 );
            m_customMat[r][c]->setDecimals( 6 );
            m_customMat[r][c]->setValue( r == c ? 1.0 : 0.0 );
            setFieldMaxWidth( m_customMat[r][c], kStdNumericFieldWidth );
            matGrid->addWidget( m_customMat[r][c], r, c );
        }
    }
    matForm->addRow( tr( "Custom 3×3 matrix:" ), m_customMatrixWrap );
    m_customMatrixLabel =
        qobject_cast<QLabel *>( matForm->labelForField( m_customMatrixWrap ) );
    connect(
        m_matrixMethod,
        &QComboBox::currentIndexChanged,
        this,
        &MainWindow::updateMatrixMethodDependentUi );
    updateMatrixMethodDependentUi();

    m_customCameraMake  = new QLineEdit;
    m_customCameraModel = new QLineEdit;
    auto *makeWrap      = new QWidget;
    auto *makeHBox      = new QHBoxLayout( makeWrap );
    makeHBox->setContentsMargins( 0, 0, 0, 0 );
    makeHBox->addWidget( m_customCameraMake, 1 );
    auto *modelWrap = new QWidget;
    auto *modelHBox = new QHBoxLayout( modelWrap );
    modelHBox->setContentsMargins( 0, 0, 0, 0 );
    modelHBox->addWidget( m_customCameraModel, 1 );
    matForm->addRow( tr( "Override camera make:" ), makeWrap );
    matForm->addRow( tr( "Override camera model:" ), modelWrap );

    auto        *grpTone  = new QWidget;
    QFormLayout *toneForm = nullptr;
    mountFormInWidget( grpTone, &toneForm );
    toneForm->setHorizontalSpacing( 12 );
    toneForm->setFieldGrowthPolicy( QFormLayout::FieldsStayAtSizeHint );
    m_headroom = new QDoubleSpinBox;
    m_headroom->setRange( 0.0, 1.0e6 );
    m_headroom->setDecimals( 3 );
    m_headroom->setValue( 6.0 );
    m_scale = new QDoubleSpinBox;
    m_scale->setRange( 0.0, 1.0e6 );
    m_scale->setDecimals( 6 );
    m_scale->setValue( 1.0 );
    toneForm->addRow( tr( "Headroom:" ), m_headroom );
    toneForm->addRow( tr( "Scale:" ), m_scale );
    setFieldMaxWidth( m_headroom, kStdNumericFieldWidth );
    setFieldMaxWidth( m_scale, kStdNumericFieldWidth );

    QWidget *lensFlagsBox = nullptr;
    QWidget *lensOverride = nullptr;
#ifdef RTA_GUI_HAS_LENSFUN
    lensFlagsBox          = new QWidget;
    QFormLayout *lensLay  = nullptr;
    mountFormInWidget( lensFlagsBox, &lensLay );
    lensLay->setHorizontalSpacing( 12 );
    lensLay->setVerticalSpacing( 8 );
    lensLay->setFieldGrowthPolicy( QFormLayout::ExpandingFieldsGrow );
    m_lensCorrAberration = new QCheckBox( tr( "Chromatic aberration" ) );
    m_lensCorrDistortion = new QCheckBox( tr( "Distortion" ) );
    m_lensCorrVignetting = new QCheckBox( tr( "Vignetting" ) );
    m_requireLens        = new QCheckBox;
    auto *lensCorrStack  = new QWidget;
    auto *lensCorrVBox   = new QVBoxLayout( lensCorrStack );
    lensCorrVBox->setContentsMargins( 0, 0, 0, 0 );
    lensCorrVBox->setSpacing( 4 );
    lensCorrVBox->addWidget( m_lensCorrAberration );
    lensCorrVBox->addWidget( m_lensCorrDistortion );
    lensCorrVBox->addWidget( m_lensCorrVignetting );
    lensLay->addRow( tr( "Correction types:" ), lensCorrStack );
    lensLay->addRow(
        tr( "Fail if correction unavailable:" ),
        wrapCheckBoxForFormRow( m_requireLens ) );
    // Tall field: align label with the first checkbox, not vertical center of stack.
    alignFormLabelTopForField( lensLay, lensCorrStack );
    alignFormFieldTopForField( lensLay, lensCorrStack );

    lensOverride       = new QWidget;
    QFormLayout *ovLay = nullptr;
    mountFormInWidget( lensOverride, &ovLay );
    ovLay->setHorizontalSpacing( 12 );
    ovLay->setVerticalSpacing( 8 );
    ovLay->setFieldGrowthPolicy( QFormLayout::ExpandingFieldsGrow );
    m_lensMetadataOverride = new QCheckBox;
    m_lensMetadataOverride->setObjectName(
        QStringLiteral( "guiLensMetadataOverride" ) );
    m_lensMetadataOverride->setToolTip(
        tr( "When enabled, use the make, model, aperture, focal length, and "
            "focus distance below for lens correction; when off, converter "
            "defaults apply (same as leaving CLI overrides unset)." ) );
    ovLay->addRow(
        tr( "Override:" ), wrapCheckBoxForFormRow( m_lensMetadataOverride ) );
    connect(
        m_lensMetadataOverride,
        &QCheckBox::toggled,
        this,
        &MainWindow::updateLensMetadataOverrideUi );
    m_lensMake     = new QLineEdit;
    m_lensModel    = new QLineEdit;
    m_lensAperture = new QDoubleSpinBox;
    m_lensAperture->setRange( 0.0, 1.0e6 );
    m_lensAperture->setDecimals( 3 );
    m_lensFocal = new QDoubleSpinBox;
    m_lensFocal->setRange( 0.0, 1.0e6 );
    m_lensFocal->setDecimals( 3 );
    m_lensFocus = new QDoubleSpinBox;
    m_lensFocus->setRange( 0.0, 1.0e9 );
    m_lensFocus->setDecimals( 4 );
    auto *lensMakeWrap = new QWidget;
    auto *lensMakeHBox = new QHBoxLayout( lensMakeWrap );
    lensMakeHBox->setContentsMargins( 0, 0, 0, 0 );
    lensMakeHBox->addWidget( m_lensMake, 1 );
    auto *lensModelWrap = new QWidget;
    auto *lensModelHBox = new QHBoxLayout( lensModelWrap );
    lensModelHBox->setContentsMargins( 0, 0, 0, 0 );
    lensModelHBox->addWidget( m_lensModel, 1 );
    ovLay->addRow( tr( "Make:" ), lensMakeWrap );
    ovLay->addRow( tr( "Model:" ), lensModelWrap );
    ovLay->addRow( tr( "Aperture (f-number):" ), m_lensAperture );
    ovLay->addRow( tr( "Focal length (mm):" ), m_lensFocal );
    ovLay->addRow( tr( "Focus distance:" ), m_lensFocus );
    setFieldMaxWidth( m_lensAperture, kStdNumericFieldWidth );
    setFieldMaxWidth( m_lensFocal, kStdNumericFieldWidth );
    setFieldMaxWidth( m_lensFocus, kStdNumericFieldWidth );
    updateLensMetadataOverrideUi();

#else
    m_lensCorrAberration   = nullptr;
    m_lensCorrDistortion   = nullptr;
    m_lensCorrVignetting   = nullptr;
    m_requireLens          = nullptr;
    m_lensMetadataOverride = nullptr;
    m_lensMake             = nullptr;
    m_lensModel            = nullptr;
    m_lensAperture         = nullptr;
    m_lensFocal            = nullptr;
    m_lensFocus            = nullptr;
#endif

    // --- Tab pages: Inputs, Basic, Advanced, Logs ---
    auto *inputsInner = new QWidget;
    auto *inputsOuter = new QVBoxLayout( inputsInner );
    applySettingsTabPageChrome( inputsOuter );
    inputsOuter->setSpacing( kCollapsibleTabSectionSpacing );
    inputsOuter->addWidget( wrapSettingsSectionTail( inputGroup, 1 ), 1 );
    inputsOuter->addWidget(
        wrapCollapsibleSection( pathGroup, tr( "Output settings" ), false ), 0 );

    auto *basicInner = new QWidget;
    auto *basicOuter = new QVBoxLayout( basicInner );
    applySettingsTabPageChrome( basicOuter );
    basicOuter->setSpacing( kCollapsibleTabSectionSpacing );
    basicOuter->addWidget(
        wrapCollapsibleSection( rawLevels, tr( "Levels && exposure" ) ) );
    basicOuter->addWidget( wrapCollapsibleSection(
        rawChroma, tr( "Chromatic aberration && size" ) ) );
    basicOuter->addWidget(
        wrapCollapsibleSection( grpWb, tr( "White balance" ) ) );
    basicOuter->addWidget(
        wrapCollapsibleSection( grpTone, tr( "Tone && scale" ) ) );
    basicOuter->addStretch();

    auto *advancedInner = new QWidget;
    auto *advancedOuter = new QVBoxLayout( advancedInner );
    applySettingsTabPageChrome( advancedOuter );
    advancedOuter->setSpacing( kCollapsibleTabSectionSpacing );
    advancedOuter->addWidget(
        wrapCollapsibleSection( grpSpectral, tr( "Spectral data" ) ) );
    advancedOuter->addWidget( wrapCollapsibleSection(
        grpMat, tr( "Colour matrix && camera" ) ) );
    advancedOuter->addWidget( wrapCollapsibleSection(
        rawCrop, tr( "Crop, orientation && denoise" ) ) );
    advancedOuter->addWidget(
        wrapCollapsibleSection( rawDemo, tr( "Demosaic" ) ) );
#ifdef RTA_GUI_HAS_LENSFUN
    advancedOuter->addWidget( wrapCollapsibleSection(
        lensFlagsBox, tr( "Lens corrections" ) ) );
    advancedOuter->addWidget( wrapCollapsibleSection(
        lensOverride, tr( "Lens metadata" ) ) );
#endif
    advancedOuter->addWidget( wrapCollapsibleSection(
        logCacheGroup, tr( "Cache" ) ) );
    advancedOuter->addStretch();

    auto *logsInner = new QWidget;
    auto *logsOuter = new QVBoxLayout( logsInner );
    applySettingsTabPageChrome( logsOuter );
    logsOuter->setSpacing( kCollapsibleTabSectionSpacing );
    logsOuter->addWidget( wrapCollapsibleSection(
        logsOptionsGroup, tr( "Log output settings" ), false ) );
    logsOuter->addWidget( m_log, 1 );

    m_settingsTabs->addTab( wrapScroll( inputsInner ), tr( "Inputs" ) );
    m_settingsTabs->addTab( wrapScroll( basicInner ), tr( "Basic" ) );
    m_settingsTabs->addTab( wrapScroll( advancedInner ), tr( "Advanced" ) );
    m_settingsTabs->addTab( logsInner, tr( "Logs" ) );

    auto *runRow = new QWidget;
    auto *runLay = new QHBoxLayout( runRow );
    runLay->setContentsMargins( 0, 0, 0, 0 );
    m_convertButton = new QPushButton( tr( "Convert" ) );
    m_convertButton->setObjectName( QStringLiteral( "guiConvertButton" ) );
    m_convertButton->setDefault( true );
    m_convertButton->setAutoDefault( true );
    m_cancelButton = new QPushButton( tr( "Cancel" ) );
    m_cancelButton->setObjectName( QStringLiteral( "guiCancelButton" ) );
    m_cancelButton->setAutoDefault( false );
    m_cancelButton->setEnabled( false );
    m_progress = new QProgressBar;
    m_progress->setObjectName( QStringLiteral( "guiProgressBar" ) );
    m_progress->setRange( 0, 1 );
    m_progress->setValue( 0 );
    m_progress->setTextVisible( true );
    m_progress->setMaximumHeight( 24 );
    m_progress->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    runLay->addWidget( m_convertButton );
    runLay->addWidget( m_cancelButton );
    runLay->addWidget( m_progress, 1 );
    connect(
        m_convertButton, &QPushButton::clicked, this, &MainWindow::onConvert );
    connect(
        m_cancelButton, &QPushButton::clicked, this, &MainWindow::onCancel );

    auto *bottom    = new QWidget;
    auto *bottomLay = new QVBoxLayout( bottom );
    bottomLay->setContentsMargins( 0, 0, 0, 0 );
    bottomLay->setSpacing( 0 );
    bottomLay->addWidget( runRow );
    bottom->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum );

    auto *central    = new QWidget;
    auto *centralLay = new QVBoxLayout( central );
    centralLay->setContentsMargins(
        kCentralChromeMargin,
        0,
        kCentralChromeMargin,
        kCentralChromeMargin );
    centralLay->setSpacing( kCentralChromeMargin );
    centralLay->addWidget( m_settingsTabs, 1 );
    centralLay->addWidget( bottom, 0 );
    setCentralWidget( central );

    alignFormFieldTopForField( rawLay, blackLevelBlock );
    alignFormFieldTopForField( rawLay, saturationBlock );

    auto *fileMenu = menuBar()->addMenu( tr( "File" ) );
    fileMenu->addAction( tr( "Add files…" ), this, &MainWindow::onAddFiles );
    fileMenu->addAction( tr( "Add folder…" ), this, &MainWindow::onAddFolder );
    fileMenu->addSeparator();
    fileMenu->addAction( tr( "Quit" ), this, &QWidget::close );

    auto *helpMenu = menuBar()->addMenu( tr( "Help" ) );
    helpMenu->addAction( tr( "About" ), this, &MainWindow::onAbout );

    resize( 1100, 820 );

    loadPreferences();
}

rta::util::ImageConverter::Settings MainWindow::buildSettingsFromUi() const
{
    S s;

    switch ( m_wbMethod->currentIndex() )
    {
        case 0: s.WB_method = S::WBMethod::Metadata; break;
        case 1: s.WB_method = S::WBMethod::Illuminant; break;
        case 2: s.WB_method = S::WBMethod::Box; break;
        default: s.WB_method = S::WBMethod::Custom; break;
    }

    switch ( m_matrixMethod->currentIndex() )
    {
        case 0: s.matrix_method = S::MatrixMethod::Auto; break;
        case 1: s.matrix_method = S::MatrixMethod::Spectral; break;
        case 2: s.matrix_method = S::MatrixMethod::Metadata; break;
        case 3: s.matrix_method = S::MatrixMethod::Adobe; break;
        default: s.matrix_method = S::MatrixMethod::Custom; break;
    }

    s.illuminant = m_illuminant->text().toStdString();
    if ( s.WB_method == S::WBMethod::Illuminant && s.illuminant.empty() )
    {
        s.illuminant = "D55";
    }

    for ( int i = 0; i < 4; ++i )
    {
        s.WB_box[i] = m_wbBox[i]->value();
    }
    for ( int i = 0; i < 4; ++i )
    {
        s.custom_WB[i] = static_cast<float>( m_customWb[i]->value() );
    }
    for ( int r = 0; r < 3; ++r )
    {
        for ( int c = 0; c < 3; ++c )
        {
            s.custom_matrix[r][c] =
                static_cast<float>( m_customMat[r][c]->value() );
        }
    }

    s.custom_camera_make  = m_customCameraMake->text().toStdString();
    s.custom_camera_model = m_customCameraModel->text().toStdString();
    s.headroom            = static_cast<float>( m_headroom->value() );
    s.scale               = static_cast<float>( m_scale->value() );

    s.auto_bright              = m_autoBright->isChecked();
    s.adjust_maximum_threshold = static_cast<float>( m_adjustMaximum->value() );
    s.black_level              = ( m_blackLevelFromMetadata != nullptr &&
                      m_blackLevelFromMetadata->isChecked() )
                                     ? -1
                                     : m_blackLevel->value();
    s.saturation_level         = ( m_saturationFromMetadata != nullptr &&
                           m_saturationFromMetadata->isChecked() )
                                     ? 0
                                     : m_saturationLevel->value();
    s.chromatic_aberration[0]  = static_cast<float>( m_chromaR->value() );
    s.chromatic_aberration[1]  = static_cast<float>( m_chromaB->value() );
    s.half_size                = m_halfSize->isChecked();
    s.highlight_mode           = m_highlightMode->currentData().toInt();
    for ( int i = 0; i < 4; ++i )
    {
        s.crop_box[i] = m_cropBox[i]->value();
    }

    switch ( m_cropMode->currentIndex() )
    {
        case 0: s.crop_mode = S::CropMode::Off; break;
        case 1: s.crop_mode = S::CropMode::Soft; break;
        default: s.crop_mode = S::CropMode::Hard; break;
    }

    s.flip               = m_flip->currentData().toInt();
    s.denoise_threshold  = static_cast<float>( m_denoise->value() );
    s.demosaic_algorithm = m_demosaic->currentText().toStdString();

    s.overwrite   = m_overwrite->isChecked();
    s.create_dirs = m_createDirs->isChecked();
    s.output_dir  = m_outputDir->text().toStdString();

    s.database_directories =
        spectralDatabaseDirsFromLineEdit( m_dataDir->text() );

#ifdef RTA_GUI_HAS_LENSFUN
    if ( m_lensCorrAberration != nullptr )
    {
        s.lens_correction_types = S::LensCorrectionType::None;
        if ( m_lensCorrAberration->isChecked() )
        {
            s.lens_correction_types |= S::LensCorrectionType::Aberration;
        }
        if ( m_lensCorrDistortion->isChecked() )
        {
            s.lens_correction_types |= S::LensCorrectionType::Distortion;
        }
        if ( m_lensCorrVignetting->isChecked() )
        {
            s.lens_correction_types |= S::LensCorrectionType::Vignetting;
        }
        s.require_lens_correction = m_requireLens->isChecked();
        if ( m_lensMetadataOverride != nullptr &&
             m_lensMetadataOverride->isChecked() )
        {
            s.custom_lens_make  = m_lensMake->text().toStdString();
            s.custom_lens_model = m_lensModel->text().toStdString();
            s.custom_aperture   = static_cast<float>( m_lensAperture->value() );
            s.custom_focal_length = static_cast<float>( m_lensFocal->value() );
            s.custom_focus_distance =
                static_cast<float>( m_lensFocus->value() );
        }
        else
        {
            s.custom_lens_make.clear();
            s.custom_lens_model.clear();
            s.custom_aperture       = 0.0f;
            s.custom_focal_length   = 0.0f;
            s.custom_focus_distance = 0.0f;
        }
    }
#endif

    s.use_timing       = m_useTiming->isChecked();
    s.disable_cache    = m_disableCache->isChecked();
    s.disable_exiftool = m_disableExiftool->isChecked();
    {
        bool      ok = false;
        const int v  = m_verbosity->currentData().toInt( &ok );
        s.verbosity  = ok ? v : 0;
    }

    return s;
}

void MainWindow::appendLog( const QString &line )
{
    m_log->append( line );
}

void MainWindow::setUiBusy( bool busy )
{
    m_convertButton->setEnabled( !busy );
    m_cancelButton->setEnabled( busy );
}

void MainWindow::updateMatrixMethodDependentUi()
{
    // Combo order matches `buildSettingsFromUi`: …, custom (index 4).
    const bool useCustomMatrix =
        m_matrixMethod != nullptr && m_matrixMethod->currentIndex() == 4;
    if ( m_customMatrixWrap != nullptr )
    {
        m_customMatrixWrap->setVisible( useCustomMatrix );
    }
    if ( m_customMatrixLabel != nullptr )
    {
        m_customMatrixLabel->setVisible( useCustomMatrix );
    }
}

void MainWindow::updateWbMethodDependentUi()
{
    if ( m_wbMethod == nullptr )
    {
        return;
    }
    const int  idx             = m_wbMethod->currentIndex();
    const bool showIlluminant  = ( idx == 1 );
    const bool showBoxRegion   = ( idx == 2 );
    const bool showCustomGains = ( idx == 3 );

    auto showPair = []( QWidget *field, QLabel *lab, bool on ) {
        if ( field != nullptr )
        {
            field->setVisible( on );
        }
        if ( lab != nullptr )
        {
            lab->setVisible( on );
        }
    };

    showPair( m_wbIlluminantWrap, m_wbIlluminantLabel, showIlluminant );
    showPair( m_wbBoxRegionWrap, m_wbBoxRegionLabel, showBoxRegion );
    showPair( m_wbCustomGainsWrap, m_wbCustomGainsLabel, showCustomGains );
}

void MainWindow::updateBlackSaturationUi()
{
    if ( m_blackLevel != nullptr && m_blackLevelFromMetadata != nullptr )
    {
        m_blackLevel->setEnabled( !m_blackLevelFromMetadata->isChecked() );
    }
    if ( m_saturationLevel != nullptr && m_saturationFromMetadata != nullptr )
    {
        m_saturationLevel->setEnabled( !m_saturationFromMetadata->isChecked() );
    }
}

void MainWindow::updateLensMetadataOverrideUi()
{
#ifdef RTA_GUI_HAS_LENSFUN
    if ( m_lensMetadataOverride == nullptr || m_lensMake == nullptr )
    {
        return;
    }
    const bool on = m_lensMetadataOverride->isChecked();
    m_lensMake->setEnabled( on );
    m_lensModel->setEnabled( on );
    m_lensAperture->setEnabled( on );
    m_lensFocal->setEnabled( on );
    m_lensFocus->setEnabled( on );
#endif
}

void MainWindow::onAddFiles()
{
    const QStringList files = QFileDialog::getOpenFileNames( this );
    for ( const QString &f: files )
    {
        m_fileList->addItem( f );
    }
}

void MainWindow::onAddFolder()
{
    const QString dir = QFileDialog::getExistingDirectory( this );
    if ( dir.isEmpty() )
    {
        return;
    }
    const std::vector<std::string> paths = { dir.toStdString() };
    const auto        batches = rta::util::collect_image_files( paths );
    const QStringList flat    = flattenBatches( batches );
    for ( const QString &f: flat )
    {
        m_fileList->addItem( f );
    }
}

void MainWindow::onRemoveSelected()
{
    for ( auto *item: m_fileList->selectedItems() )
    {
        delete m_fileList->takeItem( m_fileList->row( item ) );
    }
}

void MainWindow::onClearFiles()
{
    m_fileList->clear();
}

void MainWindow::onBrowseOutput()
{
    const QString d = QFileDialog::getExistingDirectory( this );
    if ( !d.isEmpty() )
    {
        m_outputDir->setText( d );
    }
}

void MainWindow::onBrowseDataDir()
{
    const QString d = QFileDialog::getExistingDirectory( this );
    if ( !d.isEmpty() )
    {
        m_dataDir->setText( d );
    }
}

void MainWindow::onConvert()
{
    QStringList paths;
    for ( int i = 0; i < m_fileList->count(); ++i )
    {
        paths << m_fileList->item( i )->text();
    }
    if ( paths.isEmpty() )
    {
        QMessageBox::warning(
            this,
            QApplication::applicationDisplayName(),
            tr( "Add at least one input file." ) );
        return;
    }

    const S settings = buildSettingsFromUi();

    m_progress->setRange( 0, paths.size() );
    m_progress->setValue( 0 );
    appendLog( tr( "Starting batch (%1 files)…" ).arg( paths.size() ) );

    auto *thread = new ConversionThread( this );
    m_worker     = thread;
    connect(
        thread,
        &ConversionThread::fileStarted,
        this,
        &MainWindow::onConversionFileStarted );
    connect(
        thread,
        &ConversionThread::fileFinished,
        this,
        &MainWindow::onConversionFileFinished );
    connect(
        thread,
        &ConversionThread::progress,
        this,
        &MainWindow::onConversionProgress );
    connect(
        thread,
        &ConversionThread::batchFinished,
        this,
        &MainWindow::onBatchFinished );
    connect( thread, &QThread::finished, thread, &QObject::deleteLater );

    thread->setJob( settings, paths );
    setUiBusy( true );
    thread->start();
}

void MainWindow::onCancel()
{
    if ( m_worker )
    {
        m_worker->requestCancel();
        appendLog( tr( "Cancel requested after current file…" ) );
    }
}

void MainWindow::onConversionFileStarted( int index, QString path )
{
    appendLog( tr( "[%1] %2" ).arg( index + 1 ).arg( path ) );
}

void MainWindow::onConversionFileFinished( int index, bool ok, QString message )
{
    if ( ok )
    {
        appendLog( tr( "  OK" ) );
    }
    else
    {
        appendLog( tr( "  FAILED: %1" ).arg( message ) );
    }
    Q_UNUSED( index );
}

void MainWindow::onConversionProgress( int done, int total )
{
    m_progress->setMaximum( total );
    m_progress->setValue( done );
}

void MainWindow::onBatchFinished()
{
    appendLog( tr( "Batch finished." ) );
    setUiBusy( false );
    m_worker = nullptr;
}

void MainWindow::onAbout()
{
    QString body = tr(
        "%1 — ACES container output from camera RAW.\n"
        "Settings match the same options as the rawtoaces image converter." )
                       .arg( QApplication::applicationDisplayName() );
#ifdef VERSION
    body.prepend( tr( "Version %1\n\n" ).arg( QStringLiteral( VERSION ) ) );
#endif
    QMessageBox::about(
        this,
        tr( "About %1" ).arg( QApplication::applicationDisplayName() ),
        body );
}

namespace
{
constexpr auto kPrefsRootQLS       = "rawtoaces_gui";
constexpr int  kPrefsFormatVersion = 1;

void setComboBoxIndexClamped( QComboBox *comboBox, int index )
{
    if ( comboBox == nullptr || comboBox->count() <= 0 )
    {
        return;
    }
    comboBox->setCurrentIndex( qBound( 0, index, comboBox->count() - 1 ) );
}

void setComboBoxCurrentByIntData( QComboBox *comboBox, int value )
{
    if ( comboBox == nullptr || comboBox->count() <= 0 )
    {
        return;
    }
    const int found = comboBox->findData( value );
    comboBox->setCurrentIndex( found >= 0 ? found : 0 );
}

void setVerbosityComboFromLevel( QComboBox *comboBox, int level )
{
    if ( comboBox == nullptr )
    {
        return;
    }
    const int clamped = std::clamp( level, 0, 4 );
    const int found   = comboBox->findData( clamped );
    comboBox->setCurrentIndex( found >= 0 ? found : 0 );
}

} // namespace

void MainWindow::closeEvent( QCloseEvent *event )
{
    if ( m_worker != nullptr && m_worker->isRunning() )
    {
        const auto reply = QMessageBox::question(
            this,
            QApplication::applicationDisplayName(),
            tr( "A conversion is still running. Cancel it and close?" ),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No );
        if ( reply != QMessageBox::Yes )
        {
            event->ignore();
            return;
        }
        m_worker->requestCancel();
        constexpr int kStopWaitMs = 300000;
        if ( !m_worker->wait( kStopWaitMs ) )
        {
            QMessageBox::warning(
                this,
                QApplication::applicationDisplayName(),
                tr( "The conversion could not be stopped before the timeout. "
                    "Try again after the current file finishes." ) );
            event->ignore();
            return;
        }
    }
    savePreferences();
    QMainWindow::closeEvent( event );
}

void MainWindow::savePreferences() const
{
    QSettings settings;
    settings.beginGroup( kPrefsRootQLS );
    settings.setValue( QStringLiteral( "formatVersion" ), kPrefsFormatVersion );

    settings.beginGroup( QStringLiteral( "window" ) );
    settings.setValue( QStringLiteral( "geometry" ), saveGeometry() );
    if ( m_settingsTabs != nullptr )
    {
        settings.setValue(
            QStringLiteral( "settingsTab" ), m_settingsTabs->currentIndex() );
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "paths" ) );
    if ( m_outputDir != nullptr )
    {
        settings.setValue( QStringLiteral( "outputDir" ), m_outputDir->text() );
    }
    if ( m_dataDir != nullptr )
    {
        settings.setValue(
            QStringLiteral( "spectralDataOverride" ), m_dataDir->text() );
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "colour" ) );
    if ( m_wbMethod != nullptr )
    {
        settings.setValue(
            QStringLiteral( "wbMethodIndex" ), m_wbMethod->currentIndex() );
    }
    if ( m_illuminant != nullptr )
    {
        settings.setValue(
            QStringLiteral( "illuminant" ), m_illuminant->text() );
    }
    for ( int i = 0; i < 4 && m_wbBox[i] != nullptr; ++i )
    {
        settings.setValue(
            QStringLiteral( "wbBox%1" ).arg( i ), m_wbBox[i]->value() );
    }
    for ( int i = 0; i < 4 && m_customWb[i] != nullptr; ++i )
    {
        settings.setValue(
            QStringLiteral( "customWb%1" ).arg( i ), m_customWb[i]->value() );
    }
    if ( m_matrixMethod != nullptr )
    {
        settings.setValue(
            QStringLiteral( "matrixMethodIndex" ),
            m_matrixMethod->currentIndex() );
    }
    for ( int r = 0; r < 3; ++r )
    {
        for ( int c = 0; c < 3; ++c )
        {
            if ( m_customMat[r][c] != nullptr )
            {
                settings.setValue(
                    QStringLiteral( "customMatrix_%1_%2" ).arg( r ).arg( c ),
                    m_customMat[r][c]->value() );
            }
        }
    }
    if ( m_customCameraMake != nullptr )
    {
        settings.setValue(
            QStringLiteral( "customCameraMake" ), m_customCameraMake->text() );
    }
    if ( m_customCameraModel != nullptr )
    {
        settings.setValue(
            QStringLiteral( "customCameraModel" ),
            m_customCameraModel->text() );
    }
    if ( m_headroom != nullptr )
    {
        settings.setValue( QStringLiteral( "headroom" ), m_headroom->value() );
    }
    if ( m_scale != nullptr )
    {
        settings.setValue( QStringLiteral( "scale" ), m_scale->value() );
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "raw" ) );
    if ( m_autoBright != nullptr )
    {
        settings.setValue(
            QStringLiteral( "autoBright" ), m_autoBright->isChecked() );
    }
    if ( m_adjustMaximum != nullptr )
    {
        settings.setValue(
            QStringLiteral( "adjustMaximum" ), m_adjustMaximum->value() );
    }
    if ( m_blackLevelFromMetadata != nullptr )
    {
        settings.setValue(
            QStringLiteral( "blackLevelFromMetadata" ),
            m_blackLevelFromMetadata->isChecked() );
    }
    if ( m_blackLevel != nullptr )
    {
        settings.setValue(
            QStringLiteral( "blackLevel" ), m_blackLevel->value() );
    }
    if ( m_saturationFromMetadata != nullptr )
    {
        settings.setValue(
            QStringLiteral( "saturationFromMetadata" ),
            m_saturationFromMetadata->isChecked() );
    }
    if ( m_saturationLevel != nullptr )
    {
        settings.setValue(
            QStringLiteral( "saturationLevel" ), m_saturationLevel->value() );
    }
    if ( m_chromaR != nullptr )
    {
        settings.setValue( QStringLiteral( "chromaR" ), m_chromaR->value() );
    }
    if ( m_chromaB != nullptr )
    {
        settings.setValue( QStringLiteral( "chromaB" ), m_chromaB->value() );
    }
    if ( m_halfSize != nullptr )
    {
        settings.setValue(
            QStringLiteral( "halfSize" ), m_halfSize->isChecked() );
    }
    if ( m_highlightMode != nullptr )
    {
        settings.setValue(
            QStringLiteral( "highlightMode" ),
            m_highlightMode->currentData().toInt() );
    }
    for ( int i = 0; i < 4 && m_cropBox[i] != nullptr; ++i )
    {
        settings.setValue(
            QStringLiteral( "cropBox%1" ).arg( i ), m_cropBox[i]->value() );
    }
    if ( m_cropMode != nullptr )
    {
        settings.setValue(
            QStringLiteral( "cropModeIndex" ), m_cropMode->currentIndex() );
    }
    if ( m_flip != nullptr )
    {
        settings.setValue(
            QStringLiteral( "flip" ), m_flip->currentData().toInt() );
    }
    if ( m_denoise != nullptr )
    {
        settings.setValue( QStringLiteral( "denoise" ), m_denoise->value() );
    }
    if ( m_demosaic != nullptr )
    {
        settings.setValue(
            QStringLiteral( "demosaicAlgorithm" ), m_demosaic->currentText() );
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "output" ) );
    if ( m_overwrite != nullptr )
    {
        settings.setValue(
            QStringLiteral( "overwrite" ), m_overwrite->isChecked() );
    }
    if ( m_createDirs != nullptr )
    {
        settings.setValue(
            QStringLiteral( "createDirs" ), m_createDirs->isChecked() );
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "diagnostics" ) );
    if ( m_useTiming != nullptr )
    {
        settings.setValue(
            QStringLiteral( "useTiming" ), m_useTiming->isChecked() );
    }
    if ( m_disableCache != nullptr )
    {
        settings.setValue(
            QStringLiteral( "disableCache" ), m_disableCache->isChecked() );
    }
    if ( m_disableExiftool != nullptr )
    {
        settings.setValue(
            QStringLiteral( "disableExiftool" ),
            m_disableExiftool->isChecked() );
    }
    if ( m_verbosity != nullptr )
    {
        bool      ok = false;
        const int v  = m_verbosity->currentData().toInt( &ok );
        settings.setValue( QStringLiteral( "verbosity" ), ok ? v : 0 );
    }
    settings.endGroup();

#ifdef RTA_GUI_HAS_LENSFUN
    if ( m_lensCorrAberration != nullptr )
    {
        settings.beginGroup( QStringLiteral( "lens" ) );
        settings.setValue(
            QStringLiteral( "corrAberration" ),
            m_lensCorrAberration->isChecked() );
        settings.setValue(
            QStringLiteral( "corrDistortion" ),
            m_lensCorrDistortion->isChecked() );
        settings.setValue(
            QStringLiteral( "corrVignetting" ),
            m_lensCorrVignetting->isChecked() );
        settings.setValue(
            QStringLiteral( "requireLens" ), m_requireLens->isChecked() );
        settings.setValue(
            QStringLiteral( "lensMetadataOverride" ),
            m_lensMetadataOverride->isChecked() );
        settings.setValue( QStringLiteral( "lensMake" ), m_lensMake->text() );
        settings.setValue( QStringLiteral( "lensModel" ), m_lensModel->text() );
        settings.setValue(
            QStringLiteral( "lensAperture" ), m_lensAperture->value() );
        settings.setValue(
            QStringLiteral( "lensFocal" ), m_lensFocal->value() );
        settings.setValue(
            QStringLiteral( "lensFocus" ), m_lensFocus->value() );
        settings.endGroup();
    }
#endif

    settings.endGroup();
    settings.sync();
}

void MainWindow::loadPreferences()
{
    QSettings settings;
    settings.beginGroup( kPrefsRootQLS );
    if ( !settings.contains( QStringLiteral( "formatVersion" ) ) )
    {
        settings.endGroup();
        return;
    }

    settings.beginGroup( QStringLiteral( "window" ) );
    const QByteArray geometry =
        settings.value( QStringLiteral( "geometry" ) ).toByteArray();
    if ( !geometry.isEmpty() )
    {
        restoreGeometry( geometry );
    }
    const int tabIndex =
        settings.value( QStringLiteral( "settingsTab" ), 0 ).toInt();
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "paths" ) );
    if ( m_outputDir != nullptr )
    {
        m_outputDir->setText(
            settings.value( QStringLiteral( "outputDir" ) ).toString() );
    }
    if ( m_dataDir != nullptr )
    {
        m_dataDir->setText(
            settings.value( QStringLiteral( "spectralDataOverride" ) )
                .toString() );
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "colour" ) );
    setComboBoxIndexClamped(
        m_wbMethod,
        settings.value( QStringLiteral( "wbMethodIndex" ), 0 ).toInt() );
    if ( m_illuminant != nullptr )
    {
        m_illuminant->setText(
            settings.value( QStringLiteral( "illuminant" ) ).toString() );
    }
    for ( int i = 0; i < 4 && m_wbBox[i] != nullptr; ++i )
    {
        m_wbBox[i]->setValue(
            settings
                .value(
                    QStringLiteral( "wbBox%1" ).arg( i ), m_wbBox[i]->value() )
                .toInt() );
    }
    for ( int i = 0; i < 4 && m_customWb[i] != nullptr; ++i )
    {
        m_customWb[i]->setValue(
            settings
                .value(
                    QStringLiteral( "customWb%1" ).arg( i ),
                    m_customWb[i]->value() )
                .toDouble() );
    }
    setComboBoxIndexClamped(
        m_matrixMethod,
        settings.value( QStringLiteral( "matrixMethodIndex" ), 0 ).toInt() );
    for ( int r = 0; r < 3; ++r )
    {
        for ( int c = 0; c < 3; ++c )
        {
            if ( m_customMat[r][c] != nullptr )
            {
                m_customMat[r][c]->setValue(
                    settings
                        .value(
                            QStringLiteral( "customMatrix_%1_%2" )
                                .arg( r )
                                .arg( c ),
                            m_customMat[r][c]->value() )
                        .toDouble() );
            }
        }
    }
    if ( m_customCameraMake != nullptr )
    {
        m_customCameraMake->setText(
            settings.value( QStringLiteral( "customCameraMake" ) ).toString() );
    }
    if ( m_customCameraModel != nullptr )
    {
        m_customCameraModel->setText(
            settings.value( QStringLiteral( "customCameraModel" ) )
                .toString() );
    }
    if ( m_headroom != nullptr )
    {
        m_headroom->setValue(
            settings.value( QStringLiteral( "headroom" ), m_headroom->value() )
                .toDouble() );
    }
    if ( m_scale != nullptr )
    {
        m_scale->setValue(
            settings.value( QStringLiteral( "scale" ), m_scale->value() )
                .toDouble() );
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "raw" ) );
    if ( m_autoBright != nullptr )
    {
        m_autoBright->setChecked(
            settings
                .value(
                    QStringLiteral( "autoBright" ), m_autoBright->isChecked() )
                .toBool() );
    }
    if ( m_adjustMaximum != nullptr )
    {
        m_adjustMaximum->setValue( settings
                                       .value(
                                           QStringLiteral( "adjustMaximum" ),
                                           m_adjustMaximum->value() )
                                       .toDouble() );
    }
    if ( m_blackLevelFromMetadata != nullptr )
    {
        m_blackLevelFromMetadata->setChecked(
            settings
                .value(
                    QStringLiteral( "blackLevelFromMetadata" ),
                    m_blackLevelFromMetadata->isChecked() )
                .toBool() );
    }
    if ( m_blackLevel != nullptr )
    {
        m_blackLevel->setValue(
            settings
                .value( QStringLiteral( "blackLevel" ), m_blackLevel->value() )
                .toInt() );
    }
    if ( m_saturationFromMetadata != nullptr )
    {
        m_saturationFromMetadata->setChecked(
            settings
                .value(
                    QStringLiteral( "saturationFromMetadata" ),
                    m_saturationFromMetadata->isChecked() )
                .toBool() );
    }
    if ( m_saturationLevel != nullptr )
    {
        m_saturationLevel->setValue(
            settings
                .value(
                    QStringLiteral( "saturationLevel" ),
                    m_saturationLevel->value() )
                .toInt() );
    }
    if ( m_chromaR != nullptr )
    {
        m_chromaR->setValue(
            settings.value( QStringLiteral( "chromaR" ), m_chromaR->value() )
                .toDouble() );
    }
    if ( m_chromaB != nullptr )
    {
        m_chromaB->setValue(
            settings.value( QStringLiteral( "chromaB" ), m_chromaB->value() )
                .toDouble() );
    }
    if ( m_halfSize != nullptr )
    {
        m_halfSize->setChecked(
            settings
                .value( QStringLiteral( "halfSize" ), m_halfSize->isChecked() )
                .toBool() );
    }
    if ( m_highlightMode != nullptr )
    {
        const int v = settings
                          .value(
                              QStringLiteral( "highlightMode" ),
                              m_highlightMode->currentData().toInt() )
                          .toInt();
        setComboBoxCurrentByIntData( m_highlightMode, std::clamp( v, 0, 9 ) );
    }
    for ( int i = 0; i < 4 && m_cropBox[i] != nullptr; ++i )
    {
        m_cropBox[i]->setValue( settings
                                    .value(
                                        QStringLiteral( "cropBox%1" ).arg( i ),
                                        m_cropBox[i]->value() )
                                    .toInt() );
    }
    setComboBoxIndexClamped(
        m_cropMode,
        settings.value( QStringLiteral( "cropModeIndex" ), 1 ).toInt() );
    if ( m_flip != nullptr )
    {
        const int v =
            settings
                .value(
                    QStringLiteral( "flip" ), m_flip->currentData().toInt() )
                .toInt();
        setComboBoxCurrentByIntData( m_flip, std::clamp( v, 0, 8 ) );
    }
    if ( m_denoise != nullptr )
    {
        m_denoise->setValue(
            settings.value( QStringLiteral( "denoise" ), m_denoise->value() )
                .toDouble() );
    }
    if ( m_demosaic != nullptr )
    {
        const QString algo =
            settings.value( QStringLiteral( "demosaicAlgorithm" ) ).toString();
        if ( !algo.isEmpty() )
        {
            const int demosaicIx = m_demosaic->findText( algo );
            if ( demosaicIx >= 0 )
            {
                m_demosaic->setCurrentIndex( demosaicIx );
            }
        }
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "output" ) );
    if ( m_overwrite != nullptr )
    {
        m_overwrite->setChecked(
            settings
                .value(
                    QStringLiteral( "overwrite" ), m_overwrite->isChecked() )
                .toBool() );
    }
    if ( m_createDirs != nullptr )
    {
        m_createDirs->setChecked(
            settings
                .value(
                    QStringLiteral( "createDirs" ), m_createDirs->isChecked() )
                .toBool() );
    }
    settings.endGroup();

    settings.beginGroup( QStringLiteral( "diagnostics" ) );
    if ( m_useTiming != nullptr )
    {
        m_useTiming->setChecked(
            settings
                .value(
                    QStringLiteral( "useTiming" ), m_useTiming->isChecked() )
                .toBool() );
    }
    if ( m_disableCache != nullptr )
    {
        m_disableCache->setChecked( settings
                                        .value(
                                            QStringLiteral( "disableCache" ),
                                            m_disableCache->isChecked() )
                                        .toBool() );
    }
    if ( m_disableExiftool != nullptr )
    {
        m_disableExiftool->setChecked(
            settings
                .value(
                    QStringLiteral( "disableExiftool" ),
                    m_disableExiftool->isChecked() )
                .toBool() );
    }
    if ( m_verbosity != nullptr )
    {
        setVerbosityComboFromLevel(
            m_verbosity,
            settings.value( QStringLiteral( "verbosity" ), 0 ).toInt() );
    }
    settings.endGroup();

#ifdef RTA_GUI_HAS_LENSFUN
    if ( m_lensCorrAberration != nullptr )
    {
        settings.beginGroup( QStringLiteral( "lens" ) );
        m_lensCorrAberration->setChecked(
            settings
                .value(
                    QStringLiteral( "corrAberration" ),
                    m_lensCorrAberration->isChecked() )
                .toBool() );
        m_lensCorrDistortion->setChecked(
            settings
                .value(
                    QStringLiteral( "corrDistortion" ),
                    m_lensCorrDistortion->isChecked() )
                .toBool() );
        m_lensCorrVignetting->setChecked(
            settings
                .value(
                    QStringLiteral( "corrVignetting" ),
                    m_lensCorrVignetting->isChecked() )
                .toBool() );
        m_requireLens->setChecked( settings
                                       .value(
                                           QStringLiteral( "requireLens" ),
                                           m_requireLens->isChecked() )
                                       .toBool() );
        m_lensMetadataOverride->setChecked(
            settings.value( QStringLiteral( "lensMetadataOverride" ), false )
                .toBool() );
        m_lensMake->setText(
            settings.value( QStringLiteral( "lensMake" ) ).toString() );
        m_lensModel->setText(
            settings.value( QStringLiteral( "lensModel" ) ).toString() );
        m_lensAperture->setValue(
            settings
                .value(
                    QStringLiteral( "lensAperture" ), m_lensAperture->value() )
                .toDouble() );
        m_lensFocal->setValue(
            settings
                .value( QStringLiteral( "lensFocal" ), m_lensFocal->value() )
                .toDouble() );
        m_lensFocus->setValue(
            settings
                .value( QStringLiteral( "lensFocus" ), m_lensFocus->value() )
                .toDouble() );
        settings.endGroup();
        updateLensMetadataOverrideUi();
    }
#endif

    settings.endGroup();

    updateWbMethodDependentUi();
    updateMatrixMethodDependentUi();
    updateBlackSaturationUi();

    if ( m_settingsTabs != nullptr )
    {
        const int tabCount = m_settingsTabs->count();
        if ( tabCount > 0 )
        {
            m_settingsTabs->setCurrentIndex(
                qBound( 0, tabIndex, tabCount - 1 ) );
        }
    }
}
