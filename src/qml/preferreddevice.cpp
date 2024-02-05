// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
// SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>

#include "preferreddevice.h"

#include "context.h"
#include "debug.h"
#include "server.h"
#include "sink.h"

PreferredDevice::PreferredDevice(QObject *parent)
    : QObject(parent)
{
    connect(PulseAudioQt::Context::instance(), &PulseAudioQt::Context::sinkAdded, this, [this](PulseAudioQt::Sink *sink) {
        connect(sink, &PulseAudioQt::Sink::stateChanged, this, &PreferredDevice::updatePreferredSink, Qt::UniqueConnection);
        updatePreferredSink();
    });
    connect(PulseAudioQt::Context::instance(), &PulseAudioQt::Context::sinkRemoved, this, &PreferredDevice::updatePreferredSink);
    connect(PulseAudioQt::Context::instance()->server(), &PulseAudioQt::Server::defaultSinkChanged, this, &PreferredDevice::updatePreferredSink);
}

PulseAudioQt::Sink *PreferredDevice::sink()
{
    return m_sink;
}

void PreferredDevice::updatePreferredSink()
{
    auto sink = findPreferredSink();

    if (sink != m_sink) {
        qCDebug(PLASMAPA) << "Changing preferred sink to" << sink << (sink ? sink->name() : "");
        m_sink = sink;
        Q_EMIT sinkChanged();
    }
}

[[nodiscard]] PulseAudioQt::Sink *PreferredDevice::findPreferredSink() const
{
    const auto sinks = PulseAudioQt::Context::instance()->sinks();

    // Only one sink is the preferred one
    if (sinks.count() == 1) {
        return sinks.at(0);
    }

    const auto defaultSink = PulseAudioQt::Context::instance()->server()->defaultSink();

    auto lookForState = [&](PulseAudioQt::Device::State state) {
        PulseAudioQt::Sink *ret = nullptr;
        for (const auto &sink : sinks) {
            if (sink->state() != state || (sink->isVirtualDevice() && !sink->isDefault())) {
                continue;
            }
            if (!ret) {
                ret = sink;
            } else if (sink == defaultSink) {
                ret = sink;
                break;
            }
        }
        return ret;
    };

    PulseAudioQt::Sink *preferred = nullptr;

    // Look for playing sinks + prefer default sink
    preferred = lookForState(PulseAudioQt::Device::RunningState);
    if (preferred) {
        return preferred;
    }

    // Look for idle sinks + prefer default sink
    preferred = lookForState(PulseAudioQt::Device::IdleState);
    if (preferred) {
        return preferred;
    }

    // Fallback to default sink
    return defaultSink;
}
