#pragma once
/**
 * @file OverlaySettingsDialog.hpp
 * @author Marcos Henrique
 * @date 16/05/2026
 * @brief Modal de configuração do overlay.
 */
#ifndef NY_FUSION_MONITOR_OVERLAY_SETTINGS_DIALOG_HPP
#define NY_FUSION_MONITOR_OVERLAY_SETTINGS_DIALOG_HPP

#include <memory>

#include <QDialog>
#include <QHash>

namespace ny::ui::viewmodels {
class OverlaySettingsViewModel;
}

class QCheckBox;
class QComboBox;
class QGridLayout;
class QLabel;
class QLayout;
class QPushButton;
class QSlider;
class QWidget;

namespace ny::ui::qt {

class OverlaySettingsDialog final : public QDialog {
    Q_OBJECT

public:
    explicit OverlaySettingsDialog(QWidget* parent = nullptr);
    ~OverlaySettingsDialog() override;

private:
    void buildUi();
    void wireSignals();
    void loadFromViewModel();
    void refreshPreview();
    void updateValidationState();
    void saveAndAccept();

    QWidget* buildSectionCard(const QString& title, const QString& hint, QLayout* contentLayout);
    QWidget* buildPreviewMetricCard(const QString& metricId) const;

    std::unique_ptr<ny::ui::viewmodels::OverlaySettingsViewModel> m_settings;

    QCheckBox* m_enableOverlayCheck{ nullptr };
    QCheckBox* m_autoEnableOverlayCheck{ nullptr };
    QCheckBox* m_showLabelsCheck{ nullptr };
    QComboBox* m_positionCombo{ nullptr };
    QSlider* m_opacitySlider{ nullptr };
    QLabel* m_opacityValueLabel{ nullptr };
    QLabel* m_validationLabel{ nullptr };
    QLabel* m_previewSummaryLabel{ nullptr };
    QLabel* m_previewStatusBadge{ nullptr };
    QLabel* m_previewGameBadge{ nullptr };
    QWidget* m_previewSurface{ nullptr };
    QGridLayout* m_previewMetricsLayout{ nullptr };
    QPushButton* m_saveButton{ nullptr };
    QHash<QString, QCheckBox*> m_metricChecks;
    QHash<QString, QComboBox*> m_metricDisplayCombos;
};

} // namespace ny::ui::qt

#endif // NY_FUSION_MONITOR_OVERLAY_SETTINGS_DIALOG_HPP

