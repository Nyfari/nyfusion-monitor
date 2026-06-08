#pragma once
/**
 * @file FrameMetricsService.hpp
 * @author Marcos Henrique
 * @date 16/05/2026
 * @brief Mede FPS real do jogo em execução.
 *
 * Mecanismo 1 — Hook LD_PRELOAD (preferencial):
 *   libny_fps_hook.so intercepta glXSwapBuffers/eglSwapBuffers/vkQueuePresentKHR
 *   dentro do processo do jogo e incrementa um contador em memória compartilhada
 *   (/tmp/.ny_fps_hook). Este serviço lê esse contador a cada segundo.
 *   Requer que o jogo seja lançado com:
 *     LD_PRELOAD=/caminho/para/libny_fps_hook.so %command%
 *
 * Mecanismo 2 — XCB Present (fallback):
 *   Tenta receber PresentCompleteNotify eventos via xcb_present_select_input.
 *   NÃO funciona sob XWayland com jogos Vulkan (VK_KHR_wayland_surface bypassa
 *   o protocolo X Present), mas pode funcionar com jogos OpenGL legados.
 *
 * Detecção de API gráfica:
 *   /proc/{pid}/maps → DXVK/VKD3D/Vulkan/OpenGL
 */
#ifndef NY_FUSION_MONITOR_FRAME_METRICS_SERVICE_HPP
#define NY_FUSION_MONITOR_FRAME_METRICS_SERVICE_HPP

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTimer>

#if defined(Q_OS_LINUX) && defined(NY_HAS_XCB_PRESENT)
#  include <xcb/xcb.h>
#endif

namespace ny::ui::services {

class FrameMetricsService final : public QObject,
                                   public QAbstractNativeEventFilter {
    Q_OBJECT

public:
    struct FrameMetrics final {
        float   fps{0.0f};
        float   frameTimeMs{0.0f};
        QString graphicsApi;
        bool    hookActive{false};  ///< true quando FPS vem do hook LD_PRELOAD
    };

    explicit FrameMetricsService(QObject* parent = nullptr);
    ~FrameMetricsService() override;

    void start(qint64 pid);
    void addPid(qint64 pid);   ///< adiciona janelas de um PID extra sem resetar estado
    void stop();

    [[nodiscard]] bool         isRunning()      const;
    [[nodiscard]] qint64       trackedPid()     const;
    [[nodiscard]] FrameMetrics currentMetrics() const;

    // QAbstractNativeEventFilter — fallback XCB Present
    bool nativeEventFilter(const QByteArray& eventType,
                           void*             message,
                           qintptr*          result) override;

signals:
    void metricsChanged();

private slots:
    void onFpsTick();   // a cada 1 s: publica fps, zera contador
    void onPollTick();  // a cada 2 s: re-detecta API, verifica hook, tenta janelas

private:
    [[nodiscard]] QString detectApi(qint64 pid) const;

    // ── Hook LD_PRELOAD (mecanismo 1) ────────────────────────────────────────
    bool tryOpenHook();       ///< tenta abrir /tmp/.ny_fps_hook para o PID atual
    void closeHook();         ///< fecha o mmap do hook
    bool readHookFrameCount(uint64_t& outCount) const;

    // ── XCB Present (mecanismo 2 — fallback) ────────────────────────────────
    void setupXcbPresent();
    void findAndTrackGameWindows(qint64 pid);
    void unregisterAll();

    // ── Timers ──────────────────────────────────────────────────────────────
    QTimer m_fpsTimer;   ///< 1 segundo
    QTimer m_pollTimer;  ///< 2 segundos

    // ── Estado geral ─────────────────────────────────────────────────────────
    qint64       m_pid{0};
    FrameMetrics m_metrics;
    uint32_t     m_geEventsReceived{0};  ///< diagnóstico

    // ── Hook: memória compartilhada ──────────────────────────────────────────
    void*    m_hookMmap{nullptr};        ///< ponteiro para NyFpsShmData mapeado
    uint64_t m_hookFrameBase{0};         ///< frame_count no início do tick atual
    bool     m_hookValid{false};

    // ── XCB Present: contagem de frames via eventos ──────────────────────────
    uint32_t m_xcbFrameCount{0};         ///< frames contados pelo nativeEventFilter

#if defined(Q_OS_LINUX) && defined(NY_HAS_XCB_PRESENT)
    struct PresentReg {
        uint32_t window;
        uint32_t eid;
    };

    bool isAlreadyTracked(xcb_window_t w) const;
    void trackWindow(xcb_window_t w);

    xcb_connection_t* m_xcbConn{nullptr};
    uint8_t           m_presentMajorOpcode{0};
    bool              m_presentAvailable{false};
    bool              m_filterInstalled{false};
    bool              m_rootNotifySubscribed{false};
    QList<PresentReg> m_presentRegs;
#endif
};

} // namespace ny::ui::services

#endif // NY_FUSION_MONITOR_FRAME_METRICS_SERVICE_HPP
