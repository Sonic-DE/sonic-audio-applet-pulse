// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>
// SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>

#pragma once

#include <QObject>

namespace PulseAudioQt
{
class Sink;
} // namespace PulseAudioQt

class PreferredStream : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PulseAudioQt::Sink *sink MEMBER m_sink NOTIFY sinkChanged)
public:
    explicit PreferredStream(QObject *parent = nullptr);
    PulseAudioQt::Sink *sink();

Q_SIGNALS:
    void sinkChanged();

private:
    void updatePreferredSink();
    [[nodiscard]] PulseAudioQt::Sink *findPreferredSink() const;
    PulseAudioQt::Sink *m_sink = nullptr;
};
