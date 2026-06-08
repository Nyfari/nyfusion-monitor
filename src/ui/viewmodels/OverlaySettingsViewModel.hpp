#pragma once
/**
 * @file OverlaySettingsViewModel.hpp
 * @author Marcos Henrique
 * @date 16/05/2026
 * @brief Estado e persistência da configuração do overlay.
 */
#ifndef NY_FUSION_MONITOR_OVERLAY_SETTINGS_VIEWMODEL_HPP
#define NY_FUSION_MONITOR_OVERLAY_SETTINGS_VIEWMODEL_HPP

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QList>

namespace ny::ui::viewmodels {

class OverlaySettingsViewModel final : public QObject {
    Q_OBJECT

public:
    enum class DisplayMode {
        CompactCards = 0,
        NumbersOnly,
        NumbersAndPercentages,
        MiniCharts,
    };
    Q_ENUM(DisplayMode)

    enum class OverlayPosition {
        TopLeft = 0,
        TopRight,
        BottomLeft,
        BottomRight,
    };
    Q_ENUM(OverlayPosition)

    enum class MetricDisplayType {
        NumberOnly = 0,
        ChartOnly,
        NumberAndChart,
    };
    Q_ENUM(MetricDisplayType)

    struct MetricDefinition final {
        QString id;
        QString title;
        QString description;
        QString sampleValue;
        MetricDisplayType defaultDisplayType;
    };

    explicit OverlaySettingsViewModel(QObject* parent = nullptr);

    [[nodiscard]] bool overlayEnabled() const;
    [[nodiscard]] bool autoEnableOnGameLaunch() const;
    [[nodiscard]] bool showLabels() const;
    [[nodiscard]] int backgroundOpacity() const;
    [[nodiscard]] DisplayMode displayMode() const;
    [[nodiscard]] OverlayPosition overlayPosition() const;
    [[nodiscard]] QStringList selectedMetricIds() const;
    [[nodiscard]] MetricDisplayType metricDisplayType(const QString& metricId) const;

    void setOverlayEnabled(bool enabled);
    void setAutoEnableOnGameLaunch(bool enabled);
    void setShowLabels(bool enabled);
    void setBackgroundOpacity(int opacity);
    void setDisplayMode(DisplayMode mode);
    void setOverlayPosition(OverlayPosition position);
    void setSelectedMetricIds(const QStringList& metricIds);
    void setMetricSelected(const QString& metricId, bool selected);
    void setMetricDisplayType(const QString& metricId, MetricDisplayType displayType);

    void load();
    void save() const;
    void restoreDefaults();

    [[nodiscard]] bool isMetricSelected(const QString& metricId) const;

    static QList<MetricDefinition> availableMetrics();
    static QString displayModeLabel(DisplayMode mode);
    static QString overlayPositionLabel(OverlayPosition position);
    static QString metricDisplayTypeLabel(MetricDisplayType displayType);
    static QStringList defaultSelectedMetricIds();
    static MetricDisplayType defaultMetricDisplayType(const QString& metricId);

signals:
    void settingsChanged();

private:
    bool m_overlayEnabled{false};
    bool m_autoEnableOnGameLaunch{false};
    bool m_showLabels{true};
    int m_backgroundOpacity{78};
    DisplayMode m_displayMode{DisplayMode::CompactCards};
    OverlayPosition m_overlayPosition{OverlayPosition::TopRight};
    QStringList m_selectedMetricIds;
    QHash<QString, MetricDisplayType> m_metricDisplayTypes;
};

} // namespace ny::ui::viewmodels

#endif // NY_FUSION_MONITOR_OVERLAY_SETTINGS_VIEWMODEL_HPP

