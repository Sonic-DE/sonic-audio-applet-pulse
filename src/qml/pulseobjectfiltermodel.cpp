/*
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pulseobjectfiltermodel.h"

#include <PulseAudioQt/Client>
#include <PulseAudioQt/Context>
#include <PulseAudioQt/Device>
#include <PulseAudioQt/Models>
#include <PulseAudioQt/Port>
#include <PulseAudioQt/Profile>
#include <PulseAudioQt/Server>
#include <PulseAudioQt/Source>

using namespace PulseAudioQt;
using namespace Qt::Literals::StringLiterals;

PulseObjectFilter::PulseObjectFilter() = default;

PulseObjectFilter::PulseObjectFilter(const QString &role, const QVariant &value)
    : role(role)
    , value(value)
{
}

bool PulseObjectFilter::operator==(const PulseObjectFilter &other) const
{
    return role == other.role && value == other.value;
}

QDebug operator<<(QDebug debug, const PulseObjectFilter &filter)
{
    return debug.noquote() << "PulseObjectFilter(role=" << filter.role << ", value=" << filter.value << ")";
}

PulseObjectFilterModel::PulseObjectFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

PulseObjectFilterModel::~PulseObjectFilterModel() = default;

PulseObjectFilter PulseObjectFilterModel::filter(const QString &role, const QVariant &value)
{
    return PulseObjectFilter(role, value);
}

const QList<PulseObjectFilter> &PulseObjectFilterModel::filters()
{
    return m_filters;
}

void PulseObjectFilterModel::setFilters(const QList<PulseObjectFilter> &filters)
{
    qDebug() << "SET FILTERS" << filters;
    if (m_filters != filters) {
        m_filters = filters;
        invalidateFilter();
        Q_EMIT filtersChanged();
    }
}

bool PulseObjectFilterModel::filterOutInactiveDevices()
{
    return m_filterOutInactiveDevices;
}

void PulseObjectFilterModel::setFilterOutInactiveDevices(bool filter)
{
    if (m_filterOutInactiveDevices != filter) {
        m_filterOutInactiveDevices = filter;
        invalidateFilter();
        Q_EMIT filterOutInactiveDevicesChanged();
    }
}

bool PulseObjectFilterModel::filterVirtualDevices()
{
    return m_filterVirtualDevices;
}

void PulseObjectFilterModel::setFilterVirtualDevices(bool filter)
{
    if (m_filterVirtualDevices != filter) {
        m_filterVirtualDevices = filter;
        invalidateFilter();
        Q_EMIT filterVirtualDevicesChanged();
    }
}

bool PulseObjectFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto sourceModel = this->sourceModel();
    if (!sourceModel) {
        return false;
    }

    const auto idx = sourceModel->index(source_row, 0, source_parent);

    // Don't ever show the dummy output, that's silly
    const auto NameRole = sourceModel->roleNames().key(QByteArrayLiteral("Name"));
    if (sourceModel->data(idx, NameRole) == DUMMY_OUTPUT_NAME) {
        return false;
    }

    // Optionally run the role-based filters
    for (const auto &filter : std::as_const(m_filters)) {
        const auto FilterRole = sourceModel->roleNames().key(filter.role.toLatin1());
        if (sourceModel->data(idx, FilterRole) != filter.value) {
            return false;
        }
    }

    // Optionally exclude inactive devices
    if (m_filterOutInactiveDevices) {
        if (auto device = sourceModel->data(idx, AbstractModel::PulseObjectRole).value<Device *>()) {
            const auto ports = device->ports();
            if (ports.length() == 1 && ports[0]->availability() == Profile::Unavailable) {
                return false;
            }
        }
    }

    if (m_filterVirtualDevices) {
        if (auto device = sourceModel->data(idx, AbstractModel::PulseObjectRole).value<Device *>()) {
            return !device->isVirtualDevice();
        }
    }

    return true;
}

#include "moc_pulseobjectfiltermodel.cpp"
