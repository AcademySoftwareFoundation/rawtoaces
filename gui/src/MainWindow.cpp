#include "MainWindow.h"
#include "FileListWidget.h"
#include "ParameterWidget.h"
#include "ImageViewer.h"
#include "ConversionWorker.h"
#include "SettingsManager.h"
#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QThread>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainSplitter(nullptr)
    , m_rightSplitter(nullptr)
    , m_fileListWidget(nullptr)
    , m_parameterWidget(nullptr)
    , m_imageViewer(nullptr)
    , m_openFilesAction(nullptr)
    , m_openFolderAction(nullptr)
    , m_clearAction(nullptr)
    , m_exitAction(nullptr)
    , m_aboutAction(nullptr)
    , m_settingsAction(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_fileCountLabel(nullptr)
    , m_convertButton(nullptr)
    , m_stopButton(nullptr)
    , m_conversionWorker(nullptr)
    , m_statusUpdateTimer(nullptr)
    , m_conversionInProgress(false)
    , m_totalFiles(0)
    , m_convertedFiles(0)
{
    setupUI();
    setupMenuBar();
    setupStatusBar();
    connectSignals();
    
    setAcceptDrops(true);
    setWindowTitle("RAWtoACES GUI - Academy Software Foundation");
    QIcon appIcon;
    if (QFile::exists(":/icons/rawtoaces-brand.png")) {
        appIcon = QIcon(":/icons/rawtoaces-brand.png");
    } else {
        appIcon = QIcon(":/icons/rawtoaces.png");
    }
    setWindowIcon(appIcon);

    // Log dock
    setupLogDock();
    
    // Initialize timer
    m_statusUpdateTimer = new QTimer(this);
    m_statusUpdateTimer->setInterval(100);
    connect(m_statusUpdateTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    
    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
    
    if (m_conversionWorker) {
        m_conversionWorker->stop();
        m_conversionWorker->wait();
        delete m_conversionWorker;
    }
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    // Create main splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Create file list widget
    m_fileListWidget = new FileListWidget;
    m_fileListWidget->setMinimumWidth(300);
    
    // Create right splitter for parameters and image viewer
    m_rightSplitter = new QSplitter(Qt::Vertical);
    
    // Create parameter widget
    m_parameterWidget = new ParameterWidget;
    m_parameterWidget->setMinimumHeight(200);
    
    // Create image viewer
    m_imageViewer = new ImageViewer;
    m_imageViewer->setMinimumHeight(300);
    // Connect selection from viewer to parameter widget setters
    connect(m_imageViewer, &ImageViewer::selectionChanged, this, [this](const QRect &sel){
        QRect imgSel = m_imageViewer->getSelectionInImagePixels();
        if (!imgSel.isEmpty()) {
            // Decide routing based on current WB mode; if WB box mode -> WB, else -> crop
            auto params = m_parameterWidget->getParameters();
            if (params.wbMethod == "box") {
                m_parameterWidget->setWbBoxFromSelection(imgSel);
            } else {
                m_parameterWidget->setCropBoxFromSelection(imgSel);
            }
        }
    });
    
    // Add widgets to right splitter
    m_rightSplitter->addWidget(m_parameterWidget);
    m_rightSplitter->addWidget(m_imageViewer);
    m_rightSplitter->setSizes({300, 500});
    
    // Add widgets to main splitter
    m_mainSplitter->addWidget(m_fileListWidget);
    m_mainSplitter->addWidget(m_rightSplitter);
    m_mainSplitter->setSizes({400, 800});
    
    // Create control buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    m_convertButton = new QPushButton("Convert Files");
    m_convertButton->setEnabled(false);
    m_convertButton->setMinimumHeight(40);
    m_convertButton->setStyleSheet("QPushButton { font-weight: bold; background-color: #2E7D32; } QPushButton:hover { background-color: #4CAF50; }");
    
    m_stopButton = new QPushButton("Stop");
    m_stopButton->setEnabled(false);
    m_stopButton->setMinimumHeight(40);
    m_stopButton->setStyleSheet("QPushButton { font-weight: bold; background-color: #C62828; } QPushButton:hover { background-color: #F44336; }");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_convertButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addStretch();
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_mainSplitter);
    mainLayout->addLayout(buttonLayout);
    
    m_centralWidget->setLayout(mainLayout);
}

void MainWindow::setupMenuBar()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    m_openFilesAction = new QAction("&Open Files...", this);
    m_openFilesAction->setShortcut(QKeySequence::Open);
    m_openFilesAction->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    fileMenu->addAction(m_openFilesAction);
    
    m_openFolderAction = new QAction("Open &Folder...", this);
    m_openFolderAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    m_openFolderAction->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    fileMenu->addAction(m_openFolderAction);
    
    fileMenu->addSeparator();
    
    m_clearAction = new QAction("&Clear List", this);
    m_clearAction->setShortcut(QKeySequence("Ctrl+L"));
    m_clearAction->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    fileMenu->addAction(m_clearAction);
    
    fileMenu->addSeparator();
    
    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    fileMenu->addAction(m_exitAction);
    
    // Tools menu
    QMenu *toolsMenu = menuBar()->addMenu("&Tools");
    
    m_settingsAction = new QAction("&Settings...", this);
    m_settingsAction->setShortcut(QKeySequence::Preferences);
    m_settingsAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    toolsMenu->addAction(m_settingsAction);

    // View menu
    QMenu *viewMenu = menuBar()->addMenu("&View");
    m_viewLogAction = new QAction("&Log", this);
    m_viewLogAction->setCheckable(true);
    m_viewLogAction->setChecked(true);
    viewMenu->addAction(m_viewLogAction);
    connect(m_viewLogAction, &QAction::toggled, [this](bool on){ if (m_logDock) m_logDock->setVisible(on); });
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    
    m_aboutAction = new QAction("&About", this);
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar;
    m_progressBar->setVisible(false);
    statusBar()->addPermanentWidget(m_progressBar);
    
    m_fileCountLabel = new QLabel("Files: 0");
    statusBar()->addPermanentWidget(m_fileCountLabel);
}

void MainWindow::connectSignals()
{
    // Menu actions
    connect(m_openFilesAction, &QAction::triggered, this, &MainWindow::openFiles);
    connect(m_openFolderAction, &QAction::triggered, this, &MainWindow::openFolder);
    connect(m_clearAction, &QAction::triggered, this, &MainWindow::clearFiles);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
    
    // Buttons
    connect(m_convertButton, &QPushButton::clicked, this, &MainWindow::startConversion);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::stopConversion);
    
    // File list widget
    connect(m_fileListWidget, &FileListWidget::filesChanged, this, &MainWindow::updateConversionButton);
    connect(m_fileListWidget, &FileListWidget::fileSelected, this, &MainWindow::onFileSelectionChanged);
}

void MainWindow::setupLogDock()
{
    m_logDock = new QDockWidget("Log", this);
    m_logDock->setObjectName("LogDock");
    m_logDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    m_logView = new QPlainTextEdit(m_logDock);
    m_logView->setReadOnly(true);
    m_logView->setMaximumBlockCount(5000);
    m_logDock->setWidget(m_logView);
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);

    // Dock context actions: Clear/Copy/Save
    m_clearLogAction = new QAction("Clear Log", this);
    connect(m_clearLogAction, &QAction::triggered, this, &MainWindow::clearLog);
    m_copyLogAction = new QAction("Copy All", this);
    connect(m_copyLogAction, &QAction::triggered, this, &MainWindow::copyLog);
    m_saveLogAction = new QAction("Save Log…", this);
    connect(m_saveLogAction, &QAction::triggered, this, &MainWindow::saveLog);
    m_logDock->addActions({m_clearLogAction, m_copyLogAction, m_saveLogAction});
    m_logDock->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void MainWindow::addFiles(const QStringList &files)
{
    if (m_fileListWidget) {
        m_fileListWidget->addFiles(files);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QStringList files;
    for (const QUrl &url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            files << url.toLocalFile();
        }
    }
    
    if (!files.isEmpty()) {
        addFiles(files);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_conversionInProgress) {
        int ret = QMessageBox::question(this, "Conversion in Progress",
            "A conversion is currently in progress. Do you want to stop it and exit?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            stopConversion();
            event->accept();
        } else {
            event->ignore();
            return;
        }
    }
    
    saveSettings();
    event->accept();
}

void MainWindow::openFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(this,
        "Select RAW Files",
        SettingsManager::instance().getLastDirectory(),
        "RAW Files (*.raw *.cr2 *.cr3 *.nef *.arw *.dng *.orf *.raf *.rw2);;All Files (*)");
    
    if (!files.isEmpty()) {
        addFiles(files);
        SettingsManager::instance().setLastDirectory(QFileInfo(files.first()).absolutePath());
    }
}

void MainWindow::openFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this,
        "Select Folder Containing RAW Files",
        SettingsManager::instance().getLastDirectory());
    
    if (!folder.isEmpty()) {
        QStringList files;
        QDir dir(folder);
        QStringList filters;
        filters << "*.raw" << "*.cr2" << "*.cr3" << "*.nef" << "*.arw" 
                << "*.dng" << "*.orf" << "*.raf" << "*.rw2";
        
        for (const QString &filter : filters) {
            files += dir.entryList(QStringList() << filter, QDir::Files);
        }
        
        // Convert to absolute paths
        QStringList absoluteFiles;
        for (const QString &file : files) {
            absoluteFiles << dir.absoluteFilePath(file);
        }
        
        if (!absoluteFiles.isEmpty()) {
            addFiles(absoluteFiles);
            SettingsManager::instance().setLastDirectory(folder);
        } else {
            QMessageBox::information(this, "No Files Found", 
                "No RAW files were found in the selected folder.");
        }
    }
}

void MainWindow::clearFiles()
{
    if (m_fileListWidget) {
        m_fileListWidget->clear();
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "About RAWtoACES GUI",
        "<h3>RAWtoACES GUI v1.0.0</h3>"
        "<p>A graphical user interface for the RAWtoACES command-line tool.</p>"
        "<p>RAWtoACES converts digital camera RAW files to ACES container files.</p>"
        "<p><b>Academy Software Foundation</b><br>"
        "Website: <a href='https://www.aswf.io'>https://www.aswf.io</a></p>"
        "<p>Built with Qt and modern C++</p>");
}

void MainWindow::showSettings()
{
    // TODO: Implement settings dialog
    QMessageBox::information(this, "Settings", "Settings dialog not yet implemented.");
}

void MainWindow::startConversion()
{
    if (!m_fileListWidget || m_fileListWidget->getFileCount() == 0) {
        return;
    }
    
    // Get conversion parameters
    auto parameters = m_parameterWidget->getParameters();
    auto files = m_fileListWidget->getAllFiles();
    
    // Create worker thread
    m_conversionWorker = new ConversionWorker(files, parameters);
    connect(m_conversionWorker, &ConversionWorker::progress, 
            this, &MainWindow::onConversionProgress);
    connect(m_conversionWorker, &ConversionWorker::finished, 
            this, &MainWindow::onConversionFinished);
    connect(m_conversionWorker, &ConversionWorker::logMessage,
            this, &MainWindow::appendLog);
    
    // Update UI
    m_conversionInProgress = true;
    m_convertButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, files.size());
    m_progressBar->setValue(0);
    m_statusUpdateTimer->start();
    
    // Start conversion
    m_conversionWorker->start();
}

void MainWindow::stopConversion()
{
    if (m_conversionWorker) {
        m_conversionWorker->stop();
        m_statusLabel->setText("Stopping conversion...");
    }
}

void MainWindow::onConversionProgress(int current, int total, const QString &filename)
{
    m_progressBar->setValue(current);
    m_statusLabel->setText(QString("Converting: %1").arg(QFileInfo(filename).fileName()));
    m_convertedFiles = current;
    m_totalFiles = total;
}

void MainWindow::onConversionFinished(bool success, const QString &message)
{
    m_conversionInProgress = false;
    m_convertButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_progressBar->setVisible(false);
    m_statusUpdateTimer->stop();
    
    if (success) {
        m_statusLabel->setText("Conversion completed successfully");
        QMessageBox::information(this, "Conversion Complete", 
            "All files have been converted successfully.");
    } else {
        m_statusLabel->setText("Conversion failed");
        QMessageBox::critical(this, "Conversion Error", 
            QString("Conversion failed: %1").arg(message));
    }
    
    // Clean up worker
    if (m_conversionWorker) {
        m_conversionWorker->deleteLater();
        m_conversionWorker = nullptr;
    }
}

void MainWindow::onFileSelectionChanged(const QString &filename)
{
    if (m_imageViewer) {
        m_imageViewer->loadImage(filename);
    }
}

void MainWindow::updateStatusBar()
{
    if (m_fileListWidget) {
        int fileCount = m_fileListWidget->getFileCount();
        m_fileCountLabel->setText(QString("Files: %1").arg(fileCount));
    }
}

void MainWindow::clearLog()
{
    if (m_logView) m_logView->clear();
}

void MainWindow::appendLog(const QString &text)
{
    if (!m_logView) return;
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) return;
    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    m_logView->appendPlainText(QString("[%1] %2").arg(ts, trimmed));
    // Auto-scroll
    QScrollBar *sb = m_logView->verticalScrollBar();
    if (sb) sb->setValue(sb->maximum());
}

void MainWindow::copyLog()
{
    if (!m_logView) return;
    m_logView->selectAll();
    m_logView->copy();
    m_logView->moveCursor(QTextCursor::End);
}

void MainWindow::saveLog()
{
    if (!m_logView) return;
    const QString path = QFileDialog::getSaveFileName(this, "Save Log", QDir::homePath()+"/rawtoaces-gui.log", "Text Files (*.txt);;All Files (*)");
    if (path.isEmpty()) return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&f);
        out << m_logView->toPlainText();
        f.close();
        statusBar()->showMessage(QString("Saved log to %1").arg(path), 3000);
    } else {
        QMessageBox::warning(this, "Save Log", QString("Failed to save log to %1").arg(path));
    }
}

void MainWindow::updateConversionButton()
{
    if (m_fileListWidget && m_convertButton) {
        m_convertButton->setEnabled(m_fileListWidget->getFileCount() > 0 && !m_conversionInProgress);
    }
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("mainSplitter", m_mainSplitter->saveState());
    settings.setValue("rightSplitter", m_rightSplitter->saveState());
}

void MainWindow::loadSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    if (m_mainSplitter) {
        m_mainSplitter->restoreState(settings.value("mainSplitter").toByteArray());
    }
    if (m_rightSplitter) {
        m_rightSplitter->restoreState(settings.value("rightSplitter").toByteArray());
    }
}
