/**
 * @file OverlayRuntimeController.cpp
 * @author Marcos Henrique
 * @date 16/05/2026
 */
#include "OverlayRuntimeController.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include "FrameMetricsService.hpp"
#include "GameDetectionService.hpp"
#include "viewmodels/OverlaySettingsViewModel.hpp"

namespace ny::ui::services {

/** Retorna o caminho esperado do libny_fps_hook.so ao lado do executável. */
static QString hookLibPath() {
    const QString dir = QCoreApplication::applicationDirPath();
    const QString path = QDir(dir).filePath(QStringLiteral("libny_fps_hook.so"));
    return path;
}

OverlayRuntimeController::OverlayRuntimeController(QObject* parent)
    : QObject(parent)
    , m_settings(std::make_unique<ny::ui::viewmodels::OverlaySettingsViewModel>())
    , m_gameDetectionService(std::make_unique<GameDetectionService>(this))
    , m_frameMetricsService(std::make_unique<FrameMetricsService>(this))
{
    connect(m_gameDetectionService.get(), &GameDetectionService::detectionChanged,
            this, [this]() { recomputeState(); });

    connect(m_frameMetricsService.get(), &FrameMetricsService::metricsChanged,
            this, [this]() { recomputeState(); });

    updateFromSettings();
}

OverlayRuntimeController::~OverlayRuntimeController() = default;

void OverlayRuntimeController::reloadSettings() {
    m_settings->load();
    updateFromSettings();
}

void OverlayRuntimeController::setGpuUsagePercent(const float percent) {
    m_gameDetectionService->setGpuUsagePercent(percent);
}

OverlayRuntimeController::RuntimeState OverlayRuntimeController::currentState() const {
    return m_state;
}

void OverlayRuntimeController::updateFromSettings() {
    if (m_settings->overlayEnabled() && m_settings->autoEnableOnGameLaunch()) {
        m_gameDetectionService->start();
    } else {
        m_gameDetectionService->stop();
    }

    recomputeState();
}

void OverlayRuntimeController::recomputeState() {
    const auto detectionState = m_gameDetectionService->currentState();

    // ── Gerenciar ciclo de vida do FrameMetricsService ────────────────────────
    if (detectionState.gameRunning && detectionState.processId > 0) {
        if (!m_frameMetricsService->isRunning()) {
            m_frameMetricsService->start(detectionState.processId);
        } else if (m_frameMetricsService->trackedPid() != detectionState.processId) {
            m_frameMetricsService->addPid(detectionState.processId);
        }
    } else {
        if (m_frameMetricsService->isRunning()) {
            m_frameMetricsService->stop();
        }
    }

    // ── Montar próximo estado ─────────────────────────────────────────────────
    RuntimeState nextState;
    nextState.configuredEnabled      = m_settings->overlayEnabled();
    nextState.autoEnableOnGameLaunch = m_settings->autoEnableOnGameLaunch();
    nextState.gameDetected           = detectionState.gameRunning;
    nextState.detectedGameName       = detectionState.gameName;
    nextState.overlayActive          = nextState.configuredEnabled
        && (!nextState.autoEnableOnGameLaunch || nextState.gameDetected);

    // ── Métricas de frame ─────────────────────────────────────────────────────
    const auto frameMetrics    = m_frameMetricsService->currentMetrics();
    nextState.fps              = frameMetrics.fps;
    nextState.frameTimeMs      = frameMetrics.frameTimeMs;
    nextState.graphicsApi      = frameMetrics.graphicsApi;
    nextState.fpsHookActive    = frameMetrics.hookActive;
    nextState.fpsHookPath      = hookLibPath();

    // ── Textos de status ──────────────────────────────────────────────────────
    if (!nextState.configuredEnabled) {
        nextState.summaryText = QStringLiteral("Overlay desligado");
        nextState.detailText  = QStringLiteral("Ative o overlay no menu OVER para permitir exibição em jogo.");
    } else if (nextState.autoEnableOnGameLaunch) {
        if (nextState.gameDetected) {
            nextState.summaryText = QStringLiteral("Overlay ativo automaticamente");
            nextState.detailText  = QStringLiteral("%1 • %2")
                .arg(nextState.detectedGameName,
                     detectionState.detectionReason.isEmpty()
                         ? QStringLiteral("detecção automática")
                         : detectionState.detectionReason);
        } else {
            nextState.summaryText = QStringLiteral("Aguardando jogo");
            nextState.detailText  = QStringLiteral("Monitorando %1 jogos indexados, apps em tela cheia e atividade alta de GPU.")
                .arg(QString::number(detectionState.indexedInstalledGames));
        }
    } else {
        nextState.summaryText = QStringLiteral("Overlay pronto");
        nextState.detailText  = QStringLiteral("Ativação manual habilitada independentemente de jogo aberto.");
    }

    // Emite apenas se algo relevante mudou
    if (nextState.configuredEnabled      == m_state.configuredEnabled
     && nextState.autoEnableOnGameLaunch == m_state.autoEnableOnGameLaunch
     && nextState.gameDetected           == m_state.gameDetected
     && nextState.overlayActive          == m_state.overlayActive
     && nextState.detectedGameName       == m_state.detectedGameName
     && nextState.summaryText            == m_state.summaryText
     && nextState.detailText             == m_state.detailText
     && qAbs(nextState.fps - m_state.fps) < 0.5f
     && nextState.graphicsApi            == m_state.graphicsApi
     && nextState.fpsHookActive          == m_state.fpsHookActive) {
        return;
    }

    m_state = nextState;
    emit stateChanged();
}

} // namespace ny::ui::services
