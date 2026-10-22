/*
 * SPDX-FileCopyrightText: 2026 XiangXiang
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QFile>
#include <QMimeData>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTest>
#include <QTextLayout>

#include "../../src/editor/markdowneditor.h"
#include "../../src/editor/markdownstates.h"

using namespace ghostwriter;

class MarkdownEditorTest : public QObject
{
    Q_OBJECT

private slots:
    void inputMethodRectUsesEditorCoordinates();
    void inputMethodAnchorRectUsesEditorCoordinates();
    void displaysAndReservesLineNumberArea();
    void highlightsInlineCodeWithBackground();
    void dropsJpegAsMarkdownImage();
    void highlightsMarkdownAfterTable_data();
    void highlightsMarkdownAfterTable();
    void removingOpeningFenceReformatsAfterClosingFence();
    void referenceDefinitionRefreshesEarlierLink();
    void paragraphImmediatelyBeforeTable();
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

void MarkdownEditorTest::dropsJpegAsMarkdownImage()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString imagePath = directory.filePath("photo test.jpg");
    QFile imageFile(imagePath);
    QVERIFY(imageFile.open(QIODevice::WriteOnly));
    imageFile.write("jpeg test data");
    imageFile.close();

    MarkdownDocument document;
    MarkdownEditor editor(&document, testColors());
    editor.resize(480, 180);
    editor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&editor));

    QMimeData mimeData;
    mimeData.setUrls({QUrl::fromLocalFile(imagePath)});
    QDragEnterEvent dragEnterEvent(QPoint(20, 20), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(editor.viewport(), &dragEnterEvent);
    QVERIFY(dragEnterEvent.isAccepted());

    QDropEvent event(QPointF(20, 20), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(editor.viewport(), &event);

    QVERIFY(event.isAccepted());
    QVERIFY(editor.toPlainText().contains("photo test"));
    QVERIFY(editor.toPlainText().contains(".jpg"));
}

static ColorScheme highlightTestColors()
{
    ColorScheme colors;
    colors.foreground = Qt::black;
    colors.background = Qt::white;
    colors.selection = Qt::blue;
    colors.cursor = Qt::black;
    colors.headingText = QColor("#3456b8");
    colors.headingMarkup = QColor("#e57c00");
    colors.codeText = QColor("#aa3377");
    colors.codeMarkup = QColor("#117744");
    colors.emphasisMarkup = QColor("#993355");
    colors.emphasisText = QColor("#774499");
    colors.blockquoteText = QColor("#885522");
    colors.blockquoteMarkup = QColor("#996633");
    colors.link = QColor("#0066bb");
    return colors;
}

static QColor foregroundAt(const QTextBlock &block, int position)
{
    QColor foreground;

    for (const auto &range : block.layout()->formats()) {
        if (range.start <= position && position < range.start + range.length && range.format.hasProperty(QTextFormat::ForegroundBrush)) {
            foreground = range.format.foreground().color();
        }
    }

    return foreground;
}

static QTextBlock blockContaining(const MarkdownDocument &document, const QString &text)
{
    for (QTextBlock block = document.begin(); block.isValid(); block = block.next()) {
        if (block.text().contains(text)) {
            return block;
        }
    }

    return QTextBlock();
}

void MarkdownEditorTest::highlightsMarkdownAfterTable_data()
{
    QTest::addColumn<QString>("newline");

    QTest::newRow("LF") << QString("\n");
    QTest::newRow("CRLF") << QString("\r\n");
    QTest::newRow("CR") << QString("\r");
    QTest::newRow("paragraph separator") << QString(QChar(0x2029));
    QTest::newRow("line separator") << QString(QChar(0x2028));
}

void MarkdownEditorTest::highlightsMarkdownAfterTable()
{
    QFETCH(QString, newline);

    const ColorScheme colors = highlightTestColors();
    const QStringList lines = {"> 这是一段引用。",
                               "",
                               "## 前期规划",
                               "",
                               "| 目标 | 实现方式 |",
                               "| --- | --- |",
                               "| 自我管理 | 逐步实施 |",
                               "| 可回溯 | 保留记录 |",
                               "",
                               "## 领域建模",
                               "",
                               "```mermaid",
                               "flowchart LR",
                               "A[记录事实] --> B[按周期回看]",
                               "```",
                               "",
                               "## 通知架构",
                               ""};

    MarkdownDocument document;
    MarkdownEditor editor(&document, colors);
    editor.setEnableLargeHeadingSizes(false);
    editor.setPlainText(lines.join(newline));
    QCoreApplication::processEvents();

    const QTextBlock table = blockContaining(document, "| 自我管理 |");
    const QTextBlock heading = blockContaining(document, "## 领域建模");
    const QTextBlock code = blockContaining(document, "flowchart LR");
    const QTextBlock lastHeading = blockContaining(document, "## 通知架构");
    QVERIFY(table.isValid());
    QVERIFY(heading.isValid());
    QVERIFY(code.isValid());
    QVERIFY(lastHeading.isValid());

    QCOMPARE(table.userState() & MarkdownStateMask, MarkdownStatePipeTableRow);
    QCOMPARE(heading.userState() & MarkdownStateMask, MarkdownStateAtxHeading2);
    QCOMPARE(foregroundAt(heading, 3), colors.headingText);
    QVERIFY(code.userState() & MarkdownStateCodeBlock);
    QCOMPARE(foregroundAt(code, 0), colors.codeText);
    QCOMPARE(lastHeading.userState() & MarkdownStateMask, MarkdownStateAtxHeading2);
    QCOMPARE(foregroundAt(lastHeading, 3), colors.headingText);
}

void MarkdownEditorTest::removingOpeningFenceReformatsAfterClosingFence()
{
    const ColorScheme colors = highlightTestColors();
    MarkdownDocument document;
    MarkdownEditor editor(&document, colors);
    editor.setEnableLargeHeadingSizes(false);
    editor.setPlainText("```\ntext\n```\n\n## Later heading\n\nbody");
    QCoreApplication::processEvents();
    const QTextBlock heading = blockContaining(document, "## Later heading");
    QCOMPARE(foregroundAt(heading, 3), colors.headingText);
    QTextCursor cursor(&document);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    QTRY_COMPARE(foregroundAt(heading, 3), colors.codeText);
    document.undo();
    QTRY_COMPARE(foregroundAt(heading, 3), colors.headingText);
    document.redo();
    QTRY_COMPARE(foregroundAt(heading, 3), colors.codeText);
}

void MarkdownEditorTest::referenceDefinitionRefreshesEarlierLink()
{
    const ColorScheme colors = highlightTestColors();
    MarkdownDocument document;
    MarkdownEditor editor(&document, colors);
    editor.setEnableLargeHeadingSizes(false);
    editor.setPlainText("[link][ref]\n\n## Heading\n");
    QCoreApplication::processEvents();
    const QTextBlock link = document.begin();
    QVERIFY(foregroundAt(link, 1) != colors.link);
    QTextCursor cursor(&document);
    cursor.movePosition(QTextCursor::End);
    cursor.insertText("\n[ref]: https://example.com\n");
    QTRY_COMPARE(foregroundAt(link, 1), colors.link);
    document.undo();
    QTRY_VERIFY(foregroundAt(link, 1) != colors.link);
}

void MarkdownEditorTest::paragraphImmediatelyBeforeTable()
{
    const ColorScheme colors = highlightTestColors();
    MarkdownDocument document;
    MarkdownEditor editor(&document, colors);
    editor.setEnableLargeHeadingSizes(false);
    const QString text =
        "> quote\n\n## First\n\nIntroductory paragraph\n"
        "| Target | Method |\n| --- | --- |\n| Record | Review |\n\n"
        "## Later heading\n\n```mermaid\nflowchart LR\nA --> B\n```\n\n## Last heading\n";
    for (int reload = 0; reload < 2; ++reload) {
        if (reload) {
            editor.setPlainText("");
        }
        editor.setPlainText(text);
        QCoreApplication::processEvents();
        const QTextBlock table = blockContaining(document, "| Record |");
        const QTextBlock heading = blockContaining(document, "## Later heading");
        const QTextBlock code = blockContaining(document, "flowchart LR");
        const QTextBlock lastHeading = blockContaining(document, "## Last heading");
        QCOMPARE(table.userState() & MarkdownStateMask, MarkdownStatePipeTableRow);
        QCOMPARE(heading.userState() & MarkdownStateMask, MarkdownStateAtxHeading2);
        QCOMPARE(foregroundAt(heading, 3), colors.headingText);
        QCOMPARE(foregroundAt(code, 0), colors.codeText);
        QCOMPARE(foregroundAt(lastHeading, 3), colors.headingText);
    }
}

QTEST_MAIN(MarkdownEditorTest)
#include "markdowneditortest.moc"
