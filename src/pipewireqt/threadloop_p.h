// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <pipewire/thread-loop.h>

#include "initializer.h"

namespace PipeWireQt
{

class ThreadLoopPrivate
{
public:
    ThreadLoopPrivate()
        : m_loop(makeThreadLoop(), pw_thread_loop_destroy)
    {
        pw_thread_loop_start(m_loop.get());
        pw_thread_loop_lock(m_loop.get());
    }

    pw_thread_loop *makeThreadLoop()
    {
        std::ignore = Initializer::instance();
        auto loop = pw_thread_loop_new("PipeWireQt", nullptr);
        Q_ASSERT(loop);
        return loop;
    }

    std::unique_ptr<pw_thread_loop, decltype(&pw_thread_loop_destroy)> m_loop;
};

} // namespace PipeWireQt
