#pragma once
/**
 * @file GameDetectionService.hpp
 * @author Marcos Henrique
 * @date 16/05/2026
 * @brief Detecta processos de jogo em execução para ativação automática do overlay.
 */
#ifndef NY_FUSION_MONITOR_GAME_DETECTION_SERVICE_HPP
#define NY_FUSION_MONITOR_GAME_DETECTION_SERVICE_HPP

#include <QObject>
#include <QString>
#include <QTimer>

namespace ny::ui::services {

class GameDetectionService final : public QObject {
    Q_OBJECT

public:
    struct DetectionState final {
        bool gameRunning{false};
        bool fullscreenAppDetected{false};
        bool installedGameMatch{false};
        bool highGpuActivity{false};
        QString gameName;
        QString executablePath;
        QString detectionReason;
        qint64 processId{0};
        int confidenceScore{0};
        int indexedInstalledGames{0};
    };

    explicit GameDetectionService(QObject* parent = nullptr);

    void start();
    void stop();
    void setGpuUsagePercent(float percent);
    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] DetectionState currentState() const;

signals:
    void detectionChanged();

private:
    void poll();
    DetectionState detectGameProcess() const;

    QTimer m_pollTimer;
    DetectionState m_state;
    float m_lastGpuUsagePercent{0.0f};
};

} // namespace ny::ui::services

#endif // NY_FUSION_MONITOR_GAME_DETECTION_SERVICE_HPP

