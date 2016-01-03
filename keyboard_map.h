/*
 * The following array is taken from 
 * http://www.osdever.net/bkerndev/Docs/keyboard.htm
 * All credits where due
 */

#define ESC 0

#define LSFT_CODE 0x2a
#define RSFT_CODE 0x36
#define LSFT 0
#define RSFT 0

#define LCTL 0
#define LALT 0

#define META 0

#define CLCK 0
#define NLCK 0
#define SLCK 0

#define INS 0
#define HOME 0
#define PGUP 0
#define DEL 127
#define END 0
#define PGDN 0

#define UPAR 0
#define LTAR 0
#define RTAR 0
#define DNAR 0

#define F1 0
#define F2 0
#define F3 0
#define F4 0
#define F5 0
#define F6 0
#define F7 0
#define F8 0
#define F9 0
#define F10 0
#define F11 0
#define F12 0

unsigned char keyboard_map[128] = {
	0, ESC, '1', '2', '3', '4', '5', '6', '7', '8',
	'9', '0', '-', '=', '\b', '\t',	'q', 'w', 'e', 'r',
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', LCTL,
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '`', LSFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
	'm', ',', '.', '/', RSFT, '*', LALT, ' ', CLCK, F1,
	F2, F3, F4, F5, F6, F7, F8, F9, F10, NLCK,
	/* Keypad */
	SLCK, HOME, UPAR, PGUP, '-', LTAR, '5', RTAR, '+', END,
	DNAR, PGDN, INS, DEL, 0, 0, 0, F11, F12, 0,
	
	//don't exist on normal keyboards
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 99 */,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 109 */,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* 119 */,
	0, 0, 0, 0, 0, 0, 0, 0 /* 127 */
};

unsigned char shift_map[52] = {
	/* 2 offset */
	'!', '@', '#', '$', '%', '^', '&', '*',
	'(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',
	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', LCTL,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'"', '~', LSFT, '|', 'Z', 'X', 'C', 'V', 'B', 'N',
	'M', '<', '>', '?'
};

unsigned char nlck_map[13] = {
	/* Keypad - 71 offset*/
	'7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.'
};

