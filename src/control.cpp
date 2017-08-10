#include "control.h"
#include "defs.h"
#include "timer.h"

#include <iostream>
#include <sstream>
#include <functional>
#include <unordered_map>
#include <thread>

static std::thread *threadSearch;
static bool isSearching;

int getMove(S_BOARD *pos, std::string str) {
    if (str.length() < 4) return 0;
    if (str.length() > 5) return 0;

    char f = str[0];
    int file = f - 'a';
    char r = str[1];
    int rank = r - '1';
    if (file < FILE_A || file > FILE_H || rank < RANK_1 || rank > RANK_8) return 0;
    int from = FR2SQ(file, rank);

    f = str[2];
    file = f - 'a';
    r = str[3];
    rank = r - '1';
    if (file < FILE_A || file > FILE_H || rank < RANK_1 || rank > RANK_8) return 0;
    int to = FR2SQ(file, rank);

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    for (int i = 0; i < list->count; i++) {
        if ((FROMSQ(list->moves[i].move) == from) && (TOSQ(list->moves[i].move) == to)) {
            if (MakeMove(pos, list->moves[i].move)) {
                TakeMove(pos);
                if (PROMOTED(list->moves[i].move)) {
                    int prom = wQ;

                    if (str.length() == 5) {
                        char p = str[4];
                        switch (p)
                        {
                        case 'q': break;
                        case 'r': prom = wR; break;
                        case 'b': prom = wB; break;
                        case 'n': prom = wN; break;
                        }
                    }

                    prom += (pos->side * 6); //black = 1 and wQ + 6 = bQ

                    return MOVE(from, to, CAPTURED(list->moves[i].move), prom, 0);
                }
                else {
                    return list->moves[i].move;
                }
            }
            else {
                return 0;
            }
        }
    }
    return 0;
}

bool id(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	std::cout << "id name " << NAME << std::endl;
	std::cout << "uciok" << std::endl;

	return true;
}

bool ready(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	std::cout << "readyok" << std::endl;
	return true;
}

bool stop(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	info->stopped = true;
	if (isSearching) {
		threadSearch->join();
		delete(threadSearch);
		isSearching = false;
	}

	return true;
}

bool quit(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	stop(line, board, info);
	return false;
}

bool reset(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	info->stopped = true;
	ParseFen(START_FEN, board);
	return true;
}

bool searchBoard(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	info->stopped = true;
	int d = MAXDEPTH;

	if (line.length() > 1) {
		d = atoi(line.substr(1).c_str());
	}

	info->depth = d;
	info->stopped = false;

	isSearching = true;
	threadSearch = new std::thread(SearchPosition, board, info);

	return true;
}

bool go(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	info->stopped = true;
	int wtime = 0;
	int btime = 0;
	int winc = 0;
	int binc = 0;
	int movesleft = MOVES_LEFT_DEFAULT;
	int depth = MAXDEPTH;
	int movetime = 0;
	bool infinite = false;

	if (line.length() > 0) {
		line = line.substr(1);
		std::string buffer;
		std::stringstream ss(line);

		while (ss >> buffer) {
			std::string key = "wtime";
			if (!buffer.compare(key)) {
				ss >> buffer;
				wtime = atoi(buffer.c_str());
				continue;
			}
			key = "btime";
			if (!buffer.compare(key)) {
				ss >> buffer;
				btime = atoi(buffer.c_str());
				continue;
			}
			key = "winc";
			if (!buffer.compare(key)) {
				ss >> buffer;
				winc = atoi(buffer.c_str());
				continue;
			}
			key = "binc";
			if (!buffer.compare(key)) {
				ss >> buffer;
				binc = atoi(buffer.c_str());
				continue;
			}
			key = "movestogo";
			if (!buffer.compare(key)) {
				ss >> buffer;
				movesleft = atoi(buffer.c_str());
				continue;
			}
			key = "depth";
			if (!buffer.compare(key)) {
				ss >> buffer;
				depth = atoi(buffer.c_str());
				continue;
			}
			key = "movetime";
			if (!buffer.compare(key)) {
				ss >> buffer;
				movetime = atoi(buffer.c_str());
				continue;
			}
			key = "infinite";
			if (!buffer.compare(key)) {
				infinite = true;
			}
		}
	}

	info->depth = depth;
	info->stopped = false;
			
	int side = board->side;
	threadSearch = new std::thread(SearchPosition, board, info);

	if (infinite) {

	}
	else {
		if (movetime != 0) {
			startTimer(info, movetime);
		}
		else {
			if (side == BLACK) {
				startTimer(info, btime, binc, movesleft);
			}
			else {
				startTimer(info, wtime, winc, movesleft);
			}
		}
	}

	isSearching = true;

	return true;
}

bool startpos(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	info->stopped = true;
	ParseFen(START_FEN, board);

	if (line.length() > 0) {
		line = line.substr(1);
		std::string buffer;
		std::stringstream ss(line);

		ss >> buffer;
		std::string key = "moves";
		if (!buffer.compare(key)) {
			while (ss >> buffer) {
				int move = getMove(board, buffer);
				if (move != 0) MakeMove(board, move);
				else break;
			}
		}
	}

	return true;
}

bool move(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	info->stopped = true;
	int move = getMove(board, line.substr(1));
    if (move != 0) MakeMove(board, move);
	else std::cout << "illegal move!" << std::endl;
	return true;
}

bool take(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	info->stopped = true;
	if (board->hisPly > 0) TakeMove(board);
	return true;
}

bool print(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	info->stopped = true;
	PrintBoard(board);

	return true;
}

bool book(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	info->stopped = true;
	S_MOVELIST list[1];

    GetBookMove(board);

	return true;
}

bool fen(std::string line, S_BOARD* board, S_SEARCHINFO* info) {
	info->stopped = true;
    if (line.length() > 0) {
        line = line.substr(1);

        char fen[100];
        std::strcpy(fen, line.c_str());
		ParseFen(fen, board);

		std::string buffer;
		std::stringstream ss(line);

		ss >> buffer;
		std::string key = "moves";
		if (!buffer.compare(key)) {
			while (ss >> buffer) {
				int move = getMove(board, buffer);
				if (move != 0) MakeMove(board, move);
			}
		}
	}

	return true;
}

void controlLoop(S_BOARD* board, S_SEARCHINFO* info) {
	bool running = true;

    info->GAME_MODE = UCIMODE;

	std::cout << "uci protocol" << std::endl;

	std::unordered_map<std::string, std::function<bool(std::string, S_BOARD*, S_SEARCHINFO*)>> commands;
	commands.emplace("isready", ready);
	commands.emplace("stop", stop);
	commands.emplace("quit", quit);
	commands.emplace("exit", quit);
	commands.emplace("ucinewgame", reset);
	commands.emplace("position startpos", startpos);
	commands.emplace("position fen", fen);
	commands.emplace("print", print);
	commands.emplace("move", move);
	commands.emplace("take", take);
	commands.emplace("uci", id);
	commands.emplace("search", searchBoard);
	commands.emplace("go", go);
	commands.emplace("book", book);

    reset("", board, info);

	while (running) {
		std::string line;
		getline(std::cin, line);

		for (auto it = commands.begin(); it != commands.end(); it++) {
			std::string command = it->first;
			auto func = it->second;
			if (!line.substr(0, command.length()).compare(command)) {
				running = func(line.substr(command.length()), board, info);
			}
		}
				
	}

	std::cout << "shutting down..." << std::endl;
}
