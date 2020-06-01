//
// Created by andrea settimo on 04/05/2020.
//

#include "TextEdit.h"
#include <QDebug>

void TextEdit::insertFromMimeData(const QMimeData * source)
{
    std::unique_lock<std::shared_mutex> isWorkingLock(controller->getCrdt()->mutexIsWorking);
    if (source->hasText())
    {
        controller->getCrdt()->copy = true;
        controller->editor->copyFlag = true;
        if (!controller->editor->pendingChar.empty()) {
            controller->editor->count = controller->editor->pendingChar.size();
        }
        position = textCursor().position();
        isWorkingLock.unlock();
        qDebug() << "Dati inseriti!" << source->text();
        textCursor().insertText(source->text());
    }
}
