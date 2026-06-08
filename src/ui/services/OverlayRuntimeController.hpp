#pragma once
/**
 * @file OverlayRuntimeController.hpp
 * @author Marcos Henrique
 * @date 16/05/2026
 * @brief Orquestra o estado do overlay a partir da configuração e da detecção de jogos.
 */
#ifndef NY_FUSION_MONITOR_OVERLAY_RUNTIME_CONTROLLER_HPP
#define NY_FUSION_MONITOR_OVERLAY_RUNTIME_CONTROLLER_HPP

#include <memory>

#include <QObject>
#include <QString>

namespace ny::ui::services {
class FrameMetricsService;
class GameDetectionService;
}

namespace ny::ui::viewmodels {
class OverlaySettingsViewModel;
}

namespace ny::ui::services {

class OverlayRuntimeController final : public QObject {
    Q_OBJECT

public:
    struct RuntimeState final {
        // ── Controle do overlay ───────────────────────────────────────────────
        bool configuredEnabled{false};
        bool autoEnableOnGameLaunch{false};
        bool gameDetected{false};
        bool overlayActive{false};
        QString detectedGameName;
        QString summaryText;
        QString detailText;

        // ── Métricas de frame (do jogo detectado) ─────────────────────────────
        float   fps{0.0f};
        float   frameTimeMs{0.0f};
        QString graphicsApi;
        bool    fpsHookActive{false};   ///< true = FPS vem do hook LD_PRELOAD
        QString fpsHookPath;            ///< caminho do libny_fps_hook.so
    };

    explicit OverlayRuntimeController(QObject* parent = nullptr);
    ~OverlayRuntimeController() override;

    void reloadSettings();
    void setGpuUsagePercent(float percent);
    [[nodiscard]] RuntimeState currentState() const;

signals:
    void stateChanged();

private:
    void updateFromSettings();
    void recomputeState();

    std::unique_ptr<ny::ui::viewmodels::OverlaySettingsViewModel> m_settings;
    std::unique_ptr<GameDetectionService>   m_gameDetectionService;
    std::unique_ptr<FrameMetricsService>    m_frameMetricsService;
    RuntimeState m_state;
};

} // namespace ny::ui::services

#endif // NY_FUSION_MONITOR_OVERLAY_RUNTIME_CONTROLLER_HPP

