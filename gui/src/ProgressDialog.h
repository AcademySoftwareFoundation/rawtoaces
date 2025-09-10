#pragma once

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

class ProgressDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();

    void setProgress(int value);
    void setRange(int minimum, int maximum);
    void setStatus(const QString &status);
    void setDetails(const QString &details);
    void appendLog(const QString &message);

    void setCanCancel(bool canCancel);
    void setCanPause(bool canPause);
    void setIndeterminate(bool indeterminate);
    void reset();

signals:
    void pauseRequested();
    void cancelRequested();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void toggleDetails(bool show);
    void pauseClicked();
    void cancelClicked();

private:
    void setupUI();

    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_detailsLabel;
    QPushButton *m_cancelButton;
    QPushButton *m_pauseButton;
    QPushButton *m_expandButton;
    QWidget *m_detailsWidget;
    QTextEdit *m_logTextEdit;

    bool m_isExpanded;
    bool m_canCancel;
    bool m_canPause;
};
