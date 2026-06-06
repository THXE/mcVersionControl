#include "ProjectPanel.h"
#include "ProjectCard.h"
#include "Utils/ThemeManager.h"

#include <QMenu>
#include <QAction>

ProjectPanel::ProjectPanel(QWidget *parent)
    : QWidget(parent)
{
    setMinimumWidth(250);
    setObjectName("projectPanel");
    setupUi();
}

ProjectPanel::~ProjectPanel() = default;

void ProjectPanel::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === Header ===
    m_headerLabel = new QLabel(QStringLiteral("项目"), this);
    m_headerLabel->setObjectName("projectHeaderLabel");
    m_headerLabel->setFixedHeight(42);
    m_headerLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_headerLabel->setContentsMargins(15, 0, 0, 0);
    mainLayout->addWidget(m_headerLabel);

    // === Scroll Area ===
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    mainLayout->addWidget(m_scrollArea, 1);

    // Scroll content widget
    m_scrollContent = new QWidget();
    m_scrollContent->setObjectName("scrollContent");
    m_cardLayout = new QVBoxLayout(m_scrollContent);
    m_cardLayout->setContentsMargins(8, 8, 8, 8);
    m_cardLayout->setSpacing(4);
    m_cardLayout->addStretch();

    m_scrollArea->setWidget(m_scrollContent);

    // Border-right styled via QSS
}

// ============================================================
//  Public API
// ============================================================

void ProjectPanel::addProject(const ProjectInfo &info)
{
    auto *card = new ProjectCard(info, m_scrollContent);
    card->setProjectIndex(m_cards.size());

    // Connect card signals to local handlers
    connect(card, &ProjectCard::clicked,
            this, &ProjectPanel::onCardClicked);
    connect(card, &ProjectCard::doubleClicked,
            this, &ProjectPanel::onCardDoubleClicked);
    connect(card, &ProjectCard::contextMenuRequested,
            this, &ProjectPanel::onCardContextMenu);

    // Insert before the stretch
    m_cardLayout->insertWidget(m_cardLayout->count() - 1, card);
    m_cards.append(card);
}

void ProjectPanel::removeProject(int index)
{
    if (index < 0 || index >= m_cards.size()) return;

    ProjectCard *card = m_cards.takeAt(index);
    m_cardLayout->removeWidget(card);
    delete card;

    // Re-index remaining cards
    for (int i = 0; i < m_cards.size(); ++i) {
        m_cards[i]->setProjectIndex(i);
    }

    if (m_activeIndex == index) {
        m_activeIndex = -1;
        if (!m_cards.isEmpty()) {
            setActiveProject(0);
        }
    } else if (m_activeIndex > index) {
        m_activeIndex--;
    }
}

void ProjectPanel::clearProjects()
{
    for (ProjectCard *card : m_cards) {
        m_cardLayout->removeWidget(card);
        delete card;
    }
    m_cards.clear();
    m_activeIndex = -1;
}

void ProjectPanel::setActiveProject(int index)
{
    if (index < 0 || index >= m_cards.size()) return;

    // Deactivate previous
    if (m_activeIndex >= 0 && m_activeIndex < m_cards.size()) {
        m_cards[m_activeIndex]->setActive(false);
    }

    m_activeIndex = index;
    m_cards[index]->setActive(true);

    emit projectSelected(index);
}

// ============================================================
//  Card signal handlers
// ============================================================

ProjectCard *ProjectPanel::senderCard() const
{
    return qobject_cast<ProjectCard *>(sender());
}

void ProjectPanel::onCardClicked()
{
    ProjectCard *card = senderCard();
    if (card && card->projectIndex() >= 0) {
        setActiveProject(card->projectIndex());
    }
}

void ProjectPanel::onCardDoubleClicked()
{
    ProjectCard *card = senderCard();
    if (card) {
        emit projectDoubleClicked(card->projectInfo().path);
    }
}

void ProjectPanel::onCardContextMenu(const QPoint &globalPos)
{
    ProjectCard *card = senderCard();
    if (!card) return;

    int idx = card->projectIndex();
    if (idx < 0) return;

    QMenu menu(this);
    QAction *backupAction  = menu.addAction(QStringLiteral("备份"));
    QAction *deleteAction  = menu.addAction(QStringLiteral("删除"));
    menu.addSeparator();
    QAction *exportAction  = menu.addAction(QStringLiteral("导出"));

    QAction *chosen = menu.exec(globalPos);
    if (!chosen) return;

    if (chosen == backupAction) {
        emit backupRequested(idx);
    } else if (chosen == deleteAction) {
        emit deleteRequested(idx);
    } else if (chosen == exportAction) {
        // Stub
    }

    emit projectContextMenu(idx, globalPos);
}
