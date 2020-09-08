//
// Created by andrea settimo on 04/05/2020.
//

#include "TextEdit.h"
#include <qDebug>

void TextEdit::insertFromMimeData(const QMimeData * source)
{
    if (source->hasText())
    {
        controller->getCrdt()->copy = true;
        controller->editor->copyFlag = true;
        if (!controller->editor->pendingChar.empty()) {
            controller->editor->count = controller->editor->pendingChar.size();
        }
        position = textCursor().position();
        //qDebug() << "Dati inseriti!" << source->text();
        textCursor().insertText(source->text());
    }
}
