#include "commandstream.h"

CommandStream::CommandStream() {}

bool CommandStream::setup() {
    return this->bt_serial.begin("Marco Polo");
}

Command CommandStream::next_command() {
    if(this->bt_serial.available() == 0) return Command::NONE;

    char command = this->bt_serial.read();

    switch(command) {
        case 'F':
            return Command::GO_FORWARD;
        case 'B':
            return Command::GO_BACKWARDS;
        case 'f':
        case 'b':
            return Command::STOP;
        case 'L':
            return Command::POINT_LEFT;
        case 'R':
            return Command::POINT_RIGHT;
        case 'l':
        case 'r':
            return Command::POINT_AHEAD;
        case 'X':
            return Command::ABORT;
        case 'S':
            return Command::STOP_RECORDING;
        case 'C':
            return Command::BEGIN_RECORDING;
        case 'T':
            return Command::PLAY_RECORDING;
        case 'P':
            return Command::TOGGLE_LIGHTS;
    }

    return Command::NONE;
}

bool CommandStream::has_commands() {
    return this->bt_serial.available() != 0;
}
