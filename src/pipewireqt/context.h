// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

#include "pipewireqt_export.h"

namespace PipeWireQt
{

class ContextPrivate;

class PIPEWIREQT_EXPORT Context : public QObject
{
    Q_OBJECT
public:
    explicit Context(QObject *parent = nullptr);
    ~Context() override;
    Q_DISABLE_COPY_MOVE(Context)

private:
    std::unique_ptr<ContextPrivate> d;
};

} // namespace PipeWireQt
