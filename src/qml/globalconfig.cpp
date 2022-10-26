/*
    SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "globalconfig.h"

GlobalConfig::GlobalConfig(QObject *parent)
    : QObject(parent)
    , m_raiseMaximumVolume(false)
    , m_audioFeedback(true)
    , m_volumeOsd(true)
    , m_microphoneSensitivityOsd(true)
    , m_muteOsd(true)
    , m_defaultOutputDeviceOsd(true)
{
    m_configWatcher = KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("plasmaparc")));
    connect(m_configWatcher.data(), &KConfigWatcher::configChanged, this, &GlobalConfig::configChanged);
    configChanged();
}

GlobalConfig::~GlobalConfig()
{
}

void GlobalConfig::configChanged()
{
    bool raise = m_configWatcher->config()->group(QStringLiteral("Default")).readEntry(QStringLiteral("RaiseMaximumVolume"), false);
    if (raise != m_raiseMaximumVolume) {
        m_raiseMaximumVolume = raise;
        Q_EMIT raiseMaximumVolumeChanged();
    }

    raise = m_configWatcher->config()->group(QStringLiteral("Default")).readEntry(QStringLiteral("AudioFeedback"), true);
    if (raise != m_audioFeedback) {
        m_audioFeedback = raise;
        Q_EMIT audioFeedbackChanged();
    }

    raise = m_configWatcher->config()->group(QStringLiteral("Default")).readEntry(QStringLiteral("VolumeOsd"), true);
    if (raise != m_volumeOsd) {
        m_volumeOsd = raise;
        Q_EMIT volumeOsdChanged();
    }

    raise = m_configWatcher->config()->group(QStringLiteral("Default")).readEntry(QStringLiteral("MicrophoneSensitivityOsd"), true);
    if (raise != m_microphoneSensitivityOsd) {
        m_microphoneSensitivityOsd = raise;
        Q_EMIT microphoneSensitivityOsdChanged();
    }

    raise = m_configWatcher->config()->group(QStringLiteral("Default")).readEntry(QStringLiteral("MuteOsd"), true);
    if (raise != m_muteOsd) {
        m_muteOsd = raise;
        Q_EMIT muteOsdChanged();
    }

    raise = m_configWatcher->config()->group(QStringLiteral("Default")).readEntry(QStringLiteral("DefaultOutputDeviceOsd"), true);
    if (raise != m_defaultOutputDeviceOsd) {
        m_defaultOutputDeviceOsd = raise;
        Q_EMIT defaultOutputDeviceOsdChanged();
    }

    int step = m_configWatcher->config()->group(QStringLiteral("Default")).readEntry(QStringLiteral("VolumeStep"), 5);
    if (step != m_volumeStep) {
        m_volumeStep = step;
        Q_EMIT volumeStepChanged();
    }
}

bool GlobalConfig::raiseMaximumVolume()
{
    return m_raiseMaximumVolume;
}

void GlobalConfig::setRaiseMaximumVolume(bool raise)
{
    if (raise != m_raiseMaximumVolume) {
        m_configWatcher->config()->group(QStringLiteral("Default")).writeEntry(QStringLiteral("RaiseMaximumVolume"), raise, KConfigBase::Notify);
        m_configWatcher->config()->sync();
    }
}

int GlobalConfig::volumeStep()
{
    return m_volumeStep;
}

void GlobalConfig::setVolumeStep(int step)
{
    if (step != m_volumeStep) {
        m_configWatcher->config()->group(QStringLiteral("Default")).writeEntry(QStringLiteral("VolumeStep"), step, KConfigBase::Notify);
        m_configWatcher->config()->sync();
    }
}

bool GlobalConfig::audioFeedback()
{
    return m_audioFeedback;
}

void GlobalConfig::setAudioFeedback(bool raise)
{
    if (raise != m_audioFeedback) {
        m_configWatcher->config()->group(QStringLiteral("Default")).writeEntry(QStringLiteral("AudioFeedback"), raise, KConfigBase::Notify);
        m_configWatcher->config()->sync();
    }
}
bool GlobalConfig::volumeOsd()
{
    return m_volumeOsd;
}

void GlobalConfig::setVolumeOsd(bool raise)
{
    if (raise != m_volumeOsd) {
        m_configWatcher->config()->group(QStringLiteral("Default")).writeEntry(QStringLiteral("VolumeOsd"), raise, KConfigBase::Notify);
        m_configWatcher->config()->sync();
    }
}
bool GlobalConfig::microphoneSensitivityOsd()
{
    return m_microphoneSensitivityOsd;
}

void GlobalConfig::setMicrophoneSensitivityOsd(bool raise)
{
    if (raise != m_microphoneSensitivityOsd) {
        m_configWatcher->config()->group(QStringLiteral("Default")).writeEntry(QStringLiteral("MicrophoneSensitivityOsd"), raise, KConfigBase::Notify);
        m_configWatcher->config()->sync();
    }
}
bool GlobalConfig::muteOsd()
{
    return m_muteOsd;
}

void GlobalConfig::setMuteOsd(bool raise)
{
    if (raise != m_muteOsd) {
        m_configWatcher->config()->group(QStringLiteral("Default")).writeEntry(QStringLiteral("MuteOsd"), raise, KConfigBase::Notify);
        m_configWatcher->config()->sync();
    }
}
bool GlobalConfig::defaultOutputDeviceOsd()
{
    return m_defaultOutputDeviceOsd;
}

void GlobalConfig::setDefaultOutputDeviceOsd(bool raise)
{
    if (raise != m_defaultOutputDeviceOsd) {
        m_configWatcher->config()->group(QStringLiteral("Default")).writeEntry(QStringLiteral("DefaultOutputDeviceOsd"), raise, KConfigBase::Notify);
        m_configWatcher->config()->sync();
    }
}
