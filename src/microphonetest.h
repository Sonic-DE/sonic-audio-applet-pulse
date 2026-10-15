/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QTimer>
#include <QByteArray>
#include <QPointer>
#include <pulse/introspect.h>
#include <pulse/stream.h>
#include <PulseAudioQt/Source>

class MicrophoneTest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double volumeLevel READ volumeLevel NOTIFY volumeLevelChanged)
    Q_PROPERTY(PulseAudioQt::Source *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool hasRecording READ hasRecording NOTIFY hasRecordingChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)

public:
    explicit MicrophoneTest(QObject *parent = nullptr);
    ~MicrophoneTest();

    double volumeLevel() const;
    PulseAudioQt::Source *source() const;
    bool hasRecording() const;
    bool playing() const;
    bool recording() const;

public Q_SLOTS:
    void setSource(PulseAudioQt::Source *source);
    void startRecording();
    void stopRecording();
    void stopPlaying();
    void playRecording();
    void clearRecording();
    void cleanupStreams();

Q_SIGNALS:
    void volumeLevelChanged();
    void sourceChanged();
    void hasRecordingChanged();
    void showErrorMessage(const QString &message);
    void recordingFinished();
    void playbackFinished();
    void playingChanged();
    void recordingChanged();

private:
    friend class MicrophoneLifecycleTest;

    void calculateVolumeLevel(const void *data, size_t nbytes);

    void beginRecording(uint32_t sampleRate);
    void cancelSourceInfoOperation();

    static void stream_state_callback(pa_stream *s, void *userdata);
    static void stream_read_callback(pa_stream *s, size_t nbytes, void *userdata);
    static void stream_write_callback(pa_stream *s, size_t nbytes, void *userdata);
    static void source_info_callback(pa_context *c, const pa_source_info *i, int eol, void *userdata);

    QPointer<PulseAudioQt::Source> m_source;
    pa_operation *m_sourceInfoOperation = nullptr;
    pa_stream *m_recordStream = nullptr;
    pa_stream *m_playbackStream = nullptr;

    uint32_t m_sampleRate = 44100;
    uint32_t m_pendingSampleRate = 44100;

    QByteArray m_recordedData;
    size_t m_playbackOffset = 0;

    QTimer m_autoStopTimer;

    double m_volumeLevel = 0.0;
    bool m_manualStop = false;
    bool m_stopping = false;
    bool m_playing = false;
    bool m_recording = false;
};
