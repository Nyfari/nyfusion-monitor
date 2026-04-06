/**
 * @file CpuPanel.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */
#include "CpuPanel.hpp"
#include "PanelLayoutMetrics.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QVBoxLayout>

#include "widgets/CircularGauge.hpp"
#include "widgets/LoadBarWidget.hpp"
#include "widgets/SparklineWidget.hpp"

namespace ny::ui::dashboard {

using CG  = ny::ui::widgets::CircularGauge;
using SW  = ny::ui::widgets::SparklineWidget;
using LBW = ny::ui::widgets::LoadBarWidget;

float CpuPanel::parseFirstFloat(const QString& s, float fallback) {
    static QRegularExpression re(R"([-+]?\d+\.?\d*)");
    auto m = re.match(s);
    if (m.hasMatch()) {
        bool ok = false;
        float v = m.captured().toFloat(&ok);
        if (ok) return v;
    }
    return fallback;
}

CpuPanel::CpuPanel(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("panelFrame");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(14, 12, 14, 12);
    rootLayout->setSpacing(6);

    // ── Header ────────────────────────────────────────────────────────────
    auto* header = new QWidget;
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    header->setFixedHeight(PanelLayoutMetrics::kHeaderContainerHeight);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(6);

    auto* tagLabel = new QLabel("CPU");
    tagLabel->setObjectName("panelTagCpu");

    m_nameLabel = new QLabel("—");
    m_nameLabel->setObjectName("panelName");

    // Settings icon (top-right of panel header)
    auto* settingsIcon = new QLabel("⇋");
    settingsIcon->setObjectName("panelMenuIconCpu");

    headerLayout->addWidget(tagLabel);
    headerLayout->addWidget(m_nameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(settingsIcon);
    rootLayout->addWidget(header);

    // ── Gauge section: gauge no topo + FAN/FREQUENCY abaixo ───────────
    auto* gaugeSection = new QWidget;
    gaugeSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto* gaugeSectionLayout = new QVBoxLayout(gaugeSection);
    gaugeSectionLayout->setContentsMargins(0, 0, 0, 0);
    gaugeSectionLayout->setSpacing(PanelLayoutMetrics::kTopSectionSpacing);
    const int topSectionHeight =
        PanelLayoutMetrics::kGaugeContainerHeight +
        PanelLayoutMetrics::kInfoRowHeight +
        PanelLayoutMetrics::kUsageContainerHeight +
        (PanelLayoutMetrics::kTopSectionSpacing * 2);
    gaugeSection->setFixedHeight(topSectionHeight);
    m_topSectionLayout = gaugeSectionLayout;

    // ── Gauge circular (elemento dominante) ──────────────────────────
    m_gauge = new CG;
    m_gauge->setGaugeColor(QColor(0xFF, 0x9A, 0x00));
    m_gauge->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_gauge->setMinimumSize(160, 160);

    // m_usageTitleLabel / m_usageValueLabel: created but shown in gauge/bar instead
    m_usageTitleLabel = new QLabel("USAGE");
    m_usageValueLabel = new QLabel("—");
    m_usageTitleLabel->setObjectName("metricTitle");
    m_usageValueLabel->setObjectName("metricValueCpu");

    auto* gaugeContainer = new QWidget;
    gaugeContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    gaugeContainer->setFixedHeight(PanelLayoutMetrics::kGaugeContainerHeight);
    auto* gaugeContainerLayout = new QHBoxLayout(gaugeContainer);
    gaugeContainerLayout->setContentsMargins(0, 0, 0, 0);
    gaugeContainerLayout->setSpacing(0);
    gaugeContainerLayout->addStretch();
    gaugeContainerLayout->addWidget(m_gauge, 0, Qt::AlignCenter);
    gaugeContainerLayout->addStretch();
    gaugeSectionLayout->addWidget(gaugeContainer);
    m_gaugeContainer = gaugeContainer;

    // ── Linha de métricas abaixo do gauge: [FREQUENCY] [FAN SPEED] ───
    auto* metricsRow = new QWidget;
    metricsRow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    metricsRow->setFixedHeight(PanelLayoutMetrics::kInfoRowHeight);
    auto* metricsRowLayout = new QHBoxLayout(metricsRow);
    metricsRowLayout->setContentsMargins(0, 4, 0, 2);
    metricsRowLayout->setSpacing(20);

    // Bloco FREQUENCY (esquerda)
    auto* freqBlock = new QWidget;
    freqBlock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* freqBlockLayout = new QVBoxLayout(freqBlock);
    freqBlockLayout->setContentsMargins(0, 0, 0, 0);
    freqBlockLayout->setSpacing(3);

    m_voltTitleLabel = new QLabel("FREQUENCY");
    m_voltTitleLabel->setObjectName("metricTitle");
    m_voltTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_voltValueLabel = new QLabel("—");
    m_voltValueLabel->setObjectName("metricValueCpu");
    m_voltValueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    freqBlockLayout->addWidget(m_voltTitleLabel);
    freqBlockLayout->addWidget(m_voltValueLabel);
    freqBlockLayout->addStretch();

    // Bloco FAN SPEED (direita)
    auto* fanBlock = new QWidget;
    fanBlock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* fanBlockLayout = new QVBoxLayout(fanBlock);
    fanBlockLayout->setContentsMargins(0, 0, 0, 0);
    fanBlockLayout->setSpacing(3);

    m_fanTitleLabel = new QLabel("FAN SPEED");
    m_fanTitleLabel->setObjectName("metricTitle");
    m_fanTitleLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_fanValueLabel = new QLabel("—");
    m_fanValueLabel->setObjectName("metricValueCpu");
    m_fanValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    fanBlockLayout->addWidget(m_fanTitleLabel);
    fanBlockLayout->addWidget(m_fanValueLabel);
    fanBlockLayout->addStretch();

    metricsRowLayout->addWidget(freqBlock, 1, Qt::AlignLeft);
    metricsRowLayout->addWidget(fanBlock, 1, Qt::AlignRight);
    gaugeSectionLayout->addWidget(metricsRow);
    m_infoRowContainer = metricsRow;

    // ── Sparkline: CPU Usage (%) — largura total abaixo do gauge ─────
    auto* sparkContainer = new QWidget;
    sparkContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sparkContainer->setFixedHeight(PanelLayoutMetrics::kUsageContainerHeight);
    auto* sparkContainerLayout = new QVBoxLayout(sparkContainer);
    sparkContainerLayout->setContentsMargins(0, 8, 0, 0);
    sparkContainerLayout->setSpacing(3);

    auto* sparkTitle = new QLabel("CPU USAGE");
    sparkTitle->setObjectName("metricTitle");

    m_usageSpark = new SW;
    m_usageSpark->setColor(QColor(0xFF, 0xCC, 0x44));
    m_usageSpark->setMaxPoints(60);
    m_usageSpark->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_usageSpark->setFixedHeight(64);

    sparkContainerLayout->addWidget(sparkTitle);
    sparkContainerLayout->addWidget(m_usageSpark);
    gaugeSectionLayout->addWidget(sparkContainer);
    m_usageGraphContainer = sparkContainer;

    rootLayout->addWidget(gaugeSection);

    // ── Separator ─────────────────────────────────────────────────────────
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("panelSeparator");
    rootLayout->addWidget(sep);

    // ── Load bar ──────────────────────────────────────────────────────────
    auto* loadSection = new QWidget;
    auto* loadLayout  = new QVBoxLayout(loadSection);
    loadLayout->setContentsMargins(0, 2, 0, 2);
    loadLayout->setSpacing(5);

    auto* loadHeader = new QWidget;
    auto* loadHeaderLayout = new QHBoxLayout(loadHeader);
    loadHeaderLayout->setContentsMargins(0, 0, 0, 0);

    auto* loadTitle = new QLabel("LOAD");
    loadTitle->setObjectName("metricTitle");

    m_loadPct = new QLabel("0%");
    m_loadPct->setObjectName("loadPercent");

    loadHeaderLayout->addWidget(loadTitle);
    loadHeaderLayout->addStretch();
    loadHeaderLayout->addWidget(m_loadPct);
    loadLayout->addWidget(loadHeader);

    m_loadBar = new LBW;
    m_loadBar->setBarColor(QColor(0xFF, 0x9A, 0x00));
    m_loadBar->setFixedHeight(16);
    loadLayout->addWidget(m_loadBar);
    rootLayout->addWidget(loadSection);

    // ── Info cards row (RAM + System Power) ──────────────────────────────
    auto* cardsRow = new QWidget;
    auto* cardsLayout = new QHBoxLayout(cardsRow);
    cardsLayout->setContentsMargins(0, 0, 0, 0);
    cardsLayout->setSpacing(6);

    // RAM card
    auto* ramCard = new QFrame;
    ramCard->setObjectName("infoCard");
    auto* ramLayout = new QVBoxLayout(ramCard);
    ramLayout->setContentsMargins(10, 8, 10, 8);
    ramLayout->setSpacing(3);

    auto* ramTitle = new QLabel("RAM");
    ramTitle->setObjectName("metricTitle");
    m_ramLabel = new QLabel("— GB");
    m_ramLabel->setObjectName("infoCardValue");
    m_ramLabel->setWordWrap(true);

    ramLayout->addWidget(ramTitle);
    ramLayout->addWidget(m_ramLabel);

    // System Power card
    auto* pwrCard = new QFrame;
    pwrCard->setObjectName("infoCard");
    auto* pwrLayout = new QVBoxLayout(pwrCard);
    pwrLayout->setContentsMargins(10, 8, 10, 8);
    pwrLayout->setSpacing(3);

    auto* pwrTitle = new QLabel("SYSTEM POWER");
    pwrTitle->setObjectName("metricTitle");
    m_powerLabel = new QLabel("— %");
    m_powerLabel->setObjectName("infoCardValueBig");

    pwrLayout->addWidget(pwrTitle);
    pwrLayout->addWidget(m_powerLabel);

    cardsLayout->addWidget(ramCard, 1);
    cardsLayout->addWidget(pwrCard, 1);
    rootLayout->addWidget(cardsRow, 0);
}

void CpuPanel::updateData(
    const ny::ui::viewmodels::DashboardViewModel::CpuViewData&    cpu,
    const ny::ui::viewmodels::DashboardViewModel::MemoryViewData& mem)
{
    m_nameLabel->setText(cpu.name);

    // Gauge arc = CPU usage
    const float usagePct = parseFirstFloat(cpu.usage, 0.0f);
    m_gauge->setPercent(usagePct);

    // Primary: temperature
    const float tempVal = parseFirstFloat(cpu.temperature, 0.0f);
    m_gauge->setPrimaryTitle("TEMPERATURE");
    m_gauge->setPrimaryValue(tempVal > 0.0f
        ? QString::number(tempVal, 'f', 1) + "\u00B0C"
        : "—");

    // Secondary: frequency
    const float freqMHz = parseFirstFloat(cpu.frequency, 0.0f);
    const float freqGHz = freqMHz / 1000.0f;
    m_gauge->setSecondaryTitle("CLOCK");
    m_gauge->setSecondaryValue(freqMHz > 0.0f
        ? QString::number(freqGHz, 'f', 1)
        : "—");
    m_gauge->setSecondaryUnit("GHz");

    m_voltValueLabel->setText(cpu.frequency);
    const bool fanAvailable = !cpu.fanRpm.isEmpty()
        && cpu.fanRpm.compare(QStringLiteral("Indisponivel"), Qt::CaseInsensitive) != 0
        && cpu.fanRpm.compare(QStringLiteral("Indisponível"), Qt::CaseInsensitive) != 0;
    m_fanValueLabel->setText(fanAvailable ? cpu.fanRpm : QStringLiteral("0"));
    m_usageValueLabel->setText(cpu.usage);

    // Sparkline: apenas uso da CPU
    if (usagePct > 0.0f) m_usageSpark->addPoint(usagePct);

    // Load bar
    m_loadBar->setPercent(usagePct);
    m_loadPct->setText(QString::number(static_cast<int>(usagePct)) + "%");

    // RAM card: multi-line
    m_ramLabel->setText(mem.total + "\n" + mem.available + "\n" + mem.usage);

    // System Power card: potência total do sistema não disponível no backend atual
    m_powerLabel->setText("\u2014");  // — (em dash), indica indisponível
}

int CpuPanel::gaugeContainerHeightHint() const {
    if (!m_gaugeContainer) return 0;
    return (m_gaugeContainer->minimumHeight() == m_gaugeContainer->maximumHeight())
        ? m_gaugeContainer->minimumHeight()
        : m_gaugeContainer->height();
}

int CpuPanel::infoRowContainerHeightHint() const {
    if (!m_infoRowContainer) return 0;
    return (m_infoRowContainer->minimumHeight() == m_infoRowContainer->maximumHeight())
        ? m_infoRowContainer->minimumHeight()
        : m_infoRowContainer->height();
}

int CpuPanel::usageGraphContainerHeightHint() const {
    if (!m_usageGraphContainer) return 0;
    return (m_usageGraphContainer->minimumHeight() == m_usageGraphContainer->maximumHeight())
        ? m_usageGraphContainer->minimumHeight()
        : m_usageGraphContainer->height();
}

int CpuPanel::usageGraphRelativeIndex() const {
    if (!m_topSectionLayout || !m_usageGraphContainer) return -1;
    return m_topSectionLayout->indexOf(m_usageGraphContainer);
}

bool CpuPanel::isUsageImmediatelyAfterInfoRow() const {
    if (!m_topSectionLayout || !m_infoRowContainer || !m_usageGraphContainer) return false;
    const int infoIdx = m_topSectionLayout->indexOf(m_infoRowContainer);
    const int usageIdx = m_topSectionLayout->indexOf(m_usageGraphContainer);
    return usageIdx == infoIdx + 1;
}

bool CpuPanel::hasTopBlockOrder() const {
    if (!m_topSectionLayout || !m_gaugeContainer || !m_infoRowContainer || !m_usageGraphContainer) return false;
    const int gaugeIdx = m_topSectionLayout->indexOf(m_gaugeContainer);
    const int infoIdx = m_topSectionLayout->indexOf(m_infoRowContainer);
    const int usageIdx = m_topSectionLayout->indexOf(m_usageGraphContainer);
    return gaugeIdx == 0 && infoIdx == 1 && usageIdx == 2;
}

} // namespace ny::ui::dashboard
