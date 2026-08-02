/*
 * SPDX-FileCopyrightText: 2026 XiangXiang
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QRegularExpression>
#include <QTest>

#include "../../src/export/cmarkgfmexporter.h"

using namespace ghostwriter;

class CmarkGfmApiTest : public QObject
{
    Q_OBJECT

private slots:
    void previewIncludesSourcePositions();
    void fileExportHtmlStaysClean();
};

void CmarkGfmApiTest::previewIncludesSourcePositions()
{
    CmarkGfmExporter exporter;
    QString html;

    exporter.exportToHtmlForPreview(
        "# Title\n\nFirst line\nsecond line.\n\n- Item\n",
        html);

    QVERIFY(html.contains(
        QRegularExpression("<h1 data-sourcepos=\\\"1:1-1:\\d+\\\">")));
    QVERIFY(html.contains(
        QRegularExpression("<p data-sourcepos=\\\"3:1-4:\\d+\\\">")));
    QVERIFY(html.contains(
        QRegularExpression("<ul data-sourcepos=\\\"6:1-6:\\d+\\\">")));
}

void CmarkGfmApiTest::fileExportHtmlStaysClean()
{
    CmarkGfmExporter exporter;
    QString html;

    exporter.exportToHtml("# Title", html);

    QVERIFY(!html.contains("data-sourcepos"));
}

QTEST_GUILESS_MAIN(CmarkGfmApiTest)
#include "cmarkgfmapitest.moc"
