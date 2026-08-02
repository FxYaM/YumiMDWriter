/*
 * SPDX-FileCopyrightText: 2018-2023 Megan Conkle <megan.conkle@kdemail.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "previewproxy.h"

namespace ghostwriter
{
PreviewProxy::PreviewProxy(QObject *parent)
    : QObject(parent),
      m_htmlContent(""),
      m_styleSheet(""),
      m_mathEnabled(false),
      m_previewSyncEnabled(true),
      m_sourceLine(1),
      m_sourceScrollRatio(0.0),
      m_sourceViewportAnchor(0.0)
{
    ;
}

PreviewProxy::~PreviewProxy()
{
    ;
}

void PreviewProxy::setHtmlContent(const QString &html)
{
    m_htmlContent = html;
    emit htmlChanged(m_htmlContent);
}

QString PreviewProxy::htmlContent() const
{
    return m_htmlContent;
}

void PreviewProxy::setStyleSheet(const QString &css)
{
    m_styleSheet = css;
    emit styleSheetChanged(m_styleSheet);
}

QString PreviewProxy::styleSheet() const
{
    return m_styleSheet;
}

void PreviewProxy::setMathEnabled(bool enabled)
{
    if (enabled != m_mathEnabled) {
        m_mathEnabled = enabled;
        emit mathToggled(m_mathEnabled);
    }
}

bool PreviewProxy::mathEnabled() const
{
    return m_mathEnabled;
}

void PreviewProxy::setPreviewSyncEnabled(bool enabled)
{
    if (enabled != m_previewSyncEnabled) {
        m_previewSyncEnabled = enabled;
        emit previewSyncToggled(enabled);
    }
}

bool PreviewProxy::previewSyncEnabled() const
{
    return m_previewSyncEnabled;
}

void PreviewProxy::setSourcePosition(
    int lineNumber,
    qreal scrollRatio,
    qreal viewportAnchor)
{
    m_sourceLine = lineNumber;
    m_sourceScrollRatio = scrollRatio;
    m_sourceViewportAnchor = viewportAnchor;
    emit sourcePositionChanged(lineNumber, scrollRatio, viewportAnchor);
}

int PreviewProxy::sourceLine() const
{
    return m_sourceLine;
}

qreal PreviewProxy::sourceScrollRatio() const
{
    return m_sourceScrollRatio;
}

qreal PreviewProxy::sourceViewportAnchor() const
{
    return m_sourceViewportAnchor;
}
} // namespace ghostwriter
