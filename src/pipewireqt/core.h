// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

#include "pipewireqt_export.h"

namespace PipeWireQt
{

class CorePrivate;

class PIPEWIREQT_EXPORT Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(std::unique_ptr<CorePrivate> d_, QObject *parent = nullptr);
    ~Core() override;
    Q_DISABLE_COPY_MOVE(Core)

private:
    std::unique_ptr<CorePrivate> d;
};

} // namespace PipeWireQt
