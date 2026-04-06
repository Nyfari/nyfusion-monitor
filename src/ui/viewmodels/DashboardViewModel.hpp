#pragma once
/**
 * @file DashboardViewModel.hpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#ifndef NY_FUSION_MONITOR_DASHBOARDVIEWMODEL_HPP
#define NY_FUSION_MONITOR_DASHBOARDVIEWMODEL_HPP

#include <memory>
#include <optional>
#include <cstdint>

#include <QObject>
#include <QFutureWatcher>
#include <QList>
#include <QTimer>
#include <QString>

#include "services/HardwareBackendService.hpp"

namespace ny::ui::viewmodels {

    class DashboardViewModel final : public QObject {
        Q_OBJECT

    public:
        struct CpuViewData final {
            QString name;
            QString frequency;
            QString usage;
            QString temperature;
            QString fanRpm;
        };

        struct GpuViewData final {
            QString name;
            QString vendor;
            QString usage;
            QString temperature;
            QString frequency;
            QString power;
            QString vramTotal;
            QString vramUsed;
            QString fanRpm;
        };

        struct MemoryViewData final {
            QString usage;
            QString available;
            QString total;
            QString swap;
        };

        explicit DashboardViewModel(QObject* parent = nullptr);
        ~DashboardViewModel() override;

        [[nodiscard]] CpuViewData cpuData() const;
        [[nodiscard]] MemoryViewData memoryData() const;
        [[nodiscard]] QString statusText() const;
        [[nodiscard]] const QList<GpuViewData>& gpusData() const;

    signals:
        void dashboardUpdated();

    private:
        struct WorkerResult final {
            std::optional<ny::ui::services::HardwareBackendService::DashboardSnapshot> snapshot;
            std::string error;
        };

        static QString formatOptionalFloat(
            const std::optional<float>& value,
            const QString& unit,
            int decimals = 1
        );

        static QString formatOptionalUInt64(
            const std::optional<std::uint64_t>& value,
            const QString& unit
        );

        void triggerRefresh();
        void applyWorkerResult(const WorkerResult& result);

        std::unique_ptr<ny::ui::services::HardwareBackendService> m_backendService;
        QTimer m_refreshTimer;
        QFutureWatcher<WorkerResult> m_watcher;
        bool m_refreshInFlight{false};

        CpuViewData m_cpuData{};
        MemoryViewData m_memoryData{};
        QList<GpuViewData> m_gpuData{};
        QString m_statusText{"Inicializando..."};
    };

} // namespace ny::ui::viewmodels

#endif // NY_FUSION_MONITOR_DASHBOARDVIEWMODEL_HPP
