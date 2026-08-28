/*
 * SPDX-FileCopyrightText: 2026 XiangXiang
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QScrollBar>
#include <QTest>
#include <QTextLayout>

#include "../../src/editor/markdowneditor.h"

using namespace ghostwriter;

class MarkdownEditorTest : public QObject
{
    Q_OBJECT

private slots:
    void inputMethodRectUsesEditorCoordinates();
    void inputMethodAnchorRectUsesEditorCoordinates();
    void displaysAndReservesLineNumberArea();
    void highlightsInlineCodeWithBackground();
};

static ColorScheme testColors()
{
    ColorScheme colors;
    colors.foreground = Qt::black;
    colors.background = Qt::white;
    colors.selection = Qt::blue;
    colors.cursor = Qt::black;
    colors.codeText = Qt::black;
    colors.codeMarkup = Qt::darkGray;
    return colors;
}

static QTextCharFormat formatAt(const QTextBlock &block, int position)
{
    const QList<QTextLayout::FormatRange> formats = block.layout()->formats();

    for (const QTextLayout::FormatRange &range : formats) {
        if ((position >= range.start) && (position < (range.start + range.length))) {
            return range.format;
        }
    }

    return QTextCharFormat();
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

void MarkdownEditorTest::displaysAndReservesLineNumberArea()
{
    MarkdownDocument document;
    MarkdownEditor editor(&document, testColors());
    editor.setEditorWidth(EditorWidthFull);
    editor.setupPaperMargins();
    editor.resize(480, 180);
    editor.setPlainText(QString("line\n").repeated(8) + "line");
    editor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&editor));

    QWidget *lineNumberArea = editor.findChild<QWidget *>(QStringLiteral("lineNumberArea"));
    QVERIFY(lineNumberArea != nullptr);
    QVERIFY(lineNumberArea->isVisible());

    const int oneDigitWidth = lineNumberArea->width();
    QVERIFY(oneDigitWidth > 0);
    QVERIFY(editor.viewport()->geometry().left() > lineNumberArea->geometry().right());

    editor.setPlainText(QString("line\n").repeated(99) + "line");
    QCoreApplication::processEvents();

    QVERIFY(lineNumberArea->width() > oneDigitWidth);
    QVERIFY(editor.viewport()->geometry().left() > lineNumberArea->geometry().right());

    editor.verticalScrollBar()->setValue(editor.verticalScrollBar()->maximum());
    QCoreApplication::processEvents();
    QVERIFY(editor.topVisibleSourceLine() > 1);
}

void MarkdownEditorTest::highlightsInlineCodeWithBackground()
{
    const ColorScheme colors = testColors();
    MarkdownDocument document;
    MarkdownEditor editor(&document, colors);
    editor.setPlainText("before `inline` after");
    editor.highlighter()->rehighlight();
    QCoreApplication::processEvents();

    const QTextBlock block = document.firstBlock();
    const int openingBacktick = block.text().indexOf(QLatin1Char('`'));
    const int closingBacktick = block.text().lastIndexOf(QLatin1Char('`'));
    QVERIFY(openingBacktick >= 0);
    QVERIFY(closingBacktick > openingBacktick);

    const QColor expectedBackground(245, 245, 245);
    QVERIFY(expectedBackground != colors.background);
    QCOMPARE(formatAt(block, openingBacktick).background().color(), expectedBackground);
    QCOMPARE(formatAt(block, openingBacktick + 1).background().color(), expectedBackground);
    QCOMPARE(formatAt(block, closingBacktick).background().color(), expectedBackground);
}

QTEST_MAIN(MarkdownEditorTest)
#include "markdowneditortest.moc"
