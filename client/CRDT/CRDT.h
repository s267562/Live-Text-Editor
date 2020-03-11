//
// Created by simone on 05/08/19.
//

#ifndef TEXTEDITOR_CRDT_H
#define TEXTEDITOR_CRDT_H

#include <string>
#include <vector>
#include "../utils/Pos.h"
#include "../utils/Character.h"
#include "../utils/Identifier.h"
#include "../Networking/Messanger.h"
#include "../Editor/editor.h"
#include <math.h>
#include <iostream>
#include <map>
#include <QThread>
#include <QTextCursor>
#include <shared_mutex>

class Messanger;
class Controller;
class Editor;

class CRDT: public QObject {
    Q_OBJECT;
public:
    std::shared_mutex mutexCRDT;
    CRDT(QObject *parent, Messanger *messanger);

    void setStructure(const std::vector<std::vector<Character>> &initialStructure);
    void setSiteId(const QString &siteId);
    const QString &getSiteId() const;
    const Character getCharacter(Pos pos);
    Character handleLocalInsert(char val, QTextCharFormat textCharFormat, Pos pos);
    Pos handleRemoteInsert(Character character);
    std::vector<Character> handleLocalDelete(Pos startPos, Pos endPos);
    Pos handleRemoteDelete(const Character &character);
    const QString toText();
    bool styleChanged(QTextCharFormat textCharFormat, Pos pos);
    Pos handleRemoteStyleChanged(const Character &character);
    int getRow(Character blockId);
    Character getBlockIdentifier(int blockNumber);
    void printStructures();


    //TODO: Just for testing purpose
    std::vector<std::vector<Character>> getStructure();
    std::vector<std::pair<Character,int>> getStyle();

    QString toString();

    void setStyle(std::vector<std::pair<Character, int>> blocks);


private:
    QString siteId;
    std::vector<std::vector<Character>> structure;
    std::map<QString, int> versionsVector; // map<username, counter>
    static const int base = 32;
    std::vector<std::pair<Character,int>> style;
    Messanger *messanger;
    Editor* editor = nullptr;

public:
    void setEditor(Editor *editor);

private:

    // insert
    const Character generateChar(char val, QTextCharFormat textCharFormat, Pos pos, QString siteId);
    const std::vector<Identifier> findPosBefore(Pos pos);
    const std::vector<Identifier> findPosAfter(Pos pos);
    std::vector<Identifier> generatePosBetween(std::vector<Identifier> pos1, std::vector<Identifier> pos2, QString siteId, std::vector<Identifier> newPos = {}, int level = 0);
    int generateIdBetween(int min, int max);
    Pos findInsertPosition(Character character);
    Pos findEndPosition(Character lastChar, std::vector<Character> lastLine, int totalLines);
    void insertChar(Character character, Pos pos);


    // delete
    Pos findPosition(Character character);
    int findIndexInLine(Character character, std::vector<Character> line);

    std::vector<Character> deleteMultipleLines(Pos startPos, Pos endPos);
    std::vector<Character> deleteSingleLine(Pos startPos, Pos endPos);
    void removeEmptyLines();

    void mergeLines(int line);
    
    void insertBlock(Character character, Pos position);

    void removeStyleLine(int i);

public slots:
    //void localInsert(QString val, QTextCharFormat textCharFormat, Pos pos);
    void totalLocalInsert(int charsAdded, QTextCursor cursor, QString chars,  int position);
    void totalLocalStyleChange(int charsAdded, QTextCursor cursor,  int position);
    void localDelete(Pos startPos, Pos endPos);
    void newMessage(Message message);

signals:
    void writeInsert(Character);
    void writeDelete(Character);
    void writeStyleChanged(Character);

    void insertChar(char character, QTextCharFormat charFormat, Pos pos, QString);
    void changeStyle(Pos pos, const QTextCharFormat&format, QString);
    void deleteChar(Pos pos, QString);
};


#endif //TEXTEDITOR_CRDT_H
