/*
 * SPDX-FileCopyrightText: 2020-2023 Megan Conkle <megan.conkle@kdemail.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QStack>
#include <QTextStream>
#include <QtGlobal>

#include <3rdparty/cmark-gfm/src/cmark-gfm.h>
#include "markdownast.h"

namespace ghostwriter
{
class MarkdownASTPrivate
{
public:
    MarkdownASTPrivate()
    {
        ;
    }

    ~MarkdownASTPrivate()
    {
        ;
    }

    MemoryArena<MarkdownNode> arena;
    MarkdownNode *root;

    // Highlighting normally queries lines in order. Retain only the
    // top-level search position so each lookup preserves the full descent
    // rules below without rescanning preceding top-level siblings.
    mutable MarkdownNode *lastTopLevelBlock = nullptr;
    mutable int lastLookupLine = -1;
};

MarkdownAST::MarkdownAST()
    : d_ptr(new MarkdownASTPrivate())
{
    Q_D(MarkdownAST);
    
    d->root = nullptr;
}

MarkdownAST::MarkdownAST(cmark_node *root)
    : d_ptr(new MarkdownASTPrivate())
{    
    setRoot(root);
}

MarkdownAST::~MarkdownAST()
{
    Q_D(MarkdownAST);
    
    d->arena.freeAll();
    d->root = nullptr;
}

MarkdownNode *MarkdownAST::root()
{
    Q_D(MarkdownAST);
    
    return d->root;
}

void MarkdownAST::setRoot(cmark_node *root)
{
    Q_D(MarkdownAST);

    d->lastTopLevelBlock = nullptr;
    d->lastLookupLine = -1;

    d->arena.freeAll();

    if (nullptr == root) {
        d->root = nullptr;
        return;
    }

    d->root = d->arena.allocate();

    // Clone the node into memory that isn't allocated to
    // cmark-gfm's arena memory.
    QStack<cmark_node *> fromNodes;
    QStack<MarkdownNode *> toNodes;

    d->root->setDataFrom(root);
    fromNodes.push(root);
    toNodes.push(d->root);

    while (!fromNodes.isEmpty()) {
        cmark_node *source = fromNodes.pop();
        MarkdownNode *dest = toNodes.pop();

        // Prep children nodes for cloning.
        MarkdownNode *destParent = dest;
        source = cmark_node_first_child(source);

        while (NULL != source) {
            fromNodes.push(source);
            dest = d->arena.allocate();
            dest->setDataFrom(source);
            destParent->appendChild(dest);
            toNodes.push(dest);
            source = cmark_node_next(source);
        }
    }
}

MarkdownNode *MarkdownAST::findBlockAtLine(int lineNumber) const
{
    Q_D(const MarkdownAST);
    
    if ((nullptr == d->root) || (MarkdownNode::Invalid == d->root->type())) {
        return nullptr;
    }

    MarkdownNode *candidate = nullptr;
    MarkdownNode *current = d->root->firstChild();

    if ((d->lastLookupLine >= 0) && (lineNumber >= d->lastLookupLine)) {
        current = d->lastTopLevelBlock;
    }

    // Skip only top-level siblings that the original search would skip.
    // A backwards lookup restarts from the root, including queued requests
    // to rehighlight earlier setext heading or table header lines.
    while ((nullptr != current) && current->isBlockType() && (MarkdownNode::TableCell != current->type()) && (current->startLine() <= lineNumber)
           && (0 != current->endLine()) && (current->endLine() < lineNumber)) {
        current = current->next();
    }

    d->lastTopLevelBlock = current;
    d->lastLookupLine = lineNumber;

    while
    (
        (nullptr != current)
        && (current->isBlockType())
        && (MarkdownNode::TableCell != current->type())
    ) {
        if
        (
            (current->startLine() <= lineNumber)
            &&
            (
                (lineNumber <= current->endLine())
                || (0 == current->endLine())
            )
        ) {
            candidate = current;

            switch (current->type()) {
            case MarkdownNode::ListItem:
            case MarkdownNode::TaskListItem:
                return candidate;
            case MarkdownNode::Heading: {
                int lineCount = current->endLine() - current->startLine() + 1;

                if (
                    (lineCount > 2) &&
                    (lineNumber == current->endLine())) {
                    current = current->next();
                } else {
                    current = current->firstChild();
                }
                break;
            }
            default:
                current = current->firstChild();
                break;
            }
        } else if (current->startLine() > lineNumber) {
            return candidate;
        } else {
            current = current->next();
        }
    }

    return candidate;
}

QVector<MarkdownNode *> MarkdownAST::headings() const
{
    Q_D(const MarkdownAST);
    
    QVector<MarkdownNode *> headings;

    if ((nullptr == d->root) || (MarkdownNode::Invalid == d->root->type())) {
        return headings;
    }

    MarkdownNode *node = d->root->firstChild();

    while (nullptr != node) {
        if (MarkdownNode::Heading == node->type()) {
            headings.append(node);
        }

        node = node->next();
    }

    return headings;
}

void MarkdownAST::clear()
{
    Q_D(MarkdownAST);

    d->lastTopLevelBlock = nullptr;
    d->lastLookupLine = -1;

    d->arena.freeAll();
    d->root = nullptr;
}

QString MarkdownAST::toString() const
{
    Q_D(const MarkdownAST);
    
    if (nullptr == d->root) {
        return "AST is empty";
    }

    QString text;
    QTextStream stream(&text);
    QStack<MarkdownNode *> nodes;
    QStack<QString> indentation;

    nodes.push(d->root);
    indentation.push("");

    while (!nodes.empty()) {
        MarkdownNode *node = nodes.pop();
        QString indent = indentation.pop();


#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        stream << indent << "->" << node->toString() << Qt::endl;
#else
        stream << indent << "->" << node->toString() << endl;
#endif

        MarkdownNode *child = node->lastChild();
        indent += "   ";

        while (nullptr != child) {
            nodes.push(child);
            indentation.push(indent);
            child = child->previous();
        }
    }

    return text;
}
} // namespace ghostwriter
