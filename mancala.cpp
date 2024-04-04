#include <iostream>
#include <string>
#include <ctime>
#include <cstring>
#include <unistd.h> // For fork
#include <sys/socket.h>
#include <netinet/in.h>
#include <wait.h>
#include <vector>
#include <chrono>
#include <fstream>
#include <algorithm>

using namespace std;

#define CLEAR "cls"

class board
{
    int strPlr1;
    int strPlr2;
    bool   blnPlr1;
    int    intPots[14];

public:
    board(int, int);
    int  getAdjacent(int);
    bool sendMessage(int player, const string &message);
    void getMove();
    void getNextMove();
    void getNextTurn();
    int  getNumber();
    void getResults();
    void getStatus(int);
    void getSteal(int);
    int  getTranslation(int);
    int  getTranslation(int, bool);
    int  getWrap(int);
    bool isOwnSide(int);
    bool isPlaying();
    void pause();
    void setMove(int);
    void setResults();
};

board::board(int prmPlr1, int prmPlr2)
:strPlr1(prmPlr1), strPlr2(prmPlr2), blnPlr1(true) {
    for(int a = 0; a < 14; a++)
        intPots[a] = 4;
    intPots[getTranslation(-1, true)]  = 0;
    intPots[getTranslation(-1, false)] = 0;
}

int board::getAdjacent(int prmNmbr) {
    switch(prmNmbr) {
    case  0: return 12;
    case  1: return 11;
    case  2: return 10;
    case  3: return  9;
    case  4: return  8;
    case  5: return  7;
    case  6: return 13;
    case  7: return  5;
    case  8: return  4;
    case  9: return  3;
    case 10: return  2;
    case 11: return  1;
    case 12: return  0;
    case 13: return  6;
    default: return -1;
    }
}

bool board::sendMessage(int player, const string& message) { // function for sending messages for users
    int len = message.length();
    const char* buffer = message.c_str();

    int sent = send(player, buffer, len, 0);
    if (sent == -1) {
        cerr << "Message sending error: " << strerror(errno) << endl;
        return false;
    }

    return true;
}

void board::getMove() {
    int intMove;
    do {
        intMove = getNumber();
    } while(intPots[getTranslation(intMove)] == 0);

    setMove(intMove);
    return;
}

void board::getNextMove() {
    getMove();
    getNextTurn();
    return;
}

void board::getNextTurn() {
    if(blnPlr1)
        blnPlr1 = false;
    else
        blnPlr1 = true;
    return;
}

int board::getNumber() {
    int intInpt;
    string strInpt;
    int a;

    string mes1 = "p1,";
    string mes2 = "p2,";

    mes1 += "top,";
    mes2 += "top,";

    for(a = 1; a < 7; a++){
        string score1 = to_string(intPots[getTranslation(a, true)]);
        mes1 += score1 + ",";
        mes2 += score1 + ",";
    }

    // Scoring Pots
    mes1 += "score,";
    mes2 += "score,";

    string score11 = to_string(intPots[getTranslation(-1, true)]);
    mes1 += score11 + ",";
    mes2 += score11 + ",";

    string score12 = to_string(intPots[getTranslation(a, false)]);
    mes1 += score12 + ",";
    mes2 += score12 + ",";

    // Player 2
    mes1 += "bottom,";
    mes2 += "bottom,";

    for(a = 6; a > 0; a--){
        string score1 = to_string(intPots[getTranslation(a, false)]);
        mes1 += score1 + ",";
        mes2 += score1 + ",";
    }

    do {
        sleep(1);
        int numBytes;
        char buffer[1024];
        if(blnPlr1){
            mes1 += "player1";
            mes2 += "player1";
            sendMessage(strPlr1,mes1);
            sendMessage(strPlr2,mes2);
            memset(buffer, 0, sizeof(buffer));
            numBytes = recv(strPlr1, buffer , sizeof(buffer-1), 0);}
        else{
            mes1 += "player2";
            mes2 += "player2";
            sendMessage(strPlr1,mes1);
            sendMessage(strPlr2,mes2);
            memset(buffer, 0, sizeof(buffer));
            numBytes = recv(strPlr2, buffer , sizeof(buffer-1), 0);}
        if (numBytes <= 0){
            cerr << "Error podczas odbierania odpowiedzi" << endl;
            strInpt = "";
        }   
        buffer[numBytes] = '\0';
        strInpt = string(buffer);

        intInpt = atoi(strInpt.c_str());
    } while(intInpt < 1 || intInpt > 6);
    return intInpt;
}

void board::getResults() {
    setResults();
    if(intPots[getTranslation(-1, true)] > intPots[getTranslation(-1, false)]){
        sendMessage(strPlr1,"You win!");
        sendMessage(strPlr2,"You lose!");
    }
    else if(intPots[getTranslation(-1, false)] > intPots[getTranslation(-1, true)]){
        sendMessage(strPlr2,"You win!");
        sendMessage(strPlr1,"You lose!");
    }
    else{
        sendMessage(strPlr1,"It's a tie!");
        sendMessage(strPlr2,"It's a tie!");
    }
    return;
}

void board::getStatus(int prmLast) {
    pause();
    if(prmLast == getTranslation(-1)) {
        getNextTurn();
        return;
    }

    if(intPots[prmLast] == 1 && isOwnSide(prmLast)) {
        getSteal(prmLast);
        return;
    }

    return;
}

void board::getSteal(int prmLast) {
    intPots[prmLast] = 0;
    intPots[getTranslation(-1)] += intPots[getAdjacent(prmLast)] + 1;
    intPots[getAdjacent(prmLast)] = 0;
    return;
}

int board::getTranslation(int prmNmbr) {
    return getTranslation(prmNmbr, blnPlr1);
}

int board::getTranslation(int prmNmbr, bool prmPlr1) {
    if(prmPlr1) {
        switch(prmNmbr) {
        case 1:  return  5;
        case 2:  return  4;
        case 3:  return  3;
        case 4:  return  2;
        case 5:  return  1;
        case 6:  return  0;
        default: return  6;
        }
    }

    else {
        switch(prmNmbr) {
        case 1:  return 12;
        case 2:  return 11;
        case 3:  return 10;
        case 4:  return  9;
        case 5:  return  8;
        case 6:  return  7;
        default: return 13;
        }
    }
}

int board::getWrap(int prmNmbr) {
    int intNmbr = prmNmbr;
    while(intNmbr >= 14)
        intNmbr -= 14;
    return intNmbr;
}

bool board::isOwnSide(int prmNmbr) {
    if(blnPlr1 && prmNmbr >= getTranslation(1, true) && prmNmbr <= getTranslation(6, true))
        return true;

    if(!blnPlr1 && prmNmbr >= getTranslation(1, false) && prmNmbr <= getTranslation(6, false))
        return true;

    return false;
}

bool board::isPlaying() {
    bool blnMov1 = false;
    bool blnMov2 = false;

    for(int a = 1; a < 7; a++) {
        if(intPots[getTranslation(a, true)]  > 0)
            blnMov1 = true;

        if(intPots[getTranslation(a, false)] > 0)
            blnMov2 = true;
    }

    if(blnMov1 && blnMov2)
        return true;

    return false;
}

void board::pause() {
    int intTime = time(NULL);
    while(time(NULL) - intTime < 1);
    return;
}

void board::setMove(int prmMove) {
    int intTran = getTranslation(prmMove);
    int intMovs = intPots[intTran];
    int intLast = intTran;
    intPots[intTran] = 0;

    for(int a = 1; a <= intMovs; a++) {
        int intPosn = getWrap(intTran + a);
        intLast = intPosn;

        if(blnPlr1 && intPosn == getTranslation(-1, false)) {
            intMovs++;
        }

        else if(!blnPlr1 && intPosn == getTranslation(-1, true)) {
            intMovs++;
        }

        else {
            //pause();
            intPots[intPosn]++;
        }
    }

    getStatus(intLast);
}

void board::setResults() {
    int p1pots = 0;
    int p2pots = 0;
    for(int a = 1; a <= 6; a++) {
        p1pots += intPots[getTranslation(a, true)];
        p2pots += intPots[getTranslation(a, false)];
    }

    for(int a = 1; a <= 6; a++) {
        if(p1pots == 0){
            intPots[getTranslation(-1, true)]  += intPots[getTranslation(a, true)];
            intPots[getTranslation(-1, true)] += intPots[getTranslation(a, false)];
        }
        if(p2pots == 0){
            intPots[getTranslation(-1, false)]  += intPots[getTranslation(a, true)];
            intPots[getTranslation(-1, false)] += intPots[getTranslation(a, false)];
        }
    }
    return;
}

int MAX_GAMES = 5;

void playGame(int player1, int player2){
    board objBord(player1, player2);
    while(objBord.isPlaying())
    {
        objBord.getNextMove();
    }
    objBord.getResults();
}

int main() {

    int server_fd, player1, player2;
    struct sockaddr_in addr;    // initialize a server port
    int opt = 1;
    int addrlen = sizeof(addr);
    pid_t child_pid;
    int GamesRunning = 0;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 3);

    while (true) {
        if (GamesRunning < MAX_GAMES) {
            player1 = accept(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
            if (player1 < 0) { 
                cerr << "Error accepting player nr1" << endl;
                continue;
            }
            cout << "Player 1 connected." << endl;  // creatinng the first palyer and waiting for the second one

            player2 = -1;
            while (player2 < 0) { // creating the second player
                player2 = accept(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
                if (player2 < 0) {
                    std::cerr << "Error accepting playernr2." << endl;
                }
            }
            cout << "Player 2 connected. Game on." << endl;

            child_pid = fork(); // fork child is handling the game
            if (child_pid == 0) {
                playGame(player1, player2);
                close(player1);
                close(player2);
                exit(0);
            } else if (child_pid > 0) { // fork parent is waiting for the next game
                close(player1);
                close(player2);
                GamesRunning++;
            } else {
                cerr << "Error creating fork." << endl;
                return 1;
            }
        }
            // waiting for the ending the game 
        waitpid(-1, NULL, WNOHANG);
        GamesRunning--;
    }

    close(server_fd);
    return 0;
}
