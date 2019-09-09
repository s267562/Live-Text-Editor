#ifndef CONSTANTS_H
#define CONSTANTS_H

#define TIMEOUT 15000
#define OK_MESSAGE "OKY\r\n"
#define ERR_MESSAGE "ERR\r\n"
#define LOGIN_MESSAGE "LIN\r\n"
#define LOGOUT_MESSAGE "LOG\r\n"
#define REGISTRATION_MESSAGE "REG\r\n"
#define REQUEST_FILE_MESSAGE "RFL\r\n"
#define LIST_OF_USERS "USR\r\n"
#define REMOVE_USER "RUS\n\r"
#define LIST_OF_FILE "LSF\r\n"
#define INSERT_MESSAGE "INS\r\n"
#define STYLE_CAHNGED_MESSAGE "STY\r\n"
#define DELETE_MESSAGE "DEL\r\n"
#define SENDING_FILE "FIL\n\r"

enum SocketState{
    UNLOGGED,
    LOGGED
};

#endif // CONSTANTS_H
