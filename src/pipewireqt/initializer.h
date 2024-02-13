// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

#include "pipewireqt_export.h"

namespace PipeWireQt
{

class InitializerPrivate;

class PIPEWIREQT_EXPORT Initializer : public QObject
{
    Q_OBJECT
public:
    ~Initializer() override;
    Q_DISABLE_COPY_MOVE(Initializer)

    [[nodiscard]] static Initializer *instance();

private:
    explicit Initializer(QObject *parent = nullptr);
    std::unique_ptr<InitializerPrivate> d;
};

} // namespace PipeWireQt
