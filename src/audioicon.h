/*
    SPDX-FileCopyrightText: 2023 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef AUDIOICON_H
#define AUDIOICON_H

#include <QObject>
#include <QString>

class AudioIcon : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(int HIGH_UPPER_BOUND MEMBER HIGH_UPPER_BOUND CONSTANT)
    Q_PROPERTY(int VERY_HIGH_UPPER_BOUND MEMBER VERY_HIGH_UPPER_BOUND CONSTANT)
    static const int HIGH_UPPER_BOUND = 100;
    static const int VERY_HIGH_UPPER_BOUND = 125;

public Q_SLOTS:
    static QString forVolume(int volume, bool muted, QString prefix);
    static QString forFormFactor(QString formFactor);
};

#endif // AUDIOICON_H
