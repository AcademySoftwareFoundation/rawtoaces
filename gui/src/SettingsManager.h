#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include "ParameterWidget.h"

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    static SettingsManager& instance();
    
    void loadSettings();
    void saveSettings();
    
    // Last used directory
    QString getLastDirectory() const;
    void setLastDirectory(const QString &directory);
    
    // Conversion parameters
    ConversionParameters getDefaultParameters() const;
    void setDefaultParameters(const ConversionParameters &parameters);
    
    // UI settings
    QByteArray getWindowGeometry() const;
    void setWindowGeometry(const QByteArray &geometry);
    
    QByteArray getWindowState() const;
    void setWindowState(const QByteArray &state);
    
    QByteArray getSplitterState(const QString &name) const;
    void setSplitterState(const QString &name, const QByteArray &state);

private:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager() = default;
    
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    
    QSettings *m_settings;
    QString m_lastDirectory;
    ConversionParameters m_defaultParameters;
};
