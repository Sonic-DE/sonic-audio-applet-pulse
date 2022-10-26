/*
    SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include <KConfigWatcher>
#include <KSharedConfig>
#include <QObject>

class GlobalConfig : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool raiseMaximumVolume READ raiseMaximumVolume WRITE setRaiseMaximumVolume NOTIFY raiseMaximumVolumeChanged)
    Q_PROPERTY(bool audioFeedback READ audioFeedback WRITE setAudioFeedback NOTIFY audioFeedbackChanged)
    Q_PROPERTY(bool volumeOsd READ volumeOsd WRITE setVolumeOsd NOTIFY volumeOsdChanged)
    Q_PROPERTY(bool microphoneSensitivityOsd READ microphoneSensitivityOsd WRITE setMicrophoneSensitivityOsd NOTIFY microphoneSensitivityOsdChanged)
    Q_PROPERTY(bool muteOsd READ muteOsd WRITE setMuteOsd NOTIFY muteOsdChanged)
    Q_PROPERTY(bool defaultOutputDeviceOsd READ defaultOutputDeviceOsd WRITE setDefaultOutputDeviceOsd NOTIFY defaultOutputDeviceOsdChanged)
    Q_PROPERTY(int volumeStep READ volumeStep WRITE setVolumeStep NOTIFY volumeStepChanged)

public:
    explicit GlobalConfig(QObject *parent = nullptr);
    ~GlobalConfig() override;

public Q_SLOTS:
    Q_INVOKABLE void setRaiseMaximumVolume(bool raise);
    Q_INVOKABLE bool raiseMaximumVolume();

    Q_INVOKABLE void setAudioFeedback(bool raise);
    Q_INVOKABLE bool audioFeedback();

    Q_INVOKABLE void setVolumeOsd(bool raise);
    Q_INVOKABLE bool volumeOsd();

    Q_INVOKABLE void setMicrophoneSensitivityOsd(bool raise);
    Q_INVOKABLE bool microphoneSensitivityOsd();

    Q_INVOKABLE void setMuteOsd(bool raise);
    Q_INVOKABLE bool muteOsd();

    Q_INVOKABLE void setDefaultOutputDeviceOsd(bool raise);
    Q_INVOKABLE bool defaultOutputDeviceOsd();

    Q_INVOKABLE void setVolumeStep(int step);
    Q_INVOKABLE int volumeStep();

Q_SIGNALS:
    void raiseMaximumVolumeChanged() const;
    void audioFeedbackChanged() const;
    void volumeOsdChanged() const;
    void microphoneSensitivityOsdChanged() const;
    void muteOsdChanged() const;
    void defaultOutputDeviceOsdChanged() const;
    void volumeStepChanged() const;

private:
    KConfigWatcher::Ptr m_configWatcher;
    bool m_raiseMaximumVolume;
    bool m_audioFeedback;
    bool m_volumeOsd;
    bool m_microphoneSensitivityOsd;
    bool m_muteOsd;
    bool m_defaultOutputDeviceOsd;
    int m_volumeStep;

private Q_SLOTS:
    void configChanged();
};

#endif
