/*
 * SPDX-FileCopyrightText: 2026 XiangXiang
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QScrollBar>
#include <QTest>

#include "../../src/editor/markdowneditor.h"

using namespace ghostwriter;

class MarkdownEditorTest : public QObject
{
    Q_OBJECT

private slots:
    void inputMethodRectUsesEditorCoordinates();
    void inputMethodAnchorRectUsesEditorCoordinates();
};

static ColorScheme testColors()
{
    ColorScheme colors;
    colors.foreground = Qt::black;
    colors.background = Qt::white;
    colors.selection = Qt::blue;
    colors.cursor = Qt::black;
    return colors;
}

static QRect editorCoordinates(
    MarkdownEditor &editor,
    const QTextCursor &cursor)
{
    QRect rect = editor.cursorRect(cursor);
    rect.moveTopLeft(editor.viewport()->mapTo(&editor, rect.topLeft()));
    return rect;
}

void MarkdownEditorTest::inputMethodRectUsesEditorCoordinates()
{
    MarkdownDocument document;
    MarkdownEditor editor(&document, testColors());
    editor.resize(480, 180);
    editor.setPlainText(QString("line\n").repeated(120));
    editor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&editor));

    QTextCursor cursor(document.findBlockByNumber(60));
    editor.setTextCursor(cursor);
    editor.verticalScrollBar()->setValue(
        editor.verticalScrollBar()->maximum() / 2);
    QCoreApplication::processEvents();

    const QRect expected = editorCoordinates(editor, editor.textCursor());
    const QRect actual =
        editor.inputMethodQuery(Qt::ImCursorRectangle).toRect();
    QCOMPARE(actual, expected);

    editor.verticalScrollBar()->setValue(
        editor.verticalScrollBar()->maximum());
    QCoreApplication::processEvents();
    QCOMPARE(
        editor.inputMethodQuery(Qt::ImCursorRectangle).toRect(),
        editorCoordinates(editor, editor.textCursor()));
}

void MarkdownEditorTest::inputMethodAnchorRectUsesEditorCoordinates()
{
    MarkdownDocument document;
    MarkdownEditor editor(&document, testColors());
    editor.resize(480, 180);
    editor.setPlainText("first line\nsecond line\nthird line");
    editor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&editor));

    QTextCursor selection(&document);
    selection.setPosition(2);
    selection.setPosition(24, QTextCursor::KeepAnchor);
    editor.setTextCursor(selection);
    QCoreApplication::processEvents();

    QTextCursor anchorCursor(editor.textCursor());
    anchorCursor.setPosition(anchorCursor.anchor());
    QCOMPARE(
        editor.inputMethodQuery(Qt::ImAnchorRectangle).toRect(),
        editorCoordinates(editor, anchorCursor));
}

QTEST_MAIN(MarkdownEditorTest)
#include "markdowneditortest.moc"
