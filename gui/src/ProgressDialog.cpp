#include "ProgressDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QCloseEvent>

ProgressDialog::ProgressDialog(QWidget *parent)
    : QDialog(parent)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_detailsLabel(nullptr)
    , m_cancelButton(nullptr)
    , m_pauseButton(nullptr)
    , m_expandButton(nullptr)
    , m_detailsWidget(nullptr)
    , m_logTextEdit(nullptr)
    , m_isExpanded(false)
    , m_canCancel(true)
    , m_canPause(false)
{
    setupUI();
    setFixedSize(400, 150);
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::setupUI()
{
    setWindowTitle("Processing...");
    setModal(true);
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Status label
    m_statusLabel = new QLabel("Initializing...");
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);
    
    // Progress bar
    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    mainLayout->addWidget(m_progressBar);
    
    // Details label (file being processed, etc.)
    m_detailsLabel = new QLabel("");
    m_detailsLabel->setStyleSheet("QLabel { color: #666; font-size: 11px; }");
    m_detailsLabel->setWordWrap(true);
    mainLayout->addWidget(m_detailsLabel);
    
    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    m_expandButton = new QPushButton("Show Details");
    m_expandButton->setCheckable(true);
    connect(m_expandButton, &QPushButton::toggled, this, &ProgressDialog::toggleDetails);
    
    m_pauseButton = new QPushButton("Pause");
    m_pauseButton->setEnabled(false);
    connect(m_pauseButton, &QPushButton::clicked, this, &ProgressDialog::pauseClicked);
    
    m_cancelButton = new QPushButton("Cancel");
    connect(m_cancelButton, &QPushButton::clicked, this, &ProgressDialog::cancelClicked);
    
    buttonLayout->addWidget(m_expandButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_pauseButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Details widget (initially hidden)
    m_detailsWidget = new QWidget;
    QVBoxLayout *detailsLayout = new QVBoxLayout(m_detailsWidget);
    detailsLayout->setContentsMargins(0, 0, 0, 0);
    
    m_logTextEdit = new QTextEdit;
    m_logTextEdit->setMaximumHeight(150);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Courier", 9));
    detailsLayout->addWidget(m_logTextEdit);
    
    mainLayout->addWidget(m_detailsWidget);
    m_detailsWidget->hide();
}

void ProgressDialog::setProgress(int value)
{
    m_progressBar->setValue(value);
}

void ProgressDialog::setRange(int minimum, int maximum)
{
    m_progressBar->setRange(minimum, maximum);
}

void ProgressDialog::setStatus(const QString &status)
{
    m_statusLabel->setText(status);
}

void ProgressDialog::setDetails(const QString &details)
{
    m_detailsLabel->setText(details);
}

void ProgressDialog::appendLog(const QString &message)
{
    if (m_logTextEdit) {
        m_logTextEdit->append(message);
        // Auto-scroll to bottom
        QTextCursor cursor = m_logTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_logTextEdit->setTextCursor(cursor);
    }
}

void ProgressDialog::setCanCancel(bool canCancel)
{
    m_canCancel = canCancel;
    m_cancelButton->setEnabled(canCancel);
}

void ProgressDialog::setCanPause(bool canPause)
{
    m_canPause = canPause;
    m_pauseButton->setEnabled(canPause);
}

void ProgressDialog::setIndeterminate(bool indeterminate)
{
    if (indeterminate) {
        m_progressBar->setRange(0, 0);
    } else {
        m_progressBar->setRange(0, 100);
    }
}

void ProgressDialog::reset()
{
    m_progressBar->setValue(0);
    m_statusLabel->setText("Initializing...");
    m_detailsLabel->setText("");
    if (m_logTextEdit) {
        m_logTextEdit->clear();
    }
}

void ProgressDialog::toggleDetails(bool show)
{
    m_isExpanded = show;
    
    if (show) {
        m_detailsWidget->show();
        m_expandButton->setText("Hide Details");
        setFixedSize(400, 350);
    } else {
        m_detailsWidget->hide();
        m_expandButton->setText("Show Details");
        setFixedSize(400, 150);
    }
}

void ProgressDialog::pauseClicked()
{
    emit pauseRequested();
}

void ProgressDialog::cancelClicked()
{
    emit cancelRequested();
}

void ProgressDialog::closeEvent(QCloseEvent *event)
{
    if (m_canCancel) {
        emit cancelRequested();
        event->accept();
    } else {
        event->ignore();
    }
}
