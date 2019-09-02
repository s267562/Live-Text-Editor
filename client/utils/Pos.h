//
// Created by simone on 06/08/19.
//

#ifndef TEXTEDITOR_POS_H
#define TEXTEDITOR_POS_H

#include <string>
#include <ostream>

class Pos {
public:
    Pos(int ch, int line);

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

private:
    int ch; // position in the line
    int line;
};


#endif //TEXTEDITOR_POS_H
