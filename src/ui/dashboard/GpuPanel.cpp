/**
 * @file GpuPanel.cpp
 * @author Marcos Henrique
 * @date 03/04/2026
 */
#include "GpuPanel.hpp"
#include "PanelLayoutMetrics.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QSlider>
#include <QVBoxLayout>

#include "widgets/CircularGauge.hpp"
#include "widgets/SparklineWidget.hpp"

namespace ny::ui::dashboard {

using CG = ny::ui::widgets::CircularGauge;
using SW = ny::ui::widgets::SparklineWidget;

// ── Helper ────────────────────────────────────────────────────────────────────
float GpuPanel::parseFirstFloat(const QString& s, float fallback) {
    static QRegularExpression re(R"([-+]?\d+\.?\d*)");
    auto m = re.match(s);
    if (m.hasMatch()) {
        bool ok = false;
        float v = m.captured().toFloat(&ok);
        if (ok) return v;
    }
    return fallback;
}

// ── Slider row builder ────────────────────────────────────────────────────────
QWidget* GpuPanel::buildSliderRow(const QString& label, QSlider*& slider,
                                   QLabel*& valLabel, int minVal, int maxVal,
                                   int defaultVal, const QString& suffix)
{
    auto* row = new QWidget;
    auto* hl  = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(6);

    auto* lbl = new QLabel(label);
    lbl->setObjectName("sliderLabel");
    lbl->setFixedWidth(82);  // reduzido de 108 para equiparar largura mínima com CpuPanel

    slider = new QSlider(Qt::Horizontal);
    slider->setRange(minVal, maxVal);
    slider->setValue(defaultVal);
    slider->setObjectName("tuningSlider");

    valLabel = new QLabel;
    valLabel->setObjectName("sliderValue");
    valLabel->setFixedWidth(44);  // reduzido de 56: "+300 MHz" cabe em ~44px (9-10px font)
    valLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto updateVal = [valLabel, slider, suffix]() {
        int v = slider->value();
        QString txt = (v >= 0 ? "+" : "") + QString::number(v) + " " + suffix;
        valLabel->setText(txt);
    };
    updateVal();
    QObject::connect(slider, &QSlider::valueChanged, row, updateVal);

    hl->addWidget(lbl);
    hl->addWidget(slider, 1);
    hl->addWidget(valLabel);
    return row;
}

// ── Constructor ───────────────────────────────────────────────────────────────
GpuPanel::GpuPanel(QWidget* parent)
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

    auto* tagLabel = new QLabel("GPU");
    tagLabel->setObjectName("panelTag");

    m_nameLabel = new QLabel("—");
    m_nameLabel->setObjectName("panelName");

    // Settings icon (top-right of panel header)
    auto* settingsIcon = new QLabel("⇋");
    settingsIcon->setObjectName("panelMenuIcon");

    headerLayout->addWidget(tagLabel);
    headerLayout->addWidget(m_nameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(settingsIcon);
    rootLayout->addWidget(header);

    // ── Gauge section: gauge no topo + FAN/MEMORY abaixo ───────────────
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

    // ── Gauge circular (topo, centralizado) ──────────────────────────
    m_gauge = new CG;
    m_gauge->setGaugeColor(QColor(0x00, 0xdd, 0xff));
    m_gauge->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_gauge->setMinimumSize(160, 160);

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

    // ── Linha de métricas abaixo do gauge: [FAN] [MEMORY] ────────────
    auto* metricsRow = new QWidget;
    metricsRow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    metricsRow->setFixedHeight(PanelLayoutMetrics::kInfoRowHeight);
    auto* metricsRowLayout = new QHBoxLayout(metricsRow);
    metricsRowLayout->setContentsMargins(0, 4, 0, 2);
    metricsRowLayout->setSpacing(20);

    // Bloco FAN (esquerda)
    auto* fanBlock = new QWidget;
    fanBlock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* fanBlockLayout = new QVBoxLayout(fanBlock);
    fanBlockLayout->setContentsMargins(0, 0, 0, 0);
    fanBlockLayout->setSpacing(3);

    m_fanTitleLabel = new QLabel("FAN");
    m_fanTitleLabel->setObjectName("metricTitle");
    m_fanTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_fanValueLabel = new QLabel("—");
    m_fanValueLabel->setObjectName("metricValue");
    m_fanValueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_fanRpmTitleLabel = new QLabel("");
    m_fanRpmTitleLabel->setFixedHeight(0);
    m_fanRpmValueLabel = new QLabel("—");
    m_fanRpmValueLabel->setObjectName("metricValue");
    m_fanRpmValueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    fanBlockLayout->addWidget(m_fanTitleLabel);
    fanBlockLayout->addWidget(m_fanValueLabel);
    fanBlockLayout->addWidget(m_fanRpmValueLabel);
    fanBlockLayout->addStretch();

    // Bloco MEMORY (direita)
    auto* memoryBlock = new QWidget;
    memoryBlock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* memoryBlockLayout = new QVBoxLayout(memoryBlock);
    memoryBlockLayout->setContentsMargins(0, 0, 0, 0);
    memoryBlockLayout->setSpacing(3);

    m_memTitleLabel = new QLabel("MEMORY");
    m_memTitleLabel->setObjectName("metricTitle");
    m_memTitleLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_memValueLabel = new QLabel("—");
    m_memValueLabel->setObjectName("metricValue");
    m_memValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_memFreqTitleLabel = new QLabel("");
    m_memFreqTitleLabel->setFixedHeight(0);
    m_memFreqValueLabel = new QLabel("Indisponível");
    m_memFreqValueLabel->setObjectName("metricValue");
    m_memFreqValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    memoryBlockLayout->addWidget(m_memTitleLabel);
    memoryBlockLayout->addWidget(m_memValueLabel);
    memoryBlockLayout->addWidget(m_memFreqValueLabel);
    memoryBlockLayout->addStretch();

    metricsRowLayout->addWidget(fanBlock, 1, Qt::AlignLeft);
    metricsRowLayout->addWidget(memoryBlock, 1, Qt::AlignRight);
    gaugeSectionLayout->addWidget(metricsRow);
    m_infoRowContainer = metricsRow;

    // m_pwrTitleLabel / m_pwrValueLabel: criados mas não inseridos no layout
    m_pwrTitleLabel = new QLabel("POWER");
    m_pwrValueLabel = new QLabel("—");
    m_pwrTitleLabel->setObjectName("metricTitle");
    m_pwrValueLabel->setObjectName("metricValue");

    // ── Sparkline: GPU Usage (%) — largura total abaixo do gauge ─────
    auto* sparkContainer = new QWidget;
    sparkContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sparkContainer->setFixedHeight(PanelLayoutMetrics::kUsageContainerHeight);
    auto* sparkContainerLayout = new QVBoxLayout(sparkContainer);
    sparkContainerLayout->setContentsMargins(0, 8, 0, 0);
    sparkContainerLayout->setSpacing(3);

    auto* sparkTitle = new QLabel("GPU USAGE");
    sparkTitle->setObjectName("metricTitle");

    m_usageSpark = new SW;
    m_usageSpark->setColor(QColor(0x00, 0xEE, 0xBB));
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

    // ── Tuning sliders ────────────────────────────────────────────────────
    auto* tuningWidget = new QWidget;
    auto* tuningLayout = new QVBoxLayout(tuningWidget);
    tuningLayout->setContentsMargins(0, 2, 0, 2);
    tuningLayout->setSpacing(6);

    tuningLayout->addWidget(buildSliderRow("CORE CLOCK", m_coreSlider,  m_coreVal,  -200, 300,  0, "MHz"));
    tuningLayout->addWidget(buildSliderRow("MEMORY CLOCK",   m_memSlider,   m_memVal,   -500, 1000, 0, "MHz"));
    tuningLayout->addWidget(buildSliderRow("VOLTAGE",        m_voltSlider,  m_voltVal,  -50,  50,   0, "mV"));
    tuningLayout->addWidget(buildSliderRow("POWER LIMIT",    m_powerSlider, m_powerVal, 50,   130, 100, "%"));

    // Buttons row
    auto* btnRow = new QWidget;
    auto* btnLayout = new QHBoxLayout(btnRow);
    btnLayout->setContentsMargins(0, 4, 0, 0);
    btnLayout->setSpacing(8);

    auto* applyBtn = new QPushButton("APPLY");
    applyBtn->setObjectName("applyBtn");
    applyBtn->setFixedHeight(32);

    auto* resetBtn = new QPushButton("RESET");
    resetBtn->setObjectName("resetBtn");
    resetBtn->setFixedHeight(32);

    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        m_coreSlider->setValue(0);
        m_memSlider->setValue(0);
        m_voltSlider->setValue(0);
        m_powerSlider->setValue(100);
    });

    btnLayout->addWidget(applyBtn, 1);
    btnLayout->addWidget(resetBtn, 1);
    tuningLayout->addWidget(btnRow);

    rootLayout->addWidget(tuningWidget, 3);
}

// ── updateData ────────────────────────────────────────────────────────────────
void GpuPanel::updateData(const ny::ui::viewmodels::DashboardViewModel::GpuViewData& gpu) {
    m_nameLabel->setText(gpu.name);

    // Gauge: arc = usage %, primary = temperature, secondary = frequency
    const float usagePct = parseFirstFloat(gpu.usage, 0.0f);
    m_gauge->setPercent(usagePct);

    // Primary: temperature
    const float tempVal = parseFirstFloat(gpu.temperature, 0.0f);
    m_gauge->setPrimaryTitle("TEMPERATURE");
    m_gauge->setPrimaryValue(tempVal > 0.0f
        ? QString::number(tempVal, 'f', 1) + "\u00B0C"
        : "—");

    // Secondary: clock
    const float freqVal = parseFirstFloat(gpu.frequency, 0.0f);
    m_gauge->setSecondaryTitle("CLOCK");
    m_gauge->setSecondaryValue(freqVal > 0.0f
        ? QString::number(static_cast<int>(freqVal))
        : "—");
    m_gauge->setSecondaryUnit("MHz");

    // Left metrics: FAN % + FAN RPM
    // Se indisponível ou vazio, exibir "0 %" / "0 RPM" (fan pode estar parada)
    const bool fanAvailable = !gpu.fanRpm.isEmpty()
        && gpu.fanRpm.compare(QStringLiteral("Indisponivel"), Qt::CaseInsensitive) != 0
        && gpu.fanRpm.compare(QStringLiteral("Indisponível"), Qt::CaseInsensitive) != 0;
    m_fanValueLabel->setText(fanAvailable ? gpu.fanRpm : QStringLiteral("0 %"));
    m_fanRpmValueLabel->setText(fanAvailable ? gpu.fanRpm : QStringLiteral("0 RPM"));

    // Right metrics: MEMORY (vram) + MEM FREQ (indisponível no backend)
    m_memValueLabel->setText(gpu.vramUsed);
    m_memFreqValueLabel->setText(QStringLiteral("Indisponível"));

    m_pwrValueLabel->setText(gpu.power);  // stored but not displayed in layout

    // Sparkline: apenas uso da GPU
    if (usagePct > 0.0f) m_usageSpark->addPoint(usagePct);
}

int GpuPanel::gaugeContainerHeightHint() const {
    if (!m_gaugeContainer) return 0;
    return (m_gaugeContainer->minimumHeight() == m_gaugeContainer->maximumHeight())
        ? m_gaugeContainer->minimumHeight()
        : m_gaugeContainer->height();
}

int GpuPanel::infoRowContainerHeightHint() const {
    if (!m_infoRowContainer) return 0;
    return (m_infoRowContainer->minimumHeight() == m_infoRowContainer->maximumHeight())
        ? m_infoRowContainer->minimumHeight()
        : m_infoRowContainer->height();
}

int GpuPanel::usageGraphContainerHeightHint() const {
    if (!m_usageGraphContainer) return 0;
    return (m_usageGraphContainer->minimumHeight() == m_usageGraphContainer->maximumHeight())
        ? m_usageGraphContainer->minimumHeight()
        : m_usageGraphContainer->height();
}

int GpuPanel::usageGraphRelativeIndex() const {
    if (!m_topSectionLayout || !m_usageGraphContainer) return -1;
    return m_topSectionLayout->indexOf(m_usageGraphContainer);
}

bool GpuPanel::isUsageImmediatelyAfterInfoRow() const {
    if (!m_topSectionLayout || !m_infoRowContainer || !m_usageGraphContainer) return false;
    const int infoIdx = m_topSectionLayout->indexOf(m_infoRowContainer);
    const int usageIdx = m_topSectionLayout->indexOf(m_usageGraphContainer);
    return usageIdx == infoIdx + 1;
}

bool GpuPanel::hasTopBlockOrder() const {
    if (!m_topSectionLayout || !m_gaugeContainer || !m_infoRowContainer || !m_usageGraphContainer) return false;
    const int gaugeIdx = m_topSectionLayout->indexOf(m_gaugeContainer);
    const int infoIdx = m_topSectionLayout->indexOf(m_infoRowContainer);
    const int usageIdx = m_topSectionLayout->indexOf(m_usageGraphContainer);
    return gaugeIdx == 0 && infoIdx == 1 && usageIdx == 2;
}

} // namespace ny::ui::dashboard
