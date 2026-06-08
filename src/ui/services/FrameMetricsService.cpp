/**
 * @file FrameMetricsService.cpp
 * @author Marcos Henrique
 * @date 16/05/2026
 *
 * Mecanismo 1 — Hook LD_PRELOAD (preferencial):
 *   libny_fps_hook.so intercepta glXSwapBuffers/eglSwapBuffers/vkQueuePresentKHR
 *   e incrementa um contador em /tmp/.ny_fps_hook. Lemos esse arquivo via mmap.
 *
 * Mecanismo 2 — XCB Present (fallback):
 *   xcb_present_select_input nas janelas do jogo. Funciona apenas para jogos
 *   X11 que usam X Present nativo. NÃO funciona no XWayland com Vulkan.
 */
#include "FrameMetricsService.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QSet>
#include <QtGui/qguiapplication_platform.h>

// ── Hook: leitura de memória compartilhada ───────────────────────────────────
#if defined(Q_OS_LINUX)
#  include <fcntl.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#  include <unistd.h>
#  include "../../hook/ny_fps_shared.h"
#endif

// ── XCB Present sem o header xcb/present.h ──────────────────────────────────
#if defined(Q_OS_LINUX) && defined(NY_HAS_XCB_PRESENT)
#  include <cstring>

extern "C" {
    /**
     * xcb_present_select_input – opcode 3 da extensão Present.
     * Registra a conexão xcb para receber eventos PresentCompleteNotify
     * na janela indicada.
     *
     * @param c          conexão xcb
     * @param eid        ID de porta de evento (xcb_present_event_t = uint32_t)
     * @param window     janela a monitorar
     * @param event_mask máscara: 2 => CompleteNotify
     */
    xcb_void_cookie_t xcb_present_select_input(
        xcb_connection_t* c,
        uint32_t          eid,
        xcb_window_t      window,
        uint32_t          event_mask);
}

// Constantes do protocolo XCB Present (presente em xcb/present.h, replicadas
// aqui para evitar a dependência do pacote -dev).
static constexpr uint8_t  kXcbGeGeneric                   = 35u;   // XCB_GE_GENERIC
static constexpr uint16_t kPresentCompleteNotify           = 1u;    // evtype
static constexpr uint32_t kPresentEventMaskCompleteNotify  = 2u;    // bit 1

// ── XCB event masks (para SubstructureNotify na root window) ─────────────────
static constexpr uint32_t kXcbEventMaskSubstructureNotify = 0x00080000u;
static constexpr uint8_t  kXcbMapNotify                   = 19u;

// Número máximo de janelas rastreadas simultaneamente
static constexpr int kMaxTrackedWindows = 32;

#endif  // Q_OS_LINUX && NY_HAS_XCB_PRESENT

#if defined(Q_OS_LINUX)
// ── Utilitário: árvore de processos ──────────────────────────────────────────
/**
 * Retorna o conjunto de todos os PIDs na árvore descendente de rootPid
 * (inclusive o próprio rootPid). Lê /proc/<pid>/status para encontrar
 * filhos sem depender de ferramentas externas.
 */
static QSet<qint64> buildPidTree(qint64 rootPid) {
    QSet<qint64> result;
    if (rootPid <= 0) return result;

    // BFS usando /proc/<pid>/task/<tid>/children  (kernel ≥ 3.5)
    // Fallback: varrer /proc e comparar PPid
    QList<qint64> queue;
    queue.append(rootPid);

    while (!queue.isEmpty()) {
        const qint64 pid = queue.takeFirst();
        if (result.contains(pid)) continue;
        result.insert(pid);

        // Tenta ler filhos diretos via /proc/<pid>/task/<pid>/children
        QFile childrenFile(QStringLiteral("/proc/%1/task/%1/children").arg(pid));
        if (childrenFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString data = QString::fromUtf8(childrenFile.readAll()).trimmed();
            if (!data.isEmpty()) {
                const QStringList parts = data.split(QLatin1Char(' '), Qt::SkipEmptyParts);
                for (const QString& p : parts) {
                    bool ok = false;
                    const qint64 child = p.toLongLong(&ok);
                    if (ok && child > 0 && !result.contains(child))
                        queue.append(child);
                }
            }
            continue;
        }

        // Fallback: varrer /proc/<n>/status procurando PPid == pid
        const QDir procDir(QStringLiteral("/proc"));
        const auto entries = procDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& entry : entries) {
            bool ok = false;
            const qint64 candidatePid = entry.toLongLong(&ok);
            if (!ok || candidatePid <= 0 || result.contains(candidatePid)) continue;

            QFile statusFile(QStringLiteral("/proc/%1/status").arg(candidatePid));
            if (!statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

            while (!statusFile.atEnd()) {
                const QString line = QString::fromUtf8(statusFile.readLine());
                if (line.startsWith(QLatin1String("PPid:"))) {
                    const QString ppidStr = line.mid(5).trimmed();
                    bool pok = false;
                    const qint64 ppid = ppidStr.toLongLong(&pok);
                    if (pok && ppid == pid)
                        queue.append(candidatePid);
                    break;
                }
            }
        }
    }
    return result;
}
#endif  // Q_OS_LINUX

namespace ny::ui::services {

// ── Construtor / Destrutor ────────────────────────────────────────────────────

FrameMetricsService::FrameMetricsService(QObject* parent)
    : QObject(parent)
{
    // Timer de FPS: a cada 1 segundo lê e zera o contador de frames
    m_fpsTimer.setInterval(1000);
    connect(&m_fpsTimer, &QTimer::timeout, this, &FrameMetricsService::onFpsTick);

    // Timer de poll: a cada 2 segundos re-detecta API e tenta registrar novas
    // janelas (o jogo pode abrir sua janela depois que start() foi chamado)
    m_pollTimer.setInterval(2000);
    connect(&m_pollTimer, &QTimer::timeout, this, &FrameMetricsService::onPollTick);
}

FrameMetricsService::~FrameMetricsService() {
    stop();
}

// ── API pública ───────────────────────────────────────────────────────────────

void FrameMetricsService::start(qint64 pid) {
    if (m_pid == pid && m_fpsTimer.isActive()) {
        return;
    }

    stop();

    m_pid              = pid;
    m_xcbFrameCount    = 0;
    m_geEventsReceived = 0;
    m_metrics          = FrameMetrics{};
    m_metrics.graphicsApi = detectApi(pid);

    // Tenta abrir o arquivo de memória compartilhada do hook (mecanismo 1)
    tryOpenHook();

    // Configura XCB Present (mecanismo 2 — fallback se hook não estiver ativo)
    if (!m_hookValid) {
        setupXcbPresent();
        findAndTrackGameWindows(pid);
    }

    m_fpsTimer.start();
    m_pollTimer.start();
}

void FrameMetricsService::addPid(qint64 pid) {
    if (pid <= 0 || pid == m_pid) return;

    m_pid = pid;

    const QString newApi = detectApi(pid);
    if (!newApi.isEmpty() && newApi != m_metrics.graphicsApi) {
        m_metrics.graphicsApi = newApi;
        emit metricsChanged();
    }

    // Se o hook não está ativo ainda, tenta novamente com o novo PID
    if (!m_hookValid) {
        tryOpenHook();
    }

    if (!m_hookValid) {
        setupXcbPresent();
        findAndTrackGameWindows(pid);
    }
}

void FrameMetricsService::stop() {
    m_fpsTimer.stop();
    m_pollTimer.stop();

    closeHook();
    unregisterAll();

    m_pid              = 0;
    m_xcbFrameCount    = 0;
    m_geEventsReceived = 0;

    const bool hadData = m_metrics.fps > 0.0f || !m_metrics.graphicsApi.isEmpty();
    m_metrics = FrameMetrics{};
    if (hadData) {
        emit metricsChanged();
    }
}

bool FrameMetricsService::isRunning() const {
    return m_fpsTimer.isActive();
}

qint64 FrameMetricsService::trackedPid() const {
    return m_pid;
}

FrameMetricsService::FrameMetrics FrameMetricsService::currentMetrics() const {
    return m_metrics;
}

// ── Slots de timer ────────────────────────────────────────────────────────────

/**
 * Chamado a cada 1 segundo.
 *
 * Prioridade:
 *   1. Hook LD_PRELOAD  — lê frame_count do mmap, calcula delta
 *   2. XCB Present      — usa m_xcbFrameCount acumulado
 */
void FrameMetricsService::onFpsTick() {
    if (m_pid <= 0) return;

    uint32_t framesThisTick = 0;
    bool     fromHook       = false;

    // ── Mecanismo 1: hook ────────────────────────────────────────────────────
    if (m_hookValid) {
        uint64_t current = 0;
        if (readHookFrameCount(current)) {
            if (m_hookFrameBase == 0) {
                m_hookFrameBase = current; // primeiro tick: só registra base
            } else if (current >= m_hookFrameBase) {
                framesThisTick = static_cast<uint32_t>(current - m_hookFrameBase);
                m_hookFrameBase = current;
            }
            fromHook = true;
        } else {
            qDebug() << "[FrameMetrics] Hook desconectado";
            closeHook();
        }
    }

    // ── Mecanismo 2: XCB Present (fallback) ─────────────────────────────────
    if (!fromHook) {
        framesThisTick  = m_xcbFrameCount;
        m_xcbFrameCount = 0;
    }

    // ── Publica métricas ─────────────────────────────────────────────────────
    FrameMetrics newMetrics;
    newMetrics.graphicsApi = m_metrics.graphicsApi;
    newMetrics.hookActive  = fromHook;
    newMetrics.fps         = static_cast<float>(framesThisTick);
    newMetrics.frameTimeMs = (newMetrics.fps > 0.1f) ? 1000.0f / newMetrics.fps : 0.0f;

    const bool changed = qAbs(newMetrics.fps - m_metrics.fps) > 0.5f
                      || newMetrics.hookActive  != m_metrics.hookActive
                      || newMetrics.graphicsApi != m_metrics.graphicsApi;
    if (changed) {
        m_metrics = newMetrics;
        emit metricsChanged();
    }
}

/**
 * Chamado a cada 2 segundos.
 * Re-detecta API gráfica, tenta abrir o hook, e (se sem hook) tenta
 * registrar novas janelas XCB Present.
 */
void FrameMetricsService::onPollTick() {
    if (m_pid <= 0) return;

    const QString api = detectApi(m_pid);
    if (api != m_metrics.graphicsApi) {
        m_metrics.graphicsApi = api;
        emit metricsChanged();
    }

    if (!m_hookValid) {
        if (tryOpenHook()) {
            qDebug() << "[FrameMetrics] Hook detectado no PID" << m_pid;
        }
    }

#if defined(Q_OS_LINUX) && defined(NY_HAS_XCB_PRESENT)
    if (!m_hookValid && m_presentAvailable && m_presentRegs.isEmpty()) {
        findAndTrackGameWindows(m_pid);
    }
#endif
}

// ── QAbstractNativeEventFilter ────────────────────────────────────────────────

/**
 * Intercepta eventos XCB antes que o Qt os processe.
 *
 * • PresentCompleteNotify (response_type=35, evtype=1): frame apresentado → ++frameCount
 * • MapNotify (response_type=19): nova janela mapeada → tenta registrá-la
 *
 * Chamado na thread principal — sem necessidade de mutex.
 */
bool FrameMetricsService::nativeEventFilter(const QByteArray& eventType,
                                             void*             message,
                                             qintptr*          /*result*/) {
#if defined(Q_OS_LINUX) && defined(NY_HAS_XCB_PRESENT)
    if (m_pid <= 0 || !m_presentAvailable) {
        return false;
    }
    if (eventType != QByteArrayLiteral("xcb_generic_event_t")) {
        return false;
    }

    const auto* ev = static_cast<const xcb_generic_event_t*>(message);
    const uint8_t responseType = ev->response_type & ~0x80u;

    // ── MapNotify: uma nova janela foi mapeada ────────────────────────────────
    if (responseType == kXcbMapNotify) {
        QTimer::singleShot(200, this, [this]() {
            if (m_pid > 0) findAndTrackGameWindows(m_pid);
        });
        return false;
    }

    // ── GE Generic: envelope para extensões modernas (Present, XInput, etc.) ─
    if (responseType != kXcbGeGeneric) {
        return false;
    }

    const auto* ge = reinterpret_cast<const xcb_ge_generic_event_t*>(ev);

    // Log diagnóstico: mostra os primeiros GE events recebidos para cada sessão
    if (m_geEventsReceived < 5) {
        ++m_geEventsReceived;
        qDebug() << "[FrameMetrics] GE event recebido — extension:" << ge->extension
                 << "event_type:" << ge->event_type
                 << "esperado opcode:" << m_presentMajorOpcode
                 << "evtype:" << kPresentCompleteNotify;
    }

    if (ge->extension != m_presentMajorOpcode) {
        return false;
    }

    if (ge->event_type != kPresentCompleteNotify) {
        return false;
    }

    // ✓ Frame apresentado
    ++m_xcbFrameCount;

    // Log dos primeiros frames detectados
    if (m_xcbFrameCount <= 3) {
        qDebug() << "[FrameMetrics] PresentCompleteNotify! xcbFrameCount =" << m_xcbFrameCount;
    }

    return false;
#else
    Q_UNUSED(eventType)
    Q_UNUSED(message)
    return false;
#endif
}

// ── Inicialização XCB Present ─────────────────────────────────────────────────

void FrameMetricsService::setupXcbPresent() {
#if defined(Q_OS_LINUX) && defined(NY_HAS_XCB_PRESENT)
    if (m_presentAvailable) {
        return;  // já configurado
    }

    // Obtém a conexão XCB via QNativeInterface::QX11Application (Qt 6.2+)
    auto* x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!x11App) {
        qWarning() << "[FrameMetrics] setupXcbPresent: QNativeInterface::QX11Application não disponível"
                   << "(plataforma:" << QGuiApplication::platformName() << ")";
        return;
    }

    m_xcbConn = x11App->connection();

    if (!m_xcbConn || xcb_connection_has_error(m_xcbConn)) {
        qWarning() << "[FrameMetrics] setupXcbPresent: conexão XCB inválida";
        m_xcbConn = nullptr;
        return;
    }

    // Consulta se a extensão Present está disponível no servidor X
    const char* extName = "Present";
    auto cookie = xcb_query_extension(
        m_xcbConn,
        static_cast<uint16_t>(std::strlen(extName)),
        extName);

    xcb_query_extension_reply_t* reply =
        xcb_query_extension_reply(m_xcbConn, cookie, nullptr);

    if (!reply) {
        qWarning() << "[FrameMetrics] setupXcbPresent: xcb_query_extension falhou";
        return;
    }

    if (reply->present) {
        m_presentMajorOpcode = reply->major_opcode;
        m_presentAvailable   = true;

        if (!m_filterInstalled) {
            qApp->installNativeEventFilter(this);
            m_filterInstalled = true;
        }
        qDebug() << "[FrameMetrics] XCB Present OK — major_opcode:" << m_presentMajorOpcode;
    } else {
        qWarning() << "[FrameMetrics] setupXcbPresent: extensão Present NÃO disponível no servidor X";
    }

    free(reply);
#endif
}

/**
 * Enumera as janelas X11 de nível superior do jogo e as registra para
 * receber PresentCompleteNotify.
 *
 * Estratégia de correspondência (por ordem de tentativa):
 *   1. _NET_WM_PID pertence ao PID do jogo OU a qualquer processo filho/neto
 *      (necessário para Proton/Wine onde o render corre em subprocesso).
 *   2. Fallback: qualquer janela visível com geometria grande (≥ 50 % de tela)
 *      — captura jogos que não setam _NET_WM_PID corretamente.
 *
 * Além disso, subscreve SubstructureNotify na root window para que janelas
 * abertas DEPOIS de start() sejam detectadas via nativeEventFilter.
 */
void FrameMetricsService::findAndTrackGameWindows(qint64 pid) {
#if defined(Q_OS_LINUX) && defined(NY_HAS_XCB_PRESENT)
    if (!m_xcbConn || !m_presentAvailable || pid <= 0) {
        return;
    }

    // ── Obtém a janela raiz ──────────────────────────────────────────────────
    const xcb_setup_t* setup = xcb_get_setup(m_xcbConn);
    if (!setup) return;

    xcb_screen_t* screen = xcb_setup_roots_iterator(setup).data;
    if (!screen) return;

    const xcb_window_t root       = screen->root;
    const uint16_t     screenW    = screen->width_in_pixels;
    const uint16_t     screenH    = screen->height_in_pixels;
    const uint32_t     minArea    = static_cast<uint32_t>(screenW) *
                                    static_cast<uint32_t>(screenH) / 2u;

    // ── Subscreve SubstructureNotify na root para pegar janelas futuras ──────
    if (!m_rootNotifySubscribed) {
        const uint32_t mask = kXcbEventMaskSubstructureNotify;
        xcb_change_window_attributes(m_xcbConn, root,
                                     XCB_CW_EVENT_MASK, &mask);
        m_rootNotifySubscribed = true;
    }

    // ── Constrói árvore de PIDs do jogo (PID + todos descendentes) ───────────
    const QSet<qint64> pidTree = buildPidTree(pid);

    // ── Obtém o átomo _NET_WM_PID ────────────────────────────────────────────
    const char* atomStr = "_NET_WM_PID";
    xcb_intern_atom_reply_t* atomReply = xcb_intern_atom_reply(
        m_xcbConn,
        xcb_intern_atom(m_xcbConn, 1,
                        static_cast<uint16_t>(std::strlen(atomStr)), atomStr),
        nullptr);

    const xcb_atom_t pidAtom = atomReply ? atomReply->atom : static_cast<xcb_atom_t>(XCB_ATOM_NONE);
    free(atomReply);

    // ── Enumera filhos diretos da janela raiz (top-level windows) ────────────
    xcb_query_tree_reply_t* treeReply = xcb_query_tree_reply(
        m_xcbConn, xcb_query_tree(m_xcbConn, root), nullptr);

    if (!treeReply) return;

    const xcb_window_t* children  = xcb_query_tree_children(treeReply);
    const int           childCount = xcb_query_tree_children_length(treeReply);

    QList<xcb_window_t> unmatched;  // candidatos para o fallback geométrico

    for (int i = 0; i < childCount && static_cast<int>(m_presentRegs.size()) < kMaxTrackedWindows; ++i) {
        const xcb_window_t w = children[i];

        if (isAlreadyTracked(w)) continue;

        // ── Tenta match por PID (inclui toda a árvore de processos filhos) ───
        bool matchesPid = false;
        if (pidAtom != XCB_ATOM_NONE) {
            xcb_get_property_reply_t* propReply = xcb_get_property_reply(
                m_xcbConn,
                xcb_get_property(m_xcbConn, 0, w, pidAtom,
                                 XCB_ATOM_CARDINAL, 0, 1),
                nullptr);

            if (propReply &&
                propReply->type   == XCB_ATOM_CARDINAL &&
                propReply->format == 32                 &&
                xcb_get_property_value_length(propReply) >= 4)
            {
                const uint32_t wpid =
                    *reinterpret_cast<const uint32_t*>(
                        xcb_get_property_value(propReply));
                matchesPid = pidTree.contains(static_cast<qint64>(wpid));
            }
            free(propReply);
        }

        if (matchesPid) {
            trackWindow(w);
        } else {
            unmatched.append(w);
        }
    }

    // ── Fallback geométrico: registra janelas grandes se nenhuma foi encontrada
    if (m_presentRegs.isEmpty() && !unmatched.isEmpty()) {
        for (const xcb_window_t w : unmatched) {
            if (static_cast<int>(m_presentRegs.size()) >= kMaxTrackedWindows) break;

            xcb_get_geometry_reply_t* geomReply = xcb_get_geometry_reply(
                m_xcbConn, xcb_get_geometry(m_xcbConn, w), nullptr);

            if (!geomReply) continue;
            const uint32_t area = static_cast<uint32_t>(geomReply->width) *
                                  static_cast<uint32_t>(geomReply->height);
            free(geomReply);

            if (area >= minArea) {
                trackWindow(w);
                qDebug() << "[FrameMetrics] Fallback: rastreando janela grande" << w;
            }
        }
    }

    xcb_flush(m_xcbConn);
    free(treeReply);

    qDebug() << "[FrameMetrics] Janelas rastreadas:" << m_presentRegs.size()
             << "para PID" << pid << "(árvore:" << pidTree.size() << "PIDs)";
#else
    Q_UNUSED(pid)
#endif
}

// ── Helpers privados ──────────────────────────────────────────────────────────

#if defined(Q_OS_LINUX) && defined(NY_HAS_XCB_PRESENT)
bool FrameMetricsService::isAlreadyTracked(xcb_window_t w) const {
    for (const auto& reg : m_presentRegs) {
        if (reg.window == static_cast<uint32_t>(w)) return true;
    }
    return false;
}

void FrameMetricsService::trackWindow(xcb_window_t w) {
    const uint32_t eid = xcb_generate_id(m_xcbConn);
    xcb_present_select_input(m_xcbConn, eid, w, kPresentEventMaskCompleteNotify);
    m_presentRegs.push_back(PresentReg{ static_cast<uint32_t>(w), eid });
    qDebug() << "[FrameMetrics] Registrado PresentCompleteNotify na janela" << w;
}
#endif

/**
 * Cancela o registro de todos os eventos Present e remove o filtro nativo.
 * Chamado em stop() para liberar recursos do servidor X.
 */
void FrameMetricsService::unregisterAll() {
#if defined(Q_OS_LINUX) && defined(NY_HAS_XCB_PRESENT)
    if (m_filterInstalled) {
        qApp->removeNativeEventFilter(this);
        m_filterInstalled = false;
    }

    if (m_xcbConn && m_presentAvailable) {
        for (const auto& reg : m_presentRegs) {
            // event_mask = 0 → cancela a seleção deste evento port
            xcb_present_select_input(m_xcbConn, reg.eid, reg.window, 0u);
        }
        if (!m_presentRegs.isEmpty()) {
            xcb_flush(m_xcbConn);
        }
    }

    m_presentRegs.clear();
    m_presentAvailable      = false;
    m_presentMajorOpcode    = 0;
    m_xcbConn               = nullptr;
    m_rootNotifySubscribed  = false;
#endif
}

// ── Hook LD_PRELOAD — memória compartilhada ───────────────────────────────────

/**
 * Tenta abrir /tmp/.ny_fps_hook e validar que o PID no arquivo corresponde
 * ao game detectado (ou a algum processo na árvore de PIDs do jogo).
 * Retorna true se o hook foi aberto com sucesso.
 */
bool FrameMetricsService::tryOpenHook() {
#if defined(Q_OS_LINUX)
    closeHook(); // fecha qualquer mmap anterior

    int fd = ::open(NY_FPS_SHM_PATH, O_RDONLY);
    if (fd < 0) return false;

    // Verifica tamanho mínimo
    struct ::stat st{};
    if (::fstat(fd, &st) != 0 || st.st_size < static_cast<off_t>(sizeof(NyFpsShmData))) {
        ::close(fd);
        return false;
    }

    void* m = ::mmap(nullptr, sizeof(NyFpsShmData), PROT_READ, MAP_SHARED, fd, 0);
    ::close(fd);
    if (m == MAP_FAILED) return false;

    const auto* d = static_cast<const NyFpsShmData*>(m);
    if (d->magic != NY_FPS_SHM_MAGIC || d->version != NY_FPS_SHM_VERSION) {
        ::munmap(m, sizeof(NyFpsShmData));
        return false;
    }
    // Valida que o PID pertence à árvore de processos do jogo
    const qint64 hookPid = static_cast<qint64>(d->pid);
    const QSet<qint64> pidTree = buildPidTree(m_pid);
    if (!pidTree.contains(hookPid) && hookPid != m_pid) {
        // PID do arquivo não pertence ao jogo — pode ser de uma sessão anterior
        ::munmap(m, sizeof(NyFpsShmData));
        qDebug() << "[FrameMetrics] Hook encontrado mas PID" << hookPid
                 << "não pertence ao jogo (PID" << m_pid << ")";
        return false;
    }

    m_hookMmap       = m;
    m_hookFrameBase  = 0;  // será lido no primeiro tick
    m_hookValid      = true;
    qDebug() << "[FrameMetrics] Hook aberto — PID" << hookPid
             << "frame_count atual:" << d->frame_count;
    return true;
#else
    return false;
#endif
}

void FrameMetricsService::closeHook() {
#if defined(Q_OS_LINUX)
    if (m_hookMmap) {
        ::munmap(m_hookMmap, sizeof(NyFpsShmData));
        m_hookMmap = nullptr;
    }
    m_hookValid      = false;
    m_hookFrameBase  = 0;
#endif
}

/**
 * Lê frame_count do mmap com semântica acquire para sincronizar com o
 * hook que usa atomic_fetch_add (release).
 */
bool FrameMetricsService::readHookFrameCount(uint64_t& outCount) const {
#if defined(Q_OS_LINUX)
    if (!m_hookMmap || !m_hookValid) return false;

    const auto* d = static_cast<const NyFpsShmData*>(m_hookMmap);
    // Acesso atômico compatible com o _Atomic uint64_t do lado C
    outCount = __atomic_load_n(&d->frame_count, __ATOMIC_ACQUIRE);
    return true;
#else
    Q_UNUSED(outCount)
    return false;
#endif
}

// ── Detecção de API gráfica ───────────────────────────────────────────────────

QString FrameMetricsService::detectApi(qint64 pid) const {
    if (pid <= 0) {
        return {};
    }

    QFile mapsFile(QStringLiteral("/proc/%1/maps").arg(pid));
    if (!mapsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QString content = QString::fromUtf8(mapsFile.readAll()).toLower();

    // Prioridade: DX12 > DX11 > DX9 > Vulkan nativo > OpenGL

    if (content.contains(QStringLiteral("vkd3d")) ||
        content.contains(QStringLiteral("d3d12.dll"))) {
        return QStringLiteral("DirectX 12 (VKD3D→Vulkan)");
    }

    if (content.contains(QStringLiteral("dxvk")) ||
        content.contains(QStringLiteral("d3d11.dll"))) {
        return QStringLiteral("DirectX 11 (DXVK→Vulkan)");
    }

    if (content.contains(QStringLiteral("d3d9.dll"))) {
        const bool hasDxvk   = content.contains(QStringLiteral("dxvk"));
        const bool hasVulkan = content.contains(QStringLiteral("libvulkan"));
        return (hasDxvk || hasVulkan)
            ? QStringLiteral("DirectX 9 (DXVK→Vulkan)")
            : QStringLiteral("DirectX 9 (OpenGL)");
    }

    if (content.contains(QStringLiteral("libvulkan")) ||
        content.contains(QStringLiteral("vulkan/icd"))) {
        return QStringLiteral("Vulkan");
    }

    if (content.contains(QStringLiteral("libglesv2.so"))) {
        return QStringLiteral("OpenGL ES");
    }

    if (content.contains(QStringLiteral("libgl.so"))   ||
        content.contains(QStringLiteral("mesa_dri"))    ||
        content.contains(QStringLiteral("libglx"))) {
        return QStringLiteral("OpenGL");
    }

    return {};
}

} // namespace ny::ui::services
