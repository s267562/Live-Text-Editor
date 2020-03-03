//
// Created by simone on 06/08/19.
//

#ifndef TEXTEDITOR_POS_H
#define TEXTEDITOR_POS_H

#include <string>
#include <ostream>
#include <QJsonObject>

class Pos {
public:
    Pos(int ch = 0, int line = 0);

    int getCh() const;
    void setCh(int ch);
    int getLine() const;
    void setLine(int line);

    void incrementCh();
    void resetCh();
    void incrementLine();
    operator bool() const {
        if(ch < 0 || line < 0 ) return false;
        else return true;
    }

    friend std::ostream &operator<<(std::ostream &os, const Pos &pos);
    void write(QJsonObject &json) const;
    void read(QJsonObject json);

private:
    int ch; // position in the line
    int line;
};


#endif //TEXTEDITOR_POS_H
