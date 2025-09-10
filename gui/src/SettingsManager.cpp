#include "SettingsManager.h"
#include <QStandardPaths>
#include <QDir>

SettingsManager& SettingsManager::instance()
{
    static SettingsManager instance;
    return instance;
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings(nullptr)
{
    m_settings = new QSettings(this);
    m_lastDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

void SettingsManager::loadSettings()
{
    m_lastDirectory = m_settings->value("lastDirectory", 
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
}

void SettingsManager::saveSettings()
{
    m_settings->setValue("lastDirectory", m_lastDirectory);
    m_settings->sync();
}

QString SettingsManager::getLastDirectory() const
{
    return m_lastDirectory;
}

void SettingsManager::setLastDirectory(const QString &directory)
{
    m_lastDirectory = directory;
    m_settings->setValue("lastDirectory", directory);
}

ConversionParameters SettingsManager::getDefaultParameters() const
{
    return m_defaultParameters;
}

void SettingsManager::setDefaultParameters(const ConversionParameters &parameters)
{
    m_defaultParameters = parameters;
}

QByteArray SettingsManager::getWindowGeometry() const
{
    return m_settings->value("windowGeometry").toByteArray();
}

void SettingsManager::setWindowGeometry(const QByteArray &geometry)
{
    m_settings->setValue("windowGeometry", geometry);
}

QByteArray SettingsManager::getWindowState() const
{
    return m_settings->value("windowState").toByteArray();
}

void SettingsManager::setWindowState(const QByteArray &state)
{
    m_settings->setValue("windowState", state);
}

QByteArray SettingsManager::getSplitterState(const QString &name) const
{
    return m_settings->value(QString("splitter_%1").arg(name)).toByteArray();
}

void SettingsManager::setSplitterState(const QString &name, const QByteArray &state)
{
    m_settings->setValue(QString("splitter_%1").arg(name), state);
}
