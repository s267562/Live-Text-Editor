//
// Created by simone on 06/08/19.
//

#ifndef TEXTEDITOR_POS_H
#define TEXTEDITOR_POS_H

#include <string>

class Pos {
public:
    Pos(int ch, int line);
    int getCh() const;
    int getLine() const;
    void incrementCh();
    void resetCh();
    void incrementLine();


private:
    int ch; // position in the line
    int line;
};


#endif //TEXTEDITOR_POS_H
