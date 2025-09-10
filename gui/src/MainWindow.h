#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QTimer>
#include <QDockWidget>
#include <QPlainTextEdit>
#include <QAction>
#include <QStyle>

class FileListWidget;
class ParameterWidget;
class ImageViewer;
class ConversionWorker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void addFiles(const QStringList &files);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openFiles();
    void openFolder();
    void clearFiles();
    void showAbout();
    void showSettings();
    void startConversion();
    void stopConversion();
    void onConversionProgress(int current, int total, const QString &filename);
    void onConversionFinished(bool success, const QString &message);
    void onFileSelectionChanged(const QString &filename);
    void updateStatusBar();
    void clearLog();
    void copyLog();
    void saveLog();
    void appendLog(const QString &text);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupLogDock();
    void connectSignals();
    void updateConversionButton();
    void saveSettings();
    void loadSettings();
    
    // UI Components
    QWidget *m_centralWidget;
    QSplitter *m_mainSplitter;
    QSplitter *m_rightSplitter;
    
    FileListWidget *m_fileListWidget;
    ParameterWidget *m_parameterWidget;
    ImageViewer *m_imageViewer;
    
    // Menu and toolbar
    QAction *m_openFilesAction;
    QAction *m_openFolderAction;
    QAction *m_clearAction;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_settingsAction;
    
    // Status bar
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_fileCountLabel;
    
    // Control buttons
    QPushButton *m_convertButton;
    QPushButton *m_stopButton;
    
    // Worker thread
    ConversionWorker *m_conversionWorker;
    QTimer *m_statusUpdateTimer;
    
    // State
    bool m_conversionInProgress;
    int m_totalFiles;
    int m_convertedFiles;

    // Log dock
    QDockWidget *m_logDock;
    QPlainTextEdit *m_logView;
    QAction *m_viewLogAction;
    QAction *m_clearLogAction;
    QAction *m_copyLogAction;
    QAction *m_saveLogAction;
};
