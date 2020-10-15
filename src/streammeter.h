#pragma once

#include <QObject>

struct pa_stream;

class StreamMeter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint sourceIndex MEMBER m_sourceIdx )
    Q_PROPERTY(uint streamIndex MEMBER m_streamIdx )

    Q_PROPERTY(qreal volume MEMBER m_volume NOTIFY volumeChanged)

public:
    StreamMeter(QObject *parent=nullptr);
    ~StreamMeter();

Q_SIGNALS:
        void volumeChanged();

public Q_SLOTS:
    void createStream();
private:
    void updateVolume(qreal volume);
    static void read_callback(pa_stream *s, size_t length, void *userdata);
    static void suspended_callback(pa_stream *s, void *userdata);

    uint32_t m_sourceIdx = -1;
    uint32_t m_streamIdx = -1;
    pa_stream *m_stream = nullptr;

    qreal m_volume;
};
