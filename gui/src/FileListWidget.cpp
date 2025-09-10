#include "FileListWidget.h"
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include "utils/ImageUtils.h"

FileListWidget::FileListWidget(QWidget *parent)
    : QWidget(parent)
    , m_listWidget(nullptr)
    , m_infoLabel(nullptr)
    , m_removeButton(nullptr)
    , m_clearButton(nullptr)
    , m_contextMenu(nullptr)
    , m_removeAction(nullptr)
    , m_clearAction(nullptr)
    , m_showInFinderAction(nullptr)
{
    setupUI();
}

void FileListWidget::setupUI()
{
    setWindowTitle("Input Files");
    
    // Create list widget
    m_listWidget = new QListWidget;
    m_listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listWidget->setIconSize(QSize(96, 72));
    m_listWidget->setUniformItemSizes(true);
    
    // Create info label
    m_infoLabel = new QLabel("Drop RAW files here or use File menu");
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet("QLabel { color: #888; font-style: italic; padding: 20px; }");
    
    // Create buttons
    m_removeButton = new QPushButton("Remove Selected");
    m_removeButton->setEnabled(false);
    
    m_clearButton = new QPushButton("Clear All");
    m_clearButton->setEnabled(false);
    
    // Layout buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_removeButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();
    
    // Main layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_listWidget);
    layout->addWidget(m_infoLabel);
    layout->addLayout(buttonLayout);
    setLayout(layout);
    
    // Create context menu
    m_contextMenu = new QMenu(this);
    m_removeAction = m_contextMenu->addAction("Remove");
    m_contextMenu->addSeparator();
    m_clearAction = m_contextMenu->addAction("Clear All");
    m_contextMenu->addSeparator();
    m_showInFinderAction = m_contextMenu->addAction("Show in Finder");
    
    // Connect signals
    connect(m_listWidget, &QListWidget::itemSelectionChanged, 
            this, &FileListWidget::onItemSelectionChanged);
    connect(m_listWidget, &QListWidget::customContextMenuRequested,
            this, &FileListWidget::showItemContextMenu);
    connect(m_removeButton, &QPushButton::clicked, this, &FileListWidget::removeSelected);
    connect(m_clearButton, &QPushButton::clicked, this, &FileListWidget::removeAll);
    connect(m_removeAction, &QAction::triggered, this, &FileListWidget::removeSelected);
    connect(m_clearAction, &QAction::triggered, this, &FileListWidget::removeAll);
    connect(m_showInFinderAction, &QAction::triggered, [this]() {
        QString currentFile = getCurrentFile();
        if (!currentFile.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(currentFile).absolutePath()));
        }
    });
    
    updateFileInfo();
}

void FileListWidget::addFiles(const QStringList &files)
{
    QStringList validFiles;
    QStringList invalidFiles;
    QStringList duplicateFiles;
    
    // Get existing files
    QStringList existingFiles = getAllFiles();
    
    for (const QString &file : files) {
        QFileInfo fileInfo(file);
        
        if (!fileInfo.exists()) {
            invalidFiles << file;
            continue;
        }
        
        if (fileInfo.isDir()) {
            // Handle directory - scan for RAW files
            QDir dir(file);
            QStringList filters;
            filters << "*.raw" << "*.cr2" << "*.cr3" << "*.nef" << "*.arw" 
                    << "*.dng" << "*.orf" << "*.raf" << "*.rw2";
            
            QStringList dirFiles = dir.entryList(filters, QDir::Files);
            QStringList absoluteDirFiles;
            for (const QString &dirFile : dirFiles) {
                absoluteDirFiles << dir.absoluteFilePath(dirFile);
            }
            
            if (!absoluteDirFiles.isEmpty()) {
                addFiles(absoluteDirFiles);  // Recursive call
            }
            continue;
        }
        
        if (!isValidRawFile(file)) {
            invalidFiles << file;
            continue;
        }
        
        QString absolutePath = fileInfo.absoluteFilePath();
        if (existingFiles.contains(absolutePath)) {
            duplicateFiles << absolutePath;
            continue;
        }
        
        validFiles << absolutePath;
    }
    
    // Add valid files to list
    for (const QString &file : validFiles) {
        QFileInfo fileInfo(file);
    QListWidgetItem *item = new QListWidgetItem;
        item->setText(fileInfo.fileName());
        item->setToolTip(file);
        item->setData(Qt::UserRole, file);
        
        // Add file size info
        QString sizeText = QString(" (%1)").arg(formatFileSize(fileInfo.size()));
        item->setText(item->text() + sizeText);
        
        // Small thumbnail icon
    QImage thumb = ImageUtils::loadFramedThumbnail(file, 96, 72);
    item->setIcon(QPixmap::fromImage(thumb));
        m_listWidget->addItem(item);
    }
    
    // Show warnings for invalid files
    if (!invalidFiles.isEmpty()) {
        QString message = QString("The following files are not valid RAW files:\n%1")
                         .arg(invalidFiles.join("\n"));
        QMessageBox::warning(this, "Invalid Files", message);
    }
    
    if (!duplicateFiles.isEmpty()) {
        QString message = QString("The following files are already in the list:\n%1")
                         .arg(duplicateFiles.join("\n"));
        QMessageBox::information(this, "Duplicate Files", message);
    }
    
    updateFileInfo();
    
    if (!validFiles.isEmpty()) {
        emit filesChanged();
    }
}

void FileListWidget::clear()
{
    m_listWidget->clear();
    updateFileInfo();
    emit filesChanged();
}

int FileListWidget::getFileCount() const
{
    return m_listWidget->count();
}

QStringList FileListWidget::getAllFiles() const
{
    QStringList files;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        files << item->data(Qt::UserRole).toString();
    }
    return files;
}

QString FileListWidget::getCurrentFile() const
{
    QListWidgetItem *currentItem = m_listWidget->currentItem();
    if (currentItem) {
        return currentItem->data(Qt::UserRole).toString();
    }
    return QString();
}

void FileListWidget::onItemSelectionChanged()
{
    QList<QListWidgetItem*> selectedItems = m_listWidget->selectedItems();
    m_removeButton->setEnabled(!selectedItems.isEmpty());
    
    if (!selectedItems.isEmpty()) {
        QString filename = selectedItems.first()->data(Qt::UserRole).toString();
        emit fileSelected(filename);
    }
}

void FileListWidget::removeSelected()
{
    QList<QListWidgetItem*> selectedItems = m_listWidget->selectedItems();
    for (QListWidgetItem *item : selectedItems) {
        delete m_listWidget->takeItem(m_listWidget->row(item));
    }
    
    updateFileInfo();
    emit filesChanged();
}

void FileListWidget::removeAll()
{
    if (m_listWidget->count() > 0) {
        int ret = QMessageBox::question(this, "Clear All Files",
            "Are you sure you want to remove all files from the list?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            clear();
        }
    }
}

void FileListWidget::showItemContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_listWidget->itemAt(pos);
    if (item) {
        m_removeAction->setEnabled(true);
        m_showInFinderAction->setEnabled(true);
    } else {
        m_removeAction->setEnabled(false);
        m_showInFinderAction->setEnabled(false);
    }
    
    m_clearAction->setEnabled(m_listWidget->count() > 0);
    m_contextMenu->exec(m_listWidget->mapToGlobal(pos));
}

void FileListWidget::updateFileInfo()
{
    int count = m_listWidget->count();
    m_clearButton->setEnabled(count > 0);
    
    if (count == 0) {
        m_infoLabel->setText("Drop RAW files here or use File menu");
        m_infoLabel->setVisible(true);
    } else {
        m_infoLabel->setVisible(false);
        
        // Calculate total size
        qint64 totalSize = 0;
        for (int i = 0; i < count; ++i) {
            QString filename = m_listWidget->item(i)->data(Qt::UserRole).toString();
            QFileInfo fileInfo(filename);
            totalSize += fileInfo.size();
        }
        
        QString statusText = QString("%1 files, %2 total")
                           .arg(count)
                           .arg(formatFileSize(totalSize));
        
        // Update window title
        setWindowTitle(QString("Input Files (%1)").arg(statusText));
    }
}

bool FileListWidget::isValidRawFile(const QString &filename) const
{
    QFileInfo fileInfo(filename);
    QString ext = fileInfo.suffix().toLower();
    
    QStringList validExtensions;
    validExtensions << "raw" << "cr2" << "cr3" << "nef" << "arw" 
                    << "dng" << "orf" << "raf" << "rw2" << "3fr"
                    << "ari" << "bay" << "crw" << "dcr" << "erf"
                    << "fff" << "mef" << "mos" << "mrw" << "nrw"
                    << "pef" << "ptx" << "r3d" << "rwl" << "sr2"
                    << "srf" << "x3f";
    
    return validExtensions.contains(ext);
}

QString FileListWidget::formatFileSize(qint64 size) const
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
