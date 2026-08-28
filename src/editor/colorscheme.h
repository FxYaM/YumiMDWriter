/*
 * SPDX-FileCopyrightText: 2020-2023 Megan Conkle <megan.conkle@kdemail.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef COLORSCHEME_H
#define COLORSCHEME_H

#include <QColor>

namespace ghostwriter
{
typedef struct ColorScheme {
    QColor foreground;
    QColor background;
    QColor selection;
    QColor cursor;
    QColor link;
    QColor image;
    QColor inlineHtml;
    QColor headingText;
    QColor headingMarkup;
    QColor emphasisText;
    QColor emphasisMarkup;
    QColor blockquoteText;
    QColor blockquoteMarkup;
    QColor divider;
    QColor listMarkup;
    QColor codeText;
    QColor codeMarkup;
    QColor error;
} ColorScheme;

inline QColor codeBackgroundColor(const ColorScheme &colors)
{
    constexpr int foregroundAlpha = 10;
    constexpr int colorChannelMaximum = 255;
    constexpr int backgroundAlpha = colorChannelMaximum - foregroundAlpha;

    auto blendChannel = [=](int foreground, int background) {
        return ((foreground * foregroundAlpha) + (background * backgroundAlpha) + (colorChannelMaximum / 2)) / colorChannelMaximum;
    };

    return QColor(blendChannel(colors.foreground.red(), colors.background.red()),
                  blendChannel(colors.foreground.green(), colors.background.green()),
                  blendChannel(colors.foreground.blue(), colors.background.blue()));
}
} // namespace ghostwriter

#endif // COLORSCHEME_H
