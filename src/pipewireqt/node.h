// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

#include "pipewireqt_export.h"

namespace PipeWireQt
{

class NodePrivate;

class PIPEWIREQT_EXPORT Node : public QObject
{
    Q_OBJECT
public:
    explicit Node(QObject *parent = nullptr);
    ~Node() override;
    Q_DISABLE_COPY_MOVE(Node)

private:
    std::unique_ptr<NodePrivate> d;
};

} // namespace PipeWireQt
