#pragma once

#define GREEN		"\033[0;92m"
#define BLUE		"\033[0;94m"
#define RED			"\033[0;31m"
#define YELLOW		"\033[93m"
#define END_COLOR	"\033[0m"

#define ERR -1
#define SUCCESS 0
#define FAIL 1

#define MAX_CHAN_USER 100
#define MAX_CHAN_TOTAL 1000
#define CHAN_USER_LIMIT 100
#define MAX_EVENTS 42
#define MAX_MSG_LEN 512
#define BUF_SIZE 1024

enum logMsgType { INFO, WARNING, ERROR, DEBUG };

#define DEBUG_MODE true // change it to false during evaluation
