//
// Created by simone on 06/08/19.
//

#include "Pos.h"

Pos::Pos(int ch, int line) : ch(ch), line(line) {}

int Pos::getCh() const {
    return ch;
}

int Pos::getLine() const {
    return line;
}

void Pos::incrementCh() {
    this->ch++;
}

void Pos::resetCh() {
    this->ch = 0;
}

void Pos::incrementLine() {
    this->line++;
}