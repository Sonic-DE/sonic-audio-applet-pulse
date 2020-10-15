#include "streammeter.h"

#include <pulse/pulseaudio.h>

#include <QDebug>
#include "context.h"

using namespace QPulseAudio;

StreamMeter::StreamMeter(QObject* parent)
    : QObject(parent)
{
    Context::instance()->ref();
}

StreamMeter::~StreamMeter()
{
    if (m_stream) {
        pa_stream_unref(m_stream);
    }

    Context::instance()->unref();
}

void StreamMeter::updateVolume(qreal volume)
{
    m_volume = volume;
    Q_EMIT volumeChanged();
}

void StreamMeter::createStream()
{
    qDebug() << "CREATE" << m_sourceIdx << m_streamIdx;
    bool suspend = false;
    pa_stream *s;
    char t[16];
    pa_buffer_attr attr;
    pa_sample_spec ss;
    pa_stream_flags_t flags;

    ss.channels = 1;
    ss.format = PA_SAMPLE_FLOAT32;
    ss.rate = 25;

    memset(&attr, 0, sizeof(attr));
    attr.fragsize = sizeof(float);
    attr.maxlength = (uint32_t) -1;

    snprintf(t, sizeof(t), "%u", m_sourceIdx);

    if (!(s = pa_stream_new(Context::instance()->context(), "PlasmaPA-Internal-PeakDetect", &ss, nullptr))) {
//         show_error(tr("Failed to create monitoring stream").toUtf8().constData());
        return;
    }

    if (m_streamIdx != (uint32_t) -1) {
        pa_stream_set_monitor_stream(s, m_streamIdx);
    }

    pa_stream_set_read_callback(s, read_callback, this);
    pa_stream_set_suspended_callback(s, suspended_callback, this);

    flags = (pa_stream_flags_t) (PA_STREAM_DONT_MOVE | PA_STREAM_PEAK_DETECT | PA_STREAM_ADJUST_LATENCY |
                                 (suspend ? PA_STREAM_DONT_INHIBIT_AUTO_SUSPEND : PA_STREAM_NOFLAGS) | PA_STREAM_NOFLAGS);

    if (pa_stream_connect_record(s, t, &attr, flags) < 0) {
        pa_stream_unref(s);
        return;
    }
}

void StreamMeter::suspended_callback(pa_stream *s, void *userdata)
{
    qDebug() << "suspend callback";

    StreamMeter *w = static_cast<StreamMeter*>(userdata);
    if (pa_stream_is_suspended(s)) {
            w->updateVolume(-1);
    }
}

void StreamMeter::read_callback(pa_stream *s, size_t length, void *userdata) {
    StreamMeter *w = static_cast<StreamMeter*>(userdata);
    const void *data;
    double v;

    if (pa_stream_peek(s, &data, &length) < 0) {
        qDebug() << "Failed to read data from stream";
        return;
    }

    if (!data) {
        /* nullptr data means either a hole or empty buffer.
         * Only drop the stream when there is a hole (length > 0) */
        if (length)
            pa_stream_drop(s);
        return;
    }

    assert(length > 0);
    assert(length % sizeof(float) == 0);

    v = ((const float*) data)[length / sizeof(float) -1];

    pa_stream_drop(s);

    if (v < 0)
        v = 0;
    if (v > 1)
        v = 1;

    w->updateVolume(v);
}
