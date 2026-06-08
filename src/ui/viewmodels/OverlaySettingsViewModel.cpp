/**
 * @file OverlaySettingsViewModel.cpp
 * @author Marcos Henrique
 * @date 16/05/2026
 */
#include "OverlaySettingsViewModel.hpp"

#include <algorithm>

#include <QVariantMap>
#include <QSettings>
#include <QtGlobal>

namespace ny::ui::viewmodels {
namespace {
QStringList sanitizeMetricIds(QStringList metricIds) {
    const auto definitions = OverlaySettingsViewModel::availableMetrics();

    QStringList sanitized;
    sanitized.reserve(metricIds.size());

    for (const QString& metricId : metricIds) {
        const bool exists = std::any_of(
            definitions.cbegin(),
            definitions.cend(),
            [&metricId](const OverlaySettingsViewModel::MetricDefinition& definition) {
                return definition.id == metricId;
            }
        );

        if (exists && !sanitized.contains(metricId)) {
            sanitized.push_back(metricId);
        }
    }

    return sanitized;
}

QHash<QString, OverlaySettingsViewModel::MetricDisplayType> sanitizeMetricDisplayTypes(const QVariantMap& rawTypes) {
    QHash<QString, OverlaySettingsViewModel::MetricDisplayType> result;

    const auto definitions = OverlaySettingsViewModel::availableMetrics();
    for (const auto& definition : definitions) {
        const int rawValue = rawTypes.value(definition.id, static_cast<int>(definition.defaultDisplayType)).toInt();
        switch (rawValue) {
        case static_cast<int>(OverlaySettingsViewModel::MetricDisplayType::NumberOnly):
        case static_cast<int>(OverlaySettingsViewModel::MetricDisplayType::ChartOnly):
        case static_cast<int>(OverlaySettingsViewModel::MetricDisplayType::NumberAndChart):
            result.insert(definition.id, static_cast<OverlaySettingsViewModel::MetricDisplayType>(rawValue));
            break;
        default:
            result.insert(definition.id, definition.defaultDisplayType);
            break;
        }
    }

    return result;
}
} // namespace

OverlaySettingsViewModel::OverlaySettingsViewModel(QObject* parent)
    : QObject(parent)
{
    load();
}

bool OverlaySettingsViewModel::overlayEnabled() const {
    return m_overlayEnabled;
}

bool OverlaySettingsViewModel::autoEnableOnGameLaunch() const {
    return m_autoEnableOnGameLaunch;
}

bool OverlaySettingsViewModel::showLabels() const {
    return m_showLabels;
}

int OverlaySettingsViewModel::backgroundOpacity() const {
    return m_backgroundOpacity;
}

OverlaySettingsViewModel::DisplayMode OverlaySettingsViewModel::displayMode() const {
    return m_displayMode;
}

OverlaySettingsViewModel::OverlayPosition OverlaySettingsViewModel::overlayPosition() const {
    return m_overlayPosition;
}

QStringList OverlaySettingsViewModel::selectedMetricIds() const {
    return m_selectedMetricIds;
}

OverlaySettingsViewModel::MetricDisplayType OverlaySettingsViewModel::metricDisplayType(const QString& metricId) const {
    return m_metricDisplayTypes.value(metricId, defaultMetricDisplayType(metricId));
}

void OverlaySettingsViewModel::setOverlayEnabled(const bool enabled) {
    if (m_overlayEnabled == enabled) {
        return;
    }

    m_overlayEnabled = enabled;
    emit settingsChanged();
}

void OverlaySettingsViewModel::setAutoEnableOnGameLaunch(const bool enabled) {
    if (m_autoEnableOnGameLaunch == enabled) {
        return;
    }

    m_autoEnableOnGameLaunch = enabled;
    emit settingsChanged();
}

void OverlaySettingsViewModel::setShowLabels(const bool enabled) {
    if (m_showLabels == enabled) {
        return;
    }

    m_showLabels = enabled;
    emit settingsChanged();
}

void OverlaySettingsViewModel::setBackgroundOpacity(const int opacity) {
    const int boundedOpacity = qBound(35, opacity, 100);
    if (m_backgroundOpacity == boundedOpacity) {
        return;
    }

    m_backgroundOpacity = boundedOpacity;
    emit settingsChanged();
}

void OverlaySettingsViewModel::setDisplayMode(const DisplayMode mode) {
    if (m_displayMode == mode) {
        return;
    }

    m_displayMode = mode;
    emit settingsChanged();
}

void OverlaySettingsViewModel::setOverlayPosition(const OverlayPosition position) {
    if (m_overlayPosition == position) {
        return;
    }

    m_overlayPosition = position;
    emit settingsChanged();
}

void OverlaySettingsViewModel::setSelectedMetricIds(const QStringList& metricIds) {
    const QStringList sanitizedMetricIds = sanitizeMetricIds(metricIds);
    if (m_selectedMetricIds == sanitizedMetricIds) {
        return;
    }

    m_selectedMetricIds = sanitizedMetricIds;
    emit settingsChanged();
}

void OverlaySettingsViewModel::setMetricSelected(const QString& metricId, const bool selected) {
    QStringList updatedMetricIds = m_selectedMetricIds;
    updatedMetricIds.removeAll(metricId);

    if (selected) {
        updatedMetricIds.push_back(metricId);
    }

    setSelectedMetricIds(updatedMetricIds);
}

void OverlaySettingsViewModel::setMetricDisplayType(const QString& metricId, const MetricDisplayType displayType) {
    const auto definitions = availableMetrics();
    const bool metricExists = std::any_of(
        definitions.cbegin(),
        definitions.cend(),
        [&metricId](const MetricDefinition& definition) {
            return definition.id == metricId;
        }
    );
    if (!metricExists || metricDisplayType(metricId) == displayType) {
        return;
    }

    m_metricDisplayTypes.insert(metricId, displayType);
    emit settingsChanged();
}

void OverlaySettingsViewModel::load() {
    const QStringList defaultMetricIds = defaultSelectedMetricIds();
    QSettings settings(QStringLiteral("Nyfari"), QStringLiteral("NyFusionMonitor"));
    settings.beginGroup(QStringLiteral("overlay"));

    m_overlayEnabled = settings.value(QStringLiteral("enabled"), false).toBool();
    m_autoEnableOnGameLaunch = settings.value(QStringLiteral("autoEnableOnGameLaunch"), false).toBool();
    m_showLabels = settings.value(QStringLiteral("showLabels"), true).toBool();
    m_backgroundOpacity = qBound(35, settings.value(QStringLiteral("backgroundOpacity"), 78).toInt(), 100);

    const int displayModeRaw = settings.value(
        QStringLiteral("displayMode"),
        static_cast<int>(DisplayMode::CompactCards)
    ).toInt();
    switch (displayModeRaw) {
    case static_cast<int>(DisplayMode::CompactCards):
    case static_cast<int>(DisplayMode::NumbersOnly):
    case static_cast<int>(DisplayMode::NumbersAndPercentages):
    case static_cast<int>(DisplayMode::MiniCharts):
        m_displayMode = static_cast<DisplayMode>(displayModeRaw);
        break;
    default:
        m_displayMode = DisplayMode::CompactCards;
        break;
    }

    const int overlayPositionRaw = settings.value(
        QStringLiteral("position"),
        static_cast<int>(OverlayPosition::TopRight)
    ).toInt();
    switch (overlayPositionRaw) {
    case static_cast<int>(OverlayPosition::TopLeft):
    case static_cast<int>(OverlayPosition::TopRight):
    case static_cast<int>(OverlayPosition::BottomLeft):
    case static_cast<int>(OverlayPosition::BottomRight):
        m_overlayPosition = static_cast<OverlayPosition>(overlayPositionRaw);
        break;
    default:
        m_overlayPosition = OverlayPosition::TopRight;
        break;
    }

    m_selectedMetricIds = sanitizeMetricIds(
        settings.value(QStringLiteral("selectedMetrics"), defaultMetricIds).toStringList()
    );
    if (m_selectedMetricIds.isEmpty()) {
        m_selectedMetricIds = defaultMetricIds;
    }

    m_metricDisplayTypes = sanitizeMetricDisplayTypes(
        settings.value(QStringLiteral("metricDisplayTypes")).toMap()
    );

    settings.endGroup();
    emit settingsChanged();
}

void OverlaySettingsViewModel::save() const {
    QSettings settings(QStringLiteral("Nyfari"), QStringLiteral("NyFusionMonitor"));
    settings.beginGroup(QStringLiteral("overlay"));
    settings.setValue(QStringLiteral("enabled"), m_overlayEnabled);
    settings.setValue(QStringLiteral("autoEnableOnGameLaunch"), m_autoEnableOnGameLaunch);
    settings.setValue(QStringLiteral("showLabels"), m_showLabels);
    settings.setValue(QStringLiteral("backgroundOpacity"), m_backgroundOpacity);
    settings.setValue(QStringLiteral("displayMode"), static_cast<int>(m_displayMode));
    settings.setValue(QStringLiteral("position"), static_cast<int>(m_overlayPosition));
    settings.setValue(QStringLiteral("selectedMetrics"), m_selectedMetricIds);

    QVariantMap displayTypes;
    for (auto it = m_metricDisplayTypes.cbegin(); it != m_metricDisplayTypes.cend(); ++it) {
        displayTypes.insert(it.key(), static_cast<int>(it.value()));
    }
    settings.setValue(QStringLiteral("metricDisplayTypes"), displayTypes);
    settings.endGroup();
}

void OverlaySettingsViewModel::restoreDefaults() {
    m_overlayEnabled = false;
    m_autoEnableOnGameLaunch = false;
    m_showLabels = true;
    m_backgroundOpacity = 78;
    m_displayMode = DisplayMode::CompactCards;
    m_overlayPosition = OverlayPosition::TopRight;
    m_selectedMetricIds = defaultSelectedMetricIds();
    m_metricDisplayTypes = sanitizeMetricDisplayTypes(QVariantMap{});
    emit settingsChanged();
}

bool OverlaySettingsViewModel::isMetricSelected(const QString& metricId) const {
    return m_selectedMetricIds.contains(metricId);
}

QList<OverlaySettingsViewModel::MetricDefinition> OverlaySettingsViewModel::availableMetrics() {
    return {
        { QStringLiteral("cpu_usage"),     QStringLiteral("Uso da CPU"),         QStringLiteral("Carga total do processador"),              QStringLiteral("67%"),                   MetricDisplayType::NumberAndChart },
        { QStringLiteral("cpu_temp"),      QStringLiteral("Temperatura da CPU"),  QStringLiteral("Sensor térmico principal"),                QStringLiteral("71 °C"),                 MetricDisplayType::NumberAndChart },
        { QStringLiteral("cpu_clock"),     QStringLiteral("Clock da CPU"),        QStringLiteral("Frequência média atual"),                  QStringLiteral("4.8 GHz"),               MetricDisplayType::NumberAndChart },
        { QStringLiteral("gpu_usage"),     QStringLiteral("Uso da GPU"),          QStringLiteral("Ocupação do chip gráfico"),                QStringLiteral("93%"),                   MetricDisplayType::NumberAndChart },
        { QStringLiteral("gpu_temp"),      QStringLiteral("Temperatura da GPU"),  QStringLiteral("Sensor térmico da placa"),                 QStringLiteral("68 °C"),                 MetricDisplayType::NumberAndChart },
        { QStringLiteral("gpu_clock"),     QStringLiteral("Clock da GPU"),        QStringLiteral("Frequência atual da GPU"),                 QStringLiteral("2450 MHz"),              MetricDisplayType::NumberAndChart },
        { QStringLiteral("ram_usage"),     QStringLiteral("Uso de RAM"),          QStringLiteral("Consumo atual de memória"),                QStringLiteral("12.4 GB"),               MetricDisplayType::NumberOnly     },
        { QStringLiteral("ram_available"), QStringLiteral("RAM disponível"),      QStringLiteral("Memória livre para novos processos"),      QStringLiteral("19.6 GB"),               MetricDisplayType::NumberOnly     },
        // ── Métricas de frame (requerem jogo detectado) ────────────────────────
        { QStringLiteral("fps"),           QStringLiteral("FPS"),                 QStringLiteral("Quadros por segundo (estimado via GPU)"),  QStringLiteral("~144 FPS"),              MetricDisplayType::NumberAndChart },
        { QStringLiteral("frame_time"),    QStringLiteral("Frame Time"),          QStringLiteral("Tempo entre quadros em milissegundos"),    QStringLiteral("~6.9 ms"),               MetricDisplayType::NumberAndChart },
        { QStringLiteral("graphics_api"),  QStringLiteral("Motor Gráfico"),       QStringLiteral("API e motor de renderização detectados"),  QStringLiteral("Vulkan"),                MetricDisplayType::NumberOnly     },
    };
}

QString OverlaySettingsViewModel::displayModeLabel(const DisplayMode mode) {
    switch (mode) {
    case DisplayMode::CompactCards:
        return QStringLiteral("Compacto");
    case DisplayMode::NumbersOnly:
        return QStringLiteral("Somente números");
    case DisplayMode::NumbersAndPercentages:
        return QStringLiteral("Números e porcentagens");
    case DisplayMode::MiniCharts:
        return QStringLiteral("Mini gráficos");
    }

    return QStringLiteral("Compacto");
}

QString OverlaySettingsViewModel::overlayPositionLabel(const OverlayPosition position) {
    switch (position) {
    case OverlayPosition::TopLeft:
        return QStringLiteral("Topo esquerdo");
    case OverlayPosition::TopRight:
        return QStringLiteral("Topo direito");
    case OverlayPosition::BottomLeft:
        return QStringLiteral("Inferior esquerdo");
    case OverlayPosition::BottomRight:
        return QStringLiteral("Inferior direito");
    }

    return QStringLiteral("Topo direito");
}

QString OverlaySettingsViewModel::metricDisplayTypeLabel(const MetricDisplayType displayType) {
    switch (displayType) {
    case MetricDisplayType::NumberOnly:
        return QStringLiteral("Apenas número");
    case MetricDisplayType::ChartOnly:
        return QStringLiteral("Apenas gráfico");
    case MetricDisplayType::NumberAndChart:
        return QStringLiteral("Número e gráfico");
    }

    return QStringLiteral("Número e gráfico");
}

QStringList OverlaySettingsViewModel::defaultSelectedMetricIds() {
    return {
        QStringLiteral("fps"),
        QStringLiteral("frame_time"),
        QStringLiteral("graphics_api"),
        QStringLiteral("cpu_usage"),
        QStringLiteral("cpu_temp"),
        QStringLiteral("gpu_usage"),
        QStringLiteral("gpu_temp"),
        QStringLiteral("ram_usage"),
    };
}

OverlaySettingsViewModel::MetricDisplayType OverlaySettingsViewModel::defaultMetricDisplayType(const QString& metricId) {
    const auto definitions = availableMetrics();
    const auto definitionIt = std::find_if(
        definitions.cbegin(),
        definitions.cend(),
        [&metricId](const MetricDefinition& definition) {
            return definition.id == metricId;
        }
    );

    return definitionIt != definitions.cend()
        ? definitionIt->defaultDisplayType
        : MetricDisplayType::NumberAndChart;
}

} // namespace ny::ui::viewmodels

