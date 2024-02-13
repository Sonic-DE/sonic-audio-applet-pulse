// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "context.h"

#include <pipewire/pipewire.h>

#include "core.h"
#include "core_p.h"
#include "threadloop.h"
#include "threadloop_p.h"

namespace PipeWireQt
{

class ContextPrivate
{
public:
    ContextPrivate()
        : m_context(makeContext(), pw_context_destroy)
        , m_core(std::make_unique<CorePrivate>(pw_context_connect(m_context.get(), nullptr, 0)))
    {
        pw_thread_loop_unlock(m_threadLoop.d->m_loop.get());
    }

    ~ContextPrivate()
    {
        pw_thread_loop_lock(m_threadLoop.d->m_loop.get());
        pw_thread_loop_stop(m_threadLoop.d->m_loop.get());
    }

    pw_context *makeContext()
    {
        std::ignore = Initializer::instance();

        auto dict_items = std::to_array<spa_dict_item>({
            {PW_KEY_MEDIA_TYPE, "Audio"},
            {PW_KEY_MEDIA_CATEGORY, "Manager"},
        });
        auto dict = SPA_DICT_INIT(dict_items.data(), dict_items.size());
        auto properties = pw_properties_new_dict(&dict);

        return pw_context_new(pw_thread_loop_get_loop(m_threadLoop.d->m_loop.get()), properties, 0);
    }

    ThreadLoop m_threadLoop;
    std::unique_ptr<pw_context, decltype(&pw_context_destroy)> m_context;
    Core m_core;
};

Context::Context(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<ContextPrivate>())
{
}

Context::~Context() = default;

} // namespace PipeWireQt
