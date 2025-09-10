#pragma once

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileInfo>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMenu>
#include <QContextMenuEvent>

class FileListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileListWidget(QWidget *parent = nullptr);
    
    void addFiles(const QStringList &files);
    void clear();
    int getFileCount() const;
    QStringList getAllFiles() const;
    QString getCurrentFile() const;

signals:
    void filesChanged();
    void fileSelected(const QString &filename);

private slots:
    void onItemSelectionChanged();
    void removeSelected();
    void removeAll();
    void showItemContextMenu(const QPoint &pos);

private:
    void setupUI();
    void updateFileInfo();
    bool isValidRawFile(const QString &filename) const;
    QString formatFileSize(qint64 size) const;
    
    QListWidget *m_listWidget;
    QLabel *m_infoLabel;
    QPushButton *m_removeButton;
    QPushButton *m_clearButton;
    
    QMenu *m_contextMenu;
    QAction *m_removeAction;
    QAction *m_clearAction;
    QAction *m_showInFinderAction;
};
