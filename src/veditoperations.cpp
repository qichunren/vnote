#include <QTextCursor>
#include <QTextDocument>
#include <QFontMetrics>
#include "vedit.h"
#include "veditoperations.h"
#include "vconfigmanager.h"
#include "utils/vutils.h"

extern VConfigManager vconfig;

VEditOperations::VEditOperations(VEdit *p_editor, VFile *p_file)
    : QObject(p_editor), m_editor(p_editor), m_file(p_file),
      m_editConfig(&p_editor->getConfig())
{
    m_vim = new VVim(m_editor);

    connect(m_editor, &VEdit::configUpdated,
            this, &VEditOperations::handleEditConfigUpdated);
    connect(m_vim, &VVim::modeChanged,
            this, &VEditOperations::handleVimModeChanged);
    connect(m_vim, &VVim::vimMessage,
            this, &VEditOperations::statusMessage);
    connect(m_vim, &VVim::vimStatusUpdated,
            this, &VEditOperations::vimStatusUpdated);
}

void VEditOperations::insertTextAtCurPos(const QString &p_text)
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.insertText(p_text);
    m_editor->setTextCursor(cursor);
}

VEditOperations::~VEditOperations()
{
}

void VEditOperations::updateCursorLineBg()
{
    if (m_editConfig->m_enableVimMode) {
        switch (m_vim->getMode()) {
        case VimMode::Normal:
            m_editConfig->m_cursorLineBg = QColor(vconfig.getEditorVimNormalBg());
            break;

        case VimMode::Insert:
            m_editConfig->m_cursorLineBg = QColor(vconfig.getEditorVimInsertBg());
            break;

        case VimMode::Visual:
        case VimMode::VisualLine:
            m_editConfig->m_cursorLineBg = QColor(vconfig.getEditorVimVisualBg());
            break;

        case VimMode::Replace:
            m_editConfig->m_cursorLineBg = QColor(vconfig.getEditorVimReplaceBg());
            break;

        default:
            V_ASSERT(false);
            break;
        }
    } else {
        m_editConfig->m_cursorLineBg = QColor(vconfig.getEditorCurrentLineBg());
    }

    m_editor->highlightCurrentLine();
}

void VEditOperations::handleEditConfigUpdated()
{
    // Reset to Normal mode.
    m_vim->setMode(VimMode::Normal);

    updateCursorLineBg();
}

void VEditOperations::handleVimModeChanged(VimMode p_mode)
{
    // Only highlight current visual line in Insert mode.
    m_editConfig->m_highlightWholeBlock = (p_mode != VimMode::Insert);

    updateCursorLineBg();
}

void VEditOperations::requestUpdateVimStatus()
{
    if (m_editConfig->m_enableVimMode) {
        emit vimStatusUpdated(m_vim);
    } else {
        emit vimStatusUpdated(NULL);
    }
}
