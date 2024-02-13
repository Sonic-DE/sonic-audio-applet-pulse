// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <pipewire/pipewire.h>

#include <memory>
#include <span>

#include <QDebug>

namespace std
{
template<>
struct default_delete<pw_registry> {
    void operator()(pw_registry *ptr) const
    {
        pw_proxy_destroy(reinterpret_cast<pw_proxy *>(ptr));
    }
};
} // namespace std

namespace PipeWireQt
{

class Registry
{
public:
    explicit Registry(pw_core *core);

    ~Registry()
    {
        qDebug() << Q_FUNC_INFO;
        spa_hook_remove(&m_registryListener);
    }

    void on_registry_global(uint32_t id, uint32_t permissions, const char *type, uint32_t version, const struct spa_dict *props)
    {
        qDebug() << Q_FUNC_INFO;
        qDebug() << id << permissions << type << version;
        const struct spa_dict_item *prop = nullptr;
        std::span{props->items, props->n_items};
        for (const auto &prop : std::span{props->items, props->n_items}) {
            qDebug() << prop.key << prop.value;
        }
        // for ((prop) = (props)->items; (prop) < &(props)->items[(props)->n_items]; (prop)++) {
        //     qDebug() << prop;
        // }

        if (spa_streq(type, PW_TYPE_INTERFACE_Node)) {
            auto node = pw_registry_bind(m_registry.get(), id, type, PW_VERSION_NODE, 0);
            if (auto klass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS); klass) {
                if (spa_streq(klass, "Audio/Sink")) {
                    if (auto nick = spa_dict_lookup(props, PW_KEY_NODE_NICK); nick) {
                        if (spa_streq(nick, "ALC1220 Analog")) {
                            qDebug() << "--------------------------------------------";
                        }
                    }
                }
            }
        }
    }

    std::unique_ptr<pw_registry> m_registry;
    spa_hook m_registryListener{};
};

namespace RegistryEvents
{
template<typename... Args>
void global(auto data, Args... args)
{
    static_cast<Registry *>(data)->on_registry_global(std::forward<Args>(args)...);
}
} // namespace RegistryEvents


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

class CorePrivate
{
public:
    explicit CorePrivate(pw_core *core)
        : m_core(core, pw_core_disconnect)
        , m_registry(m_core.get())
    {
    }

    std::unique_ptr<pw_core, decltype(&pw_core_disconnect)> m_core;
    Registry m_registry;
};

} // namespace PipeWireQt
