// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <pipewire/core.h>

#warning fixme temporary for node debugging
#include <pipewire/pipewire.h>
#include <spa/param/props.h>
#include <spa/param/audio/raw.h>
#include <spa/pod/builder.h>

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
        for (const auto &prop : std::span{props->items, props->n_items}) {
            qDebug() << prop.key << prop.value;
        }

        if (spa_streq(type, PW_TYPE_INTERFACE_Node)) {
            auto node = pw_registry_bind(m_registry.get(), id, type, PW_VERSION_NODE, 0);
            if (auto klass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS); klass) {
                if (spa_streq(klass, "Audio/Sink")) {
                    if (auto nick = spa_dict_lookup(props, PW_KEY_NODE_NICK); nick) {
                        if (spa_streq(nick, "ALC1220 Analog")) {
                            qDebug() << "--------------------------------------------";

                            {
                                constexpr auto n_channels = 6;

                                std::array<float, SPA_AUDIO_MAX_CHANNELS> volumes{};
                                std::ranges::fill(volumes, 0.0);
                                std::fill_n(volumes.begin(), n_channels, .15);

                                std::array<char, 1024> buffer{};
                                auto builder = SPA_POD_BUILDER_INIT(buffer.data(), buffer.size());
                                auto pod = spa_pod_builder_add_object(&builder,
                                                        SPA_TYPE_OBJECT_Props,
                                                        SPA_PARAM_Props,
                                                        SPA_PROP_channelVolumes,
                                                        SPA_POD_Array(sizeof(decltype(volumes)::value_type), SPA_TYPE_Float, n_channels, volumes.data()));

                                auto ret = pw_node_set_param(node, SPA_PARAM_Props, 0, static_cast<spa_pod *>(pod));
                            }

                            {
                                std::array<char, 1024> buffer{};
                                auto builder = SPA_POD_BUILDER_INIT(buffer.data(), buffer.size());
                                auto pod = spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props, SPA_PROP_volume, SPA_POD_Float(1.0));
                                auto ret = pw_node_set_param(node, SPA_PARAM_Props, 0, static_cast<spa_pod *>(pod));
                            }
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
