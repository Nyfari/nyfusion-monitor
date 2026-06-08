/**
 * @file OverlayWindow.cpp
 * @author Marcos Henrique
 * @date 16/05/2026
 */
#include "OverlayWindow.hpp"

#include <algorithm>
#include <optional>

#include <QCursor>
#include <QFrame>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QRegularExpression>
#include <QScreen>
#include <QShowEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

#include "viewmodels/OverlaySettingsViewModel.hpp"

namespace ny::ui::qt {
namespace {
constexpr int kOverlayMargin = 18;
constexpr int kMaxHistoryPoints = 20;

void clearLayout(QLayout* layout) {
    if (!layout) {
        return;
    }

    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->layout()) {
            clearLayout(item->layout());
        }

        if (item->widget()) {
            item->widget()->deleteLater();
        }

        delete item;
    }
}

QString metricTitleForId(const QString& metricId) {
    for (const auto& definition : ny::ui::viewmodels::OverlaySettingsViewModel::availableMetrics()) {
        if (definition.id == metricId) {
            return definition.title;
        }
    }
    return metricId;
}
} // namespace

OverlayWindow::OverlayWindow(QWidget* parent)
    : QWidget(parent)   // parent deve ser sempre nullptr — janela totalmente independente
    , m_settings(std::make_unique<ny::ui::viewmodels::OverlaySettingsViewModel>())
{
    // ── Janela completamente independente: não fecha o app, não herda o grupo ──
    setAttribute(Qt::WA_QuitOnClose, false);

    setObjectName(QStringLiteral("floatingOverlayWindow"));
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setFocusPolicy(Qt::NoFocus);

    // ── Flags base: janela autônoma, sem borda, sem sombra, sem foco ──────────
    Qt::WindowFlags flags =
        Qt::Window
        | Qt::CustomizeWindowHint
        | Qt::FramelessWindowHint
        | Qt::WindowStaysOnTopHint
        | Qt::NoDropShadowWindowHint
        | Qt::WindowDoesNotAcceptFocus;

    // ── Plataforma: X11/XCB usa BypassWindowManagerHint (override_redirect=1) ─
    // Em sessões Wayland, o main.cpp já garante QT_QPA_PLATFORM=xcb (XWayland),
    // por isso "xcb" aqui indica tanto X11 nativo quanto XWayland.
    const QString platform = QGuiApplication::platformName();
    const bool isX11 = platform.startsWith(QStringLiteral("xcb"))
                    || platform == QStringLiteral("x11");
#if defined(Q_OS_LINUX)
    if (isX11) {
        // override_redirect = 1: o WM/compositor não gerencia esta janela
        flags |= Qt::BypassWindowManagerHint;
        setAttribute(Qt::WA_X11DoNotAcceptFocus, true);
    }
#endif

    setWindowFlags(flags);
    setWindowFlag(Qt::WindowTransparentForInput, true);

    // ── Timer de re-raise: garante que a janela permaneça acima mesmo que o ──
    // ── compositor ou o jogo tente empurrá-la para baixo ─────────────────────
    m_raiseTimer.setInterval(400);
    connect(&m_raiseTimer, &QTimer::timeout, this, [this]() {
        if (!isVisible()) {
            return;
        }
        raise();
        if (windowHandle()) {
            windowHandle()->raise();
        }
    });

    buildUi();
    refreshContent();
    syncVisibility();
}

OverlayWindow::~OverlayWindow() = default;

void OverlayWindow::reloadSettings() {
    m_settings->load();
    refreshContent();
    syncVisibility();
}

void OverlayWindow::setRuntimeState(const ny::ui::services::OverlayRuntimeController::RuntimeState& state) {
    m_runtimeState = state;
    refreshContent();
    syncVisibility();
}

void OverlayWindow::updateMetrics(
    const ny::ui::viewmodels::DashboardViewModel::CpuViewData& cpu,
    const ny::ui::viewmodels::DashboardViewModel::MemoryViewData& mem,
    const QList<ny::ui::viewmodels::DashboardViewModel::GpuViewData>& gpus
) {
    m_cpuData = cpu;
    m_memoryData = mem;
    m_gpuData = gpus;
    updateHistory();
    refreshContent();
    if (isVisible()) {
        moveToConfiguredPosition();
    }
}

void OverlayWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    m_isMapped = true;
    QTimer::singleShot(0, this, [this]() {
        moveToConfiguredPosition();
        raise();
        if (windowHandle()) {
            windowHandle()->raise();
        }
    });
}

void OverlayWindow::buildUi() {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto* surface = new QFrame(this);
    surface->setObjectName(QStringLiteral("floatingOverlaySurface"));
    auto* surfaceLayout = new QVBoxLayout(surface);
    surfaceLayout->setContentsMargins(14, 12, 14, 12);
    surfaceLayout->setSpacing(10);

    auto* headerRow = new QWidget(surface);
    headerRow->setObjectName(QStringLiteral("floatingOverlayHeader"));
    auto* headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);

    auto* titleColumn = new QWidget(headerRow);
    auto* titleLayout = new QVBoxLayout(titleColumn);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(2);

    m_titleLabel = new QLabel(QStringLiteral("NyFusion Overlay"), titleColumn);
    m_titleLabel->setObjectName(QStringLiteral("floatingOverlayTitle"));

    m_subtitleLabel = new QLabel(QStringLiteral("Aguardando métricas"), titleColumn);
    m_subtitleLabel->setObjectName(QStringLiteral("floatingOverlaySubtitle"));

    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(m_subtitleLabel);

    m_modeBadge = new QLabel(headerRow);
    m_modeBadge->setObjectName(QStringLiteral("floatingOverlayModeBadge"));

    headerLayout->addWidget(titleColumn, 1);
    headerLayout->addWidget(m_modeBadge, 0, Qt::AlignTop);

    auto* metricsHost = new QWidget(surface);
    auto* metricsLayout = new QGridLayout(metricsHost);
    metricsLayout->setContentsMargins(0, 0, 0, 0);
    metricsLayout->setHorizontalSpacing(8);
    metricsLayout->setVerticalSpacing(8);
    m_metricsLayout = metricsLayout;

    surfaceLayout->addWidget(headerRow);
    surfaceLayout->addWidget(metricsHost);
    rootLayout->addWidget(surface);

    m_surface = surface;
}

void OverlayWindow::refreshContent() {
    if (!m_metricsLayout || !m_surface || !m_titleLabel || !m_subtitleLabel || !m_modeBadge) {
        return;
    }

    clearLayout(m_metricsLayout);

    m_titleLabel->setText(
        m_runtimeState.gameDetected && !m_runtimeState.detectedGameName.isEmpty()
            ? QStringLiteral("NyFusion • %1").arg(m_runtimeState.detectedGameName)
            : QStringLiteral("NyFusion Overlay")
    );

    // Subtítulo: detalhe de runtime + motor gráfico detectado
    {
        QString subtitle = m_runtimeState.detailText.isEmpty()
            ? QStringLiteral("Overlay pronto")
            : m_runtimeState.detailText;

        if (!m_runtimeState.graphicsApi.isEmpty()) {
            subtitle = QStringLiteral("%1  •  %2")
                .arg(subtitle, m_runtimeState.graphicsApi);
        }
        m_subtitleLabel->setText(subtitle);
    }

    // Badge: mostra FPS ao vivo quando disponível,
    //        "HOOK OFF" em laranja quando jogo está rodando mas hook não está ativo,
    //        senão "AO VIVO"
    if (m_runtimeState.fps > 0.1f) {
        m_modeBadge->setText(QStringLiteral("%1 FPS").arg(qRound(m_runtimeState.fps)));
        m_modeBadge->setStyleSheet(QStringLiteral("color: #39D353;"));
    } else if (m_runtimeState.gameDetected && !m_runtimeState.fpsHookActive) {
        m_modeBadge->setText(QStringLiteral("HOOK OFF"));
        m_modeBadge->setStyleSheet(QStringLiteral("color: #F0A830; font-weight: bold;"));
    } else {
        m_modeBadge->setText(QStringLiteral("AO VIVO"));
        m_modeBadge->setStyleSheet(QString());
    }

    const int alpha = qRound((m_settings->backgroundOpacity() / 100.0) * 255.0);
    m_surface->setStyleSheet(QString(
        "QFrame#floatingOverlaySurface {"
        "background-color: rgba(7, 14, 24, %1);"
        "border: 1px solid rgba(22, 42, 60, 235);"
        "border-radius: 14px;"
        "}"
    ).arg(alpha));

    const QStringList selectedMetricIds = m_settings->selectedMetricIds();
    int metricIndex = 0;
    for (const QString& metricId : selectedMetricIds) {
        const auto displayType = m_settings->metricDisplayType(metricId);
        auto* card = new QFrame(m_surface);
        card->setObjectName(QStringLiteral("floatingOverlayMetricCard"));

        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(10, 9, 10, 9);
        cardLayout->setSpacing(3);

        auto* metricTitle = new QLabel(metricTitleForId(metricId), card);
        metricTitle->setObjectName(QStringLiteral("floatingOverlayMetricTitle"));
        metricTitle->setVisible(m_settings->showLabels());

        const QString rawValue = currentMetricValue(metricId);
        auto* metricValue = new QLabel(formattedMetricValue(rawValue), card);
        metricValue->setObjectName(QStringLiteral("floatingOverlayMetricValue"));
        metricValue->setStyleSheet(QStringLiteral("color: %1;").arg(metricAccentColor(metricId)));

        QFont valueFont = metricValue->font();
        switch (displayType) {
        case ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::NumberOnly:
            valueFont.setPointSize(15);
            valueFont.setBold(true);
            break;
        case ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::ChartOnly:
            valueFont.setPointSize(10);
            valueFont.setBold(false);
            break;
        case ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::NumberAndChart:
            valueFont.setPointSize(14);
            valueFont.setBold(true);
            break;
        }
        metricValue->setFont(valueFont);

        cardLayout->addWidget(metricTitle);
        if (displayType != ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::ChartOnly) {
            cardLayout->addWidget(metricValue);
        }

        if (displayType != ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::NumberOnly) {
            auto* trendLabel = new QLabel(trendString(metricId), card);
            trendLabel->setObjectName(QStringLiteral("floatingOverlayMetricTrend"));
            cardLayout->addWidget(trendLabel);
        }

        const int columnCount = 2;
        const int row = metricIndex / columnCount;
        const int column = metricIndex % columnCount;
        m_metricsLayout->addWidget(card, row, column);
        ++metricIndex;
    }

    if (metricIndex == 0) {
        auto* emptyLabel = new QLabel(QStringLiteral("Selecione métricas no menu OVER."), m_surface);
        emptyLabel->setObjectName(QStringLiteral("floatingOverlaySubtitle"));
        m_metricsLayout->addWidget(emptyLabel, 0, 0);
    }

    // ── Banner de configuração do hook (visível quando jogo detectado mas hook OFF) ──
    // Encontra ou cria o banner abaixo da grade de métricas
    const bool needsHookBanner = m_runtimeState.gameDetected
                              && !m_runtimeState.fpsHookActive
                              && !m_runtimeState.fpsHookPath.isEmpty();

    // Remove banner anterior se existir
    if (auto* old = m_surface->findChild<QLabel*>(QStringLiteral("hookHintBanner"))) {
        old->deleteLater();
    }

    if (needsHookBanner) {
        auto* surfaceLayout = qobject_cast<QVBoxLayout*>(m_surface->layout());
        if (surfaceLayout) {
            const QString hintText = QStringLiteral(
                "⚠ FPS indisponível — adicione nas opções do Steam:\n"
                "LD_PRELOAD=%1 %%command%%"
            ).arg(m_runtimeState.fpsHookPath);

            auto* hintLabel = new QLabel(hintText, m_surface);
            hintLabel->setObjectName(QStringLiteral("hookHintBanner"));
            hintLabel->setWordWrap(true);
            hintLabel->setStyleSheet(
                QStringLiteral("color: #F0A830; font-size: 9px; padding: 4px 2px 2px 2px;")
            );
            surfaceLayout->addWidget(hintLabel);
        }
    }

    adjustSize();
}

void OverlayWindow::syncVisibility() {
    const bool shouldRemainMapped = m_runtimeState.configuredEnabled;
    const bool shouldBeVisible =
        m_runtimeState.overlayActive && !m_settings->selectedMetricIds().isEmpty();

    if (shouldRemainMapped && !m_isMapped) {
        if (m_surface) {
            m_surface->setVisible(false);
        }
        move(-10000, -10000);
        show();
        m_isMapped = true;
    }

    if (!shouldRemainMapped) {
        m_raiseTimer.stop();
        if (isVisible() || m_isMapped) {
            hide();
        }
        m_isMapped = false;
        return;
    }

    applyPassiveVisibility(shouldBeVisible);
}

void OverlayWindow::applyPassiveVisibility(const bool active) {
    if (!m_isMapped && !active) {
        return;
    }

    if (active) {
        if (!isVisible()) {
            show();
            m_isMapped = true;
        }
        if (m_surface) {
            m_surface->setVisible(true);
        }
        adjustSize();
        moveToConfiguredPosition();
        raise();
        if (windowHandle()) {
            windowHandle()->raise();
        }
        if (!m_raiseTimer.isActive()) {
            m_raiseTimer.start();
        }
        return;
    }

    // Overlay inativo: para o timer e empurra para fora da tela
    m_raiseTimer.stop();
    if (m_surface) {
        m_surface->setVisible(false);
    }
    resize(1, 1);
    move(-10000, -10000);
}

QScreen* OverlayWindow::targetScreen() const {
    if (windowHandle() && windowHandle()->screen()) {
        return windowHandle()->screen();
    }

    if (QScreen* cursorScreen = QGuiApplication::screenAt(QCursor::pos())) {
        return cursorScreen;
    }

    return QGuiApplication::primaryScreen();
}

void OverlayWindow::moveToConfiguredPosition() {
    QScreen* screen = targetScreen();
    if (!screen) {
        return;
    }

    adjustSize();

    // Usa a geometria física total da tela: o jogo em fullscreen ocupa screen->geometry(),
    // não screen->availableGeometry() (que exclui taskbar/painéis do OS).
    const QRect available = screen->geometry();
    const int overlayWidth = width();
    const int overlayHeight = height();

    QPoint position = available.topLeft() + QPoint(kOverlayMargin, kOverlayMargin);
    switch (m_settings->overlayPosition()) {
    case ny::ui::viewmodels::OverlaySettingsViewModel::OverlayPosition::TopLeft:
        position = QPoint(
            available.x() + kOverlayMargin,
            available.y() + kOverlayMargin
        );
        break;
    case ny::ui::viewmodels::OverlaySettingsViewModel::OverlayPosition::TopRight:
        position = QPoint(
            available.x() + available.width() - overlayWidth - kOverlayMargin,
            available.y() + kOverlayMargin
        );
        break;
    case ny::ui::viewmodels::OverlaySettingsViewModel::OverlayPosition::BottomLeft:
        position = QPoint(
            available.x() + kOverlayMargin,
            available.y() + available.height() - overlayHeight - kOverlayMargin
        );
        break;
    case ny::ui::viewmodels::OverlaySettingsViewModel::OverlayPosition::BottomRight:
        position = QPoint(
            available.x() + available.width() - overlayWidth - kOverlayMargin,
            available.y() + available.height() - overlayHeight - kOverlayMargin
        );
        break;
    }

    move(position);
}

void OverlayWindow::updateHistory() {
    const auto metricDefinitions = ny::ui::viewmodels::OverlaySettingsViewModel::availableMetrics();
    for (const auto& definition : metricDefinitions) {
        const std::optional<float> numericValue = parseNumericValue(currentMetricValue(definition.id));
        if (!numericValue.has_value()) {
            continue;
        }

        QVector<float>& history = m_metricHistory[definition.id];
        history.push_back(numericValue.value());
        while (history.size() > kMaxHistoryPoints) {
            history.removeFirst();
        }
    }
}

QString OverlayWindow::currentMetricValue(const QString& metricId) const {
    const auto gpu = !m_gpuData.isEmpty()
        ? m_gpuData.first()
        : ny::ui::viewmodels::DashboardViewModel::GpuViewData{};

    if (metricId == QStringLiteral("cpu_usage"))    return m_cpuData.usage;
    if (metricId == QStringLiteral("cpu_temp"))     return m_cpuData.temperature;
    if (metricId == QStringLiteral("cpu_clock"))    return m_cpuData.frequency;
    if (metricId == QStringLiteral("gpu_usage"))    return gpu.usage;
    if (metricId == QStringLiteral("gpu_temp"))     return gpu.temperature;
    if (metricId == QStringLiteral("gpu_clock"))    return gpu.frequency;
    if (metricId == QStringLiteral("ram_usage"))    return m_memoryData.usage;
    if (metricId == QStringLiteral("ram_available"))return m_memoryData.available;

    // ── Métricas de frame ─────────────────────────────────────────────────────
    if (metricId == QStringLiteral("fps")) {
        if (m_runtimeState.fps > 0.1f)
            return QStringLiteral("%1 FPS").arg(qRound(m_runtimeState.fps));
        if (m_runtimeState.gameDetected && !m_runtimeState.fpsHookActive)
            return QStringLiteral("Hook OFF");
        return QStringLiteral("—");
    }
    if (metricId == QStringLiteral("frame_time")) {
        if (m_runtimeState.frameTimeMs > 0.1f)
            return QStringLiteral("%1 ms").arg(QString::number(m_runtimeState.frameTimeMs, 'f', 1));
        if (m_runtimeState.gameDetected && !m_runtimeState.fpsHookActive)
            return QStringLiteral("Hook OFF");
        return QStringLiteral("—");
    }
    if (metricId == QStringLiteral("graphics_api")) {
        return m_runtimeState.graphicsApi.isEmpty()
            ? QStringLiteral("Aguardando…")
            : m_runtimeState.graphicsApi;
    }

    return QStringLiteral("Indisponível");
}

QString OverlayWindow::formattedMetricValue(const QString& rawValue) const {
    if (rawValue.isEmpty()) {
        return QStringLiteral("—");
    }


    return rawValue;
}

QString OverlayWindow::trendString(const QString& metricId) const {
    static const QString glyphs = QStringLiteral("▁▂▃▄▅▆▇█");

    const QVector<float> history = m_metricHistory.value(metricId);
    if (history.isEmpty()) {
        return QStringLiteral("▁▁▂▃▂▁");
    }

    const auto [minIt, maxIt] = std::minmax_element(history.cbegin(), history.cend());
    const float minValue = *minIt;
    const float maxValue = *maxIt;
    const float span = std::max(0.001f, maxValue - minValue);

    QString trend;
    trend.reserve(history.size());
    for (const float value : history) {
        const float normalized = (value - minValue) / span;
        const int glyphIndex = qBound(0, qRound(normalized * (glyphs.size() - 1)), glyphs.size() - 1);
        trend.append(glyphs.at(glyphIndex));
    }
    return trend;
}


std::optional<float> OverlayWindow::parseNumericValue(const QString& rawValue) {
    static const QRegularExpression re(QStringLiteral(R"([-+]?\d+(?:[\.,]\d+)?)"));
    const auto match = re.match(rawValue);
    if (!match.hasMatch()) {
        return std::nullopt;
    }

    bool ok = false;
    const float value = match.captured(0).replace(',', '.').toFloat(&ok);
    if (!ok) {
        return std::nullopt;
    }
    return value;
}

QString OverlayWindow::metricAccentColor(const QString& metricId) {
    if (metricId.startsWith(QStringLiteral("cpu_")))      return QStringLiteral("#FF9A00"); // laranja
    if (metricId.startsWith(QStringLiteral("gpu_")))      return QStringLiteral("#00CCEE"); // ciano
    if (metricId == QStringLiteral("fps"))                return QStringLiteral("#39D353"); // verde
    if (metricId == QStringLiteral("frame_time"))         return QStringLiteral("#39D353"); // verde
    if (metricId == QStringLiteral("graphics_api"))       return QStringLiteral("#7FD6FF"); // azul claro
    return QStringLiteral("#B78CFF");                                                        // roxo (RAM etc.)
}

} // namespace ny::ui::qt



