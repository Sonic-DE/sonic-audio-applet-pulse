// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "core.h"
#include "core_p.h"

#include <pipewire/core.h>

namespace PipeWireQt
{

Registry::Registry(pw_core *core)
    : m_registry(pw_core_get_registry(core, PW_VERSION_REGISTRY, 0))
{
    qDebug() << Q_FUNC_INFO;
    static const struct pw_registry_events registryEvents = {
        .global = RegistryEvents::global,
    };
    spa_zero(m_registryListener);
    auto ret = pw_registry_add_listener(m_registry.get(), &m_registryListener, &registryEvents, this);
    Q_ASSERT(ret >= 0);
}

Core::Core(std::unique_ptr<CorePrivate> d_, QObject *parent)
    : QObject(parent)
    , d(std::move(d_))
{
}

Core::~Core() = default;

} // namespace PipeWireQt
