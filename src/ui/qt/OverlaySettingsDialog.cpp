/**
 * @file OverlaySettingsDialog.cpp
 * @author Marcos Henrique
 * @date 16/05/2026
 */
#include "OverlaySettingsDialog.hpp"

#include <algorithm>

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSlider>
#include <QStyle>
#include <QVBoxLayout>

#include "viewmodels/OverlaySettingsViewModel.hpp"

namespace ny::ui::qt {
namespace {
using OverlayPosition = ny::ui::viewmodels::OverlaySettingsViewModel::OverlayPosition;

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

QString previewTrendForMetric(const QString& metricId) {
    if (metricId.contains(QStringLiteral("temp"))) {
        return QStringLiteral("▁▂▄▅▆▇▆▄");
    }
    if (metricId.contains(QStringLiteral("clock"))) {
        return QStringLiteral("▂▃▅▆▅▇▆▅");
    }
    return QStringLiteral("▁▃▄▆▇▅▄▆");
}

QString previewValueForMode(
    const ny::ui::viewmodels::OverlaySettingsViewModel::MetricDefinition& definition,
    const ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType displayType
) {
    switch (displayType) {
    case ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::NumberOnly:
        return definition.sampleValue;
    case ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::ChartOnly:
        return QString();
    case ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::NumberAndChart:
        return definition.sampleValue;
    }

    return definition.sampleValue;
}

QString metricAccentColor(const QString& metricId) {
    if (metricId.startsWith(QStringLiteral("cpu_"))) {
        return QStringLiteral("#FF9A00");
    }
    if (metricId.startsWith(QStringLiteral("gpu_"))) {
        return QStringLiteral("#00CCEE");
    }
    return QStringLiteral("#B28DFF");
}
} // namespace

OverlaySettingsDialog::OverlaySettingsDialog(QWidget* parent)
    : QDialog(parent)
    , m_settings(std::make_unique<ny::ui::viewmodels::OverlaySettingsViewModel>())
{
    setObjectName(QStringLiteral("overlaySettingsDialog"));
    setWindowTitle(QStringLiteral("Configuração do Overlay"));
    setModal(true);
    resize(980, 720);
    setMinimumSize(860, 620);

    buildUi();
    loadFromViewModel();
    wireSignals();
    refreshPreview();
    updateValidationState();
}

OverlaySettingsDialog::~OverlaySettingsDialog() = default;

void OverlaySettingsDialog::buildUi() {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(22, 22, 22, 18);
    rootLayout->setSpacing(16);

    auto* titleLabel = new QLabel(QStringLiteral("Configuração do overlay"), this);
    titleLabel->setObjectName(QStringLiteral("overlayDialogTitle"));

    rootLayout->addWidget(titleLabel);

    auto* contentRow = new QHBoxLayout;
    contentRow->setSpacing(18);
    rootLayout->addLayout(contentRow, 1);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setObjectName(QStringLiteral("overlayConfigScroll"));

    auto* formHost = new QWidget(scrollArea);
    auto* formLayout = new QVBoxLayout(formHost);
    formLayout->setContentsMargins(0, 0, 8, 0);
    formLayout->setSpacing(12);

    auto* activationLayout = new QVBoxLayout;
    activationLayout->setContentsMargins(0, 0, 0, 0);
    activationLayout->setSpacing(10);

    m_enableOverlayCheck = new QCheckBox(QStringLiteral("Ativar overlay"), formHost);
    m_enableOverlayCheck->setObjectName(QStringLiteral("overlayMetricCheck"));

    m_autoEnableOverlayCheck = new QCheckBox(QStringLiteral("Ligar automaticamente quando um jogo abrir"), formHost);
    m_autoEnableOverlayCheck->setObjectName(QStringLiteral("overlayMetricCheck"));

    activationLayout->addWidget(m_enableOverlayCheck);
    activationLayout->addWidget(m_autoEnableOverlayCheck);
    formLayout->addWidget(buildSectionCard(
        QStringLiteral("1. Ativação"),
        QStringLiteral("Defina se o overlay fica disponível e quando deve entrar em ação."),
        activationLayout
    ));

    auto* metricsLayout = new QVBoxLayout;
    metricsLayout->setContentsMargins(0, 0, 0, 0);
    metricsLayout->setSpacing(8);

    const auto metricDefinitions = ny::ui::viewmodels::OverlaySettingsViewModel::availableMetrics();
    for (const auto& definition : metricDefinitions) {
        auto* row = new QWidget(formHost);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(10);

        auto* checkbox = new QCheckBox(
            QStringLiteral("%1 — %2").arg(definition.title, definition.description),
            row
        );
        checkbox->setObjectName(QStringLiteral("overlayMetricCheck"));
        checkbox->setProperty("metricId", definition.id);
        checkbox->setToolTip(definition.description);
        m_metricChecks.insert(definition.id, checkbox);

        auto* displayCombo = new QComboBox(row);
        displayCombo->setObjectName(QStringLiteral("overlayCombo"));
        displayCombo->setMinimumWidth(170);
        for (const auto displayType : {
                 ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::NumberOnly,
                 ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::ChartOnly,
                 ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::NumberAndChart,
             }) {
            displayCombo->addItem(
                ny::ui::viewmodels::OverlaySettingsViewModel::metricDisplayTypeLabel(displayType),
                static_cast<int>(displayType)
            );
        }
        m_metricDisplayCombos.insert(definition.id, displayCombo);

        rowLayout->addWidget(checkbox, 1);
        rowLayout->addWidget(displayCombo, 0);
        metricsLayout->addWidget(row);
    }

    m_validationLabel = new QLabel(formHost);
    m_validationLabel->setObjectName(QStringLiteral("overlayValidationLabel"));
    m_validationLabel->setWordWrap(true);

    auto* metricsContainer = new QVBoxLayout;
    metricsContainer->setContentsMargins(0, 0, 0, 0);
    metricsContainer->setSpacing(10);
    metricsContainer->addLayout(metricsLayout);
    metricsContainer->addWidget(m_validationLabel);

    formLayout->addWidget(buildSectionCard(
        QStringLiteral("2. Informações exibidas"),
        QStringLiteral("Selecione somente os dados que precisam aparecer durante o jogo para manter leitura rápida."),
        metricsContainer
    ));

    auto* formatLayout = new QVBoxLayout;
    formatLayout->setContentsMargins(0, 0, 0, 0);
    formatLayout->setSpacing(12);

    auto buildRow = [formHost](const QString& labelText, QWidget* field) {
        auto* row = new QWidget(formHost);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(12);

        auto* label = new QLabel(labelText, row);
        label->setObjectName(QStringLiteral("overlayFieldLabel"));
        label->setFixedWidth(160);

        rowLayout->addWidget(label);
        rowLayout->addWidget(field, 1);
        return row;
    };

    m_positionCombo = new QComboBox(formHost);
    m_positionCombo->setObjectName(QStringLiteral("overlayCombo"));
    for (const OverlayPosition position : {
             OverlayPosition::TopLeft,
             OverlayPosition::TopRight,
             OverlayPosition::BottomLeft,
             OverlayPosition::BottomRight,
         }) {
        m_positionCombo->addItem(
            ny::ui::viewmodels::OverlaySettingsViewModel::overlayPositionLabel(position),
            static_cast<int>(position)
        );
    }

    m_showLabelsCheck = new QCheckBox(QStringLiteral("Mostrar nome das métricas"), formHost);
    m_showLabelsCheck->setObjectName(QStringLiteral("overlayMetricCheck"));

    auto* opacityField = new QWidget(formHost);
    auto* opacityLayout = new QHBoxLayout(opacityField);
    opacityLayout->setContentsMargins(0, 0, 0, 0);
    opacityLayout->setSpacing(10);

    m_opacitySlider = new QSlider(Qt::Horizontal, opacityField);
    m_opacitySlider->setObjectName(QStringLiteral("overlayOpacitySlider"));
    m_opacitySlider->setRange(35, 100);
    m_opacitySlider->setSingleStep(1);

    m_opacityValueLabel = new QLabel(opacityField);
    m_opacityValueLabel->setObjectName(QStringLiteral("overlayFieldValue"));
    m_opacityValueLabel->setFixedWidth(48);
    m_opacityValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    opacityLayout->addWidget(m_opacitySlider, 1);
    opacityLayout->addWidget(m_opacityValueLabel);

    formatLayout->addWidget(buildRow(QStringLiteral("Posição"), m_positionCombo));
    formatLayout->addWidget(buildRow(QStringLiteral("Rótulos"), m_showLabelsCheck));
    formatLayout->addWidget(buildRow(QStringLiteral("Opacidade"), opacityField));

    formLayout->addWidget(buildSectionCard(
        QStringLiteral("3. Formato visual"),
        QStringLiteral("Ajuste a hierarquia visual para combinar com o tipo de jogo e o espaço disponível na tela."),
        formatLayout
    ));
    formLayout->addStretch();

    scrollArea->setWidget(formHost);
    contentRow->addWidget(scrollArea, 3);

    auto* previewCard = new QFrame(this);
    previewCard->setObjectName(QStringLiteral("overlayPreviewCard"));
    previewCard->setMinimumWidth(320);
    previewCard->setMaximumWidth(380);
    auto* previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setContentsMargins(18, 18, 18, 18);
    previewLayout->setSpacing(12);

    auto* previewTitle = new QLabel(QStringLiteral("Preview do overlay"), previewCard);
    previewTitle->setObjectName(QStringLiteral("overlaySectionTitle"));

    m_previewSummaryLabel = new QLabel(previewCard);
    m_previewSummaryLabel->setObjectName(QStringLiteral("overlaySectionHint"));
    m_previewSummaryLabel->setWordWrap(true);

    m_previewSurface = new QFrame(previewCard);
    m_previewSurface->setObjectName(QStringLiteral("overlayPreviewSurface"));
    auto* previewSurfaceLayout = new QVBoxLayout(m_previewSurface);
    previewSurfaceLayout->setContentsMargins(14, 14, 14, 14);
    previewSurfaceLayout->setSpacing(10);

    auto* badgesRow = new QWidget(m_previewSurface);
    auto* badgesLayout = new QHBoxLayout(badgesRow);
    badgesLayout->setContentsMargins(0, 0, 0, 0);
    badgesLayout->setSpacing(6);

    m_previewStatusBadge = new QLabel(badgesRow);
    m_previewStatusBadge->setObjectName(QStringLiteral("overlayPreviewBadge"));

    m_previewGameBadge = new QLabel(badgesRow);
    m_previewGameBadge->setObjectName(QStringLiteral("overlayPreviewBadgeMuted"));

    badgesLayout->addWidget(m_previewStatusBadge);
    badgesLayout->addWidget(m_previewGameBadge);
    badgesLayout->addStretch();

    previewSurfaceLayout->addWidget(badgesRow);

    m_previewMetricsLayout = new QGridLayout;
    m_previewMetricsLayout->setContentsMargins(0, 0, 0, 0);
    m_previewMetricsLayout->setHorizontalSpacing(8);
    m_previewMetricsLayout->setVerticalSpacing(8);
    previewSurfaceLayout->addLayout(m_previewMetricsLayout);

    previewLayout->addWidget(previewTitle);
    previewLayout->addWidget(m_previewSummaryLabel);
    previewLayout->addWidget(m_previewSurface, 1);
    contentRow->addWidget(previewCard, 0);

    auto* buttonsRow = new QHBoxLayout;
    buttonsRow->setSpacing(8);
    rootLayout->addLayout(buttonsRow);

    auto* restoreDefaultsButton = new QPushButton(QStringLiteral("Restaurar padrão"), this);
    restoreDefaultsButton->setObjectName(QStringLiteral("overlayGhostBtn"));

    auto* cancelButton = new QPushButton(QStringLiteral("Cancelar"), this);
    cancelButton->setObjectName(QStringLiteral("overlaySecondaryBtn"));

    m_saveButton = new QPushButton(QStringLiteral("Salvar configuração"), this);
    m_saveButton->setObjectName(QStringLiteral("overlayPrimaryBtn"));

    buttonsRow->addWidget(restoreDefaultsButton);
    buttonsRow->addStretch();
    buttonsRow->addWidget(cancelButton);
    buttonsRow->addWidget(m_saveButton);

    connect(restoreDefaultsButton, &QPushButton::clicked, this, [this]() {
        m_settings->restoreDefaults();
        loadFromViewModel();
        refreshPreview();
        updateValidationState();
    });
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_saveButton, &QPushButton::clicked, this, &OverlaySettingsDialog::saveAndAccept);
}

void OverlaySettingsDialog::wireSignals() {
    connect(m_enableOverlayCheck, &QCheckBox::toggled, this, [this](const bool checked) {
        m_settings->setOverlayEnabled(checked);
    });
    connect(m_autoEnableOverlayCheck, &QCheckBox::toggled, this, [this](const bool checked) {
        m_settings->setAutoEnableOnGameLaunch(checked);
    });
    connect(m_showLabelsCheck, &QCheckBox::toggled, this, [this](const bool checked) {
        m_settings->setShowLabels(checked);
    });
    connect(m_opacitySlider, &QSlider::valueChanged, this, [this](const int value) {
        m_settings->setBackgroundOpacity(value);
    });
    connect(m_positionCombo, &QComboBox::currentIndexChanged, this, [this](const int index) {
        m_settings->setOverlayPosition(static_cast<OverlayPosition>(m_positionCombo->itemData(index).toInt()));
    });

    for (auto it = m_metricChecks.cbegin(); it != m_metricChecks.cend(); ++it) {
        const QString metricId = it.key();
        connect(it.value(), &QCheckBox::toggled, this, [this, metricId](const bool checked) {
            m_settings->setMetricSelected(metricId, checked);
        });
    }

    for (auto it = m_metricDisplayCombos.cbegin(); it != m_metricDisplayCombos.cend(); ++it) {
        const QString metricId = it.key();
        connect(it.value(), &QComboBox::currentIndexChanged, this, [this, metricId](const int index) {
            const auto displayType = static_cast<ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType>(
                m_metricDisplayCombos.value(metricId)->itemData(index).toInt()
            );
            m_settings->setMetricDisplayType(metricId, displayType);
        });
    }

    connect(m_settings.get(), &ny::ui::viewmodels::OverlaySettingsViewModel::settingsChanged, this, [this]() {
        m_opacityValueLabel->setText(QStringLiteral("%1%").arg(m_settings->backgroundOpacity()));
        refreshPreview();
        updateValidationState();
    });
}

void OverlaySettingsDialog::loadFromViewModel() {
    const QSignalBlocker blockEnable(m_enableOverlayCheck);
    const QSignalBlocker blockAutoEnable(m_autoEnableOverlayCheck);
    const QSignalBlocker blockLabels(m_showLabelsCheck);
    const QSignalBlocker blockPosition(m_positionCombo);
    const QSignalBlocker blockOpacity(m_opacitySlider);

    m_enableOverlayCheck->setChecked(m_settings->overlayEnabled());
    m_autoEnableOverlayCheck->setChecked(m_settings->autoEnableOnGameLaunch());
    m_showLabelsCheck->setChecked(m_settings->showLabels());
    m_opacitySlider->setValue(m_settings->backgroundOpacity());
    m_opacityValueLabel->setText(QStringLiteral("%1%").arg(m_settings->backgroundOpacity()));

    for (int index = 0; index < m_positionCombo->count(); ++index) {
        if (m_positionCombo->itemData(index).toInt() == static_cast<int>(m_settings->overlayPosition())) {
            m_positionCombo->setCurrentIndex(index);
            break;
        }
    }

    for (auto it = m_metricChecks.cbegin(); it != m_metricChecks.cend(); ++it) {
        QSignalBlocker metricBlocker(it.value());
        it.value()->setChecked(m_settings->isMetricSelected(it.key()));
    }

    for (auto it = m_metricDisplayCombos.cbegin(); it != m_metricDisplayCombos.cend(); ++it) {
        QSignalBlocker comboBlocker(it.value());
        const auto displayType = m_settings->metricDisplayType(it.key());
        for (int index = 0; index < it.value()->count(); ++index) {
            if (it.value()->itemData(index).toInt() == static_cast<int>(displayType)) {
                it.value()->setCurrentIndex(index);
                break;
            }
        }
    }
}

void OverlaySettingsDialog::refreshPreview() {
    const auto metricDefinitions = ny::ui::viewmodels::OverlaySettingsViewModel::availableMetrics();
    clearLayout(m_previewMetricsLayout);

    const QString overlayStatus = m_settings->overlayEnabled()
        ? QStringLiteral("OVERLAY ATIVO")
        : QStringLiteral("OVERLAY DESLIGADO");
    const QString gameStatus = m_settings->autoEnableOnGameLaunch()
        ? QStringLiteral("AUTO AO ABRIR JOGO")
        : QStringLiteral("AUTO DESATIVADO");

    m_previewStatusBadge->setText(overlayStatus);
    m_previewGameBadge->setText(gameStatus);
    m_previewSummaryLabel->setText(
        QStringLiteral("%1 • %2 métricas selecionadas")
            .arg(
                ny::ui::viewmodels::OverlaySettingsViewModel::overlayPositionLabel(m_settings->overlayPosition()),
                QString::number(m_settings->selectedMetricIds().size())
            )
    );

    const int alpha = qRound((m_settings->backgroundOpacity() / 100.0) * 255.0);
    m_previewSurface->setStyleSheet(QString(
        "QFrame#overlayPreviewSurface {"
        "background-color: rgba(6, 13, 22, %1);"
        "border: 1px solid rgba(24, 44, 64, 220);"
        "border-radius: 12px;"
        "}"
    ).arg(alpha));

    int visibleMetricCount = 0;
    for (const auto& definition : metricDefinitions) {
        if (!m_settings->isMetricSelected(definition.id)) {
            continue;
        }

        const int row = visibleMetricCount / 2;
        const int column = visibleMetricCount % 2;
        m_previewMetricsLayout->addWidget(buildPreviewMetricCard(definition.id), row, column);
        ++visibleMetricCount;
    }

    if (visibleMetricCount == 0) {
        auto* emptyLabel = new QLabel(
            QStringLiteral("Selecione pelo menos uma métrica para visualizar o overlay."),
            m_previewSurface
        );
        emptyLabel->setObjectName(QStringLiteral("overlaySectionHint"));
        emptyLabel->setWordWrap(true);
        m_previewMetricsLayout->addWidget(emptyLabel, 0, 0);
    }
}

void OverlaySettingsDialog::updateValidationState() {
    const bool hasMetrics = !m_settings->selectedMetricIds().isEmpty();
    m_validationLabel->setText(
        hasMetrics
            ? QStringLiteral("Boa escolha: mantenha apenas o necessário para leitura rápida durante o jogo.")
            : QStringLiteral("Selecione ao menos uma métrica para que o overlay tenha conteúdo útil.")
    );
    m_validationLabel->setProperty("invalid", !hasMetrics);
    style()->unpolish(m_validationLabel);
    style()->polish(m_validationLabel);
    m_saveButton->setEnabled(hasMetrics);
}

void OverlaySettingsDialog::saveAndAccept() {
    if (m_settings->selectedMetricIds().isEmpty()) {
        updateValidationState();
        return;
    }

    m_settings->save();
    accept();
}

QWidget* OverlaySettingsDialog::buildSectionCard(
    const QString& title,
    const QString& hint,
    QLayout* contentLayout
) {
    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("overlaySectionCard"));

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->setSpacing(10);

    auto* titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("overlaySectionTitle"));

    auto* hintLabel = new QLabel(hint, card);
    hintLabel->setObjectName(QStringLiteral("overlaySectionHint"));
    hintLabel->setWordWrap(true);

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(hintLabel);
    cardLayout->addLayout(contentLayout);
    return card;
}

QWidget* OverlaySettingsDialog::buildPreviewMetricCard(const QString& metricId) const {
    const auto metricDefinitions = ny::ui::viewmodels::OverlaySettingsViewModel::availableMetrics();
    const auto metricIt = std::find_if(
        metricDefinitions.cbegin(),
        metricDefinitions.cend(),
        [&metricId](const ny::ui::viewmodels::OverlaySettingsViewModel::MetricDefinition& definition) {
            return definition.id == metricId;
        }
    );

    if (metricIt == metricDefinitions.cend()) {
        return new QWidget(m_previewSurface);
    }

    const auto& definition = *metricIt;
    auto* card = new QFrame(m_previewSurface);
    card->setObjectName(QStringLiteral("overlayPreviewMetric"));

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(10, 10, 10, 10);
    cardLayout->setSpacing(4);

    auto* titleLabel = new QLabel(definition.title, card);
    titleLabel->setObjectName(QStringLiteral("overlayPreviewMetricTitle"));
    titleLabel->setVisible(m_settings->showLabels());

    const auto displayType = m_settings->metricDisplayType(definition.id);
    auto* valueLabel = new QLabel(previewValueForMode(definition, displayType), card);
    valueLabel->setObjectName(QStringLiteral("overlayPreviewMetricValue"));
    valueLabel->setProperty("accentColor", metricAccentColor(definition.id));
    valueLabel->setStyleSheet(QStringLiteral("color: %1;").arg(metricAccentColor(definition.id)));

    QFont valueFont = valueLabel->font();
    switch (displayType) {
    case ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::NumberOnly:
        valueFont.setPointSize(16);
        valueFont.setBold(true);
        break;
    case ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::ChartOnly:
        valueFont.setPointSize(10);
        valueFont.setBold(false);
        break;
    case ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::NumberAndChart:
        valueFont.setPointSize(15);
        valueFont.setBold(true);
        break;
    }
    valueLabel->setFont(valueFont);

    cardLayout->addWidget(titleLabel);
    if (displayType != ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::ChartOnly) {
        cardLayout->addWidget(valueLabel);
    }

    if (displayType != ny::ui::viewmodels::OverlaySettingsViewModel::MetricDisplayType::NumberOnly) {
        auto* trendLabel = new QLabel(previewTrendForMetric(definition.id), card);
        trendLabel->setObjectName(QStringLiteral("overlayPreviewMetricTrend"));
        cardLayout->addWidget(trendLabel);
    }

    return card;
}

} // namespace ny::ui::qt



