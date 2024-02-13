// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "threadloop.h"
#include "threadloop_p.h"

#include <pipewire/thread-loop.h>

namespace PipeWireQt
{

ThreadLoop::ThreadLoop(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<ThreadLoopPrivate>())
{
}

ThreadLoop::~ThreadLoop() = default;

} // namespace PipeWireQt
