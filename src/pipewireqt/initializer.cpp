// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "initializer.h"

#include <pipewire/pipewire.h>

namespace PipeWireQt
{

class InitializerPrivate
{
};

Initializer::Initializer(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<InitializerPrivate>())
{
    pw_init(nullptr, nullptr);
}

Initializer::~Initializer()
{
    pw_deinit();
}

Initializer *Initializer::instance()
{
    static Initializer init;
    return &init;
}
} // namespace PipeWireQt
