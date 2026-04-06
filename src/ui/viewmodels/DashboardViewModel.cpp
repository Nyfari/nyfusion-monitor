/**
 * @file DashboardViewModel.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */

#include "DashboardViewModel.hpp"

#include <QtConcurrent/QtConcurrentRun>
#include <QDateTime>

namespace ny::ui::viewmodels {
    namespace {
        QString formatBytesToGiB(const std::uint64_t bytes) {
            constexpr double oneGiB = 1024.0 * 1024.0 * 1024.0;
            const double gib = static_cast<double>(bytes) / oneGiB;
            return QString("%1 GB").arg(QString::number(gib, 'f', 1));
        }
    } // namespace

    DashboardViewModel::DashboardViewModel(QObject* parent)
        : QObject(parent)
        , m_backendService(std::make_unique<ny::ui::services::HardwareBackendService>())
    {
        connect(&m_refreshTimer, &QTimer::timeout, this, &DashboardViewModel::triggerRefresh);
        connect(&m_watcher, &QFutureWatcher<WorkerResult>::finished, this, [this]() {
            m_refreshInFlight = false;
            applyWorkerResult(m_watcher.result());
            emit dashboardUpdated();
        });

        m_refreshTimer.setInterval(1000);
        m_refreshTimer.start();

        m_memoryData.usage = QStringLiteral("Indisponivel");
        m_memoryData.available = QStringLiteral("Indisponivel");
        m_memoryData.total = QStringLiteral("Indisponivel");
        m_memoryData.swap = QStringLiteral("Indisponivel");
        triggerRefresh();
    }

    DashboardViewModel::~DashboardViewModel() {
        if (m_watcher.isRunning()) {
            m_watcher.waitForFinished();
        }
    }

    DashboardViewModel::CpuViewData DashboardViewModel::cpuData() const {
        return m_cpuData;
    }

    DashboardViewModel::MemoryViewData DashboardViewModel::memoryData() const {
        return m_memoryData;
    }

    QString DashboardViewModel::statusText() const {
        return m_statusText;
    }

    const QList<DashboardViewModel::GpuViewData>& DashboardViewModel::gpusData() const {
        return m_gpuData;
    }

    QString DashboardViewModel::formatOptionalFloat(
        const std::optional<float>& value,
        const QString& unit,
        const int decimals
    ) {
        if (!value.has_value()) {
            return QStringLiteral("Indisponivel");
        }
        return QString("%1 %2").arg(QString::number(value.value(), 'f', decimals), unit);
    }

    QString DashboardViewModel::formatOptionalUInt64(
        const std::optional<std::uint64_t>& value,
        const QString& unit
    ) {
        if (!value.has_value()) {
            return QStringLiteral("Indisponivel");
        }
        return QString("%1 %2").arg(QString::number(static_cast<qulonglong>(value.value())), unit);
    }

    void DashboardViewModel::triggerRefresh() {
        if (m_refreshInFlight) {
            return;
        }

        m_refreshInFlight = true;
        auto* backend = m_backendService.get();

        auto future = QtConcurrent::run([backend]() -> WorkerResult {
            WorkerResult result{};
            result.snapshot = backend->readSnapshot();
            result.error = backend->lastError();
            return result;
        });
        m_watcher.setFuture(future);
    }

    void DashboardViewModel::applyWorkerResult(const WorkerResult& result) {
        if (!result.snapshot.has_value()) {
            m_statusText = QString("Erro: %1").arg(QString::fromStdString(result.error));
            return;
        }

        const auto& cpu = result.snapshot->cpu;
        m_cpuData.name = cpu.name.empty() ? QStringLiteral("Indisponivel") : QString::fromStdString(cpu.name);
        m_cpuData.frequency = QString("%1 MHz").arg(QString::number(cpu.averageFrequencyMHz, 'f', 0));
        m_cpuData.usage = QString("%1 %").arg(QString::number(cpu.usagePercent, 'f', 1));
        m_cpuData.temperature = cpu.temperatureCelsius > 0.0
            ? QString("%1 C").arg(QString::number(cpu.temperatureCelsius, 'f', 1))
            : QStringLiteral("Indisponivel");
        m_cpuData.fanRpm = QStringLiteral("Indisponivel");

        const auto& memory = result.snapshot->memory;
        m_memoryData.usage = QString("%1 %").arg(QString::number(memory.usagePercent(), 'f', 1));
        m_memoryData.available = memory.freeBytes() > 0
            ? formatBytesToGiB(memory.freeBytes())
            : QStringLiteral("Indisponivel");
        m_memoryData.total = memory.totalBytes() > 0
            ? formatBytesToGiB(memory.totalBytes())
            : QStringLiteral("Indisponivel");
        m_memoryData.swap = QStringLiteral("Indisponivel");

        m_gpuData.clear();
        const auto& gpu = result.snapshot->gpu;
        if (!gpu.model.empty() || !gpu.vendor.empty()) {
            GpuViewData gpuView{};
            gpuView.name = gpu.model.empty() ? QStringLiteral("Indisponivel") : QString::fromStdString(gpu.model);
            gpuView.vendor = gpu.vendor.empty() ? QStringLiteral("Indisponivel") : QString::fromStdString(gpu.vendor);
            gpuView.usage = formatOptionalFloat(gpu.utilizationPercent, "%", 1);
            gpuView.temperature = formatOptionalFloat(gpu.temperatureCelsius, "C", 1);
            if (gpu.frequencyMHz.has_value()) {
                gpuView.frequency = QString("%1 MHz").arg(QString::number(gpu.frequencyMHz.value()));
            } else {
                gpuView.frequency = QStringLiteral("Indisponivel");
            }
            gpuView.power = formatOptionalFloat(gpu.powerWatts, "W", 1);
            gpuView.vramTotal = gpu.vramTotalMB > 0
                ? QString("%1 MB (%2 GB)")
                    .arg(QString::number(gpu.vramTotalMB))
                    .arg(QString::number(gpu.vramTotalGB))
                : QStringLiteral("Indisponivel");
            gpuView.vramUsed = formatOptionalUInt64(gpu.vramUsedMB, "MB");
            gpuView.fanRpm = QStringLiteral("Indisponivel");
            m_gpuData.push_back(gpuView);
        }

        if (m_gpuData.isEmpty()) {
            GpuViewData unavailableGpu{};
            unavailableGpu.name = QStringLiteral("Nenhuma GPU detectada");
            unavailableGpu.vendor = QStringLiteral("Indisponivel");
            unavailableGpu.usage = QStringLiteral("Indisponivel");
            unavailableGpu.temperature = QStringLiteral("Indisponivel");
            unavailableGpu.frequency = QStringLiteral("Indisponivel");
            unavailableGpu.power = QStringLiteral("Indisponivel");
            unavailableGpu.vramTotal = QStringLiteral("Indisponivel");
            unavailableGpu.vramUsed = QStringLiteral("Indisponivel");
            unavailableGpu.fanRpm = QStringLiteral("Indisponivel");
            m_gpuData.push_back(unavailableGpu);
        }
    }

} // namespace ny::ui::viewmodels
