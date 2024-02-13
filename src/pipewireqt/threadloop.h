// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

#include "pipewireqt_export.h"

namespace PipeWireQt
{

class ThreadLoopPrivate;

class PIPEWIREQT_EXPORT ThreadLoop : public QObject
{
    Q_OBJECT
public:
    explicit ThreadLoop(QObject *parent = nullptr);
    ~ThreadLoop() override;
    Q_DISABLE_COPY_MOVE(ThreadLoop)

private:
    std::unique_ptr<ThreadLoopPrivate> d;
    friend class ContextPrivate;
};

} // namespace PipeWireQt
