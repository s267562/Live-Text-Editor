//
// Created by andrea settimo on 04/05/2020.
//

#include "TextEdit.h"
#include <QDebug>

void TextEdit::insertFromMimeData(const QMimeData * source)
{
    if (source->hasText())
    {
        qDebug() << "Dati inseriti!" << source->text();
        textCursor().insertText(source->text());
    }
}
