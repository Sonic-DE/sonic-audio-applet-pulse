/*
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QDebug>
#include <QList>
#include <QObject>
#include <QSortFilterProxyModel>

using namespace Qt::Literals::StringLiterals;

// DEFAULT_SINK_NAME in module-always-sink.c
constexpr QLatin1String DUMMY_OUTPUT_NAME = "auto_null"_L1;

struct PulseObjectFilter {
    Q_GADGET

    Q_PROPERTY(QString role MEMBER role)
    Q_PROPERTY(QVariant value MEMBER value)

public:
    explicit PulseObjectFilter();
    explicit PulseObjectFilter(const QString &role, const QVariant &value);

    bool operator==(const PulseObjectFilter &other) const;

private:
    friend class PulseObjectFilterModel;
    friend QDebug operator<<(QDebug debug, const PulseObjectFilter &filter);

    QString role;
    QVariant value;
};

class PulseObjectFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QList<PulseObjectFilter> filters READ filters WRITE setFilters NOTIFY filtersChanged)
    Q_PROPERTY(bool filterOutInactiveDevices READ filterOutInactiveDevices WRITE setFilterOutInactiveDevices NOTIFY filterOutInactiveDevicesChanged)
    Q_PROPERTY(bool filterVirtualDevices READ filterVirtualDevices WRITE setFilterVirtualDevices NOTIFY filterVirtualDevicesChanged)

public:
    explicit PulseObjectFilterModel(QObject *parent = nullptr);
    ~PulseObjectFilterModel() override;

    Q_INVOKABLE static PulseObjectFilter filter(const QString &role, const QVariant &value);

    const QList<PulseObjectFilter> &filters();
    void setFilters(const QList<PulseObjectFilter> &filters);

    bool filterOutInactiveDevices();
    void setFilterOutInactiveDevices(bool filter);

    bool filterVirtualDevices();
    void setFilterVirtualDevices(bool filter);

Q_SIGNALS:
    void filtersChanged();
    void filterOutInactiveDevicesChanged();
    void filterVirtualDevicesChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QList<PulseObjectFilter> m_filters;
    bool m_filterOutInactiveDevices = false;
    bool m_filterVirtualDevices = false;
};
