/*
 *	kernel.c
 */
 
void kmain(void) {
	const char *str = "Welcome to Yornel";
	char *vidptr = (char*) 0xb8000;	//video mem starts here
	unsigned int i = 0;
	unsigned int j = 0;
	
	while(i < 80 * 25 * 2) {
		vidptr[i] = ' ';
		//attribute byte - light grey on black
		vidptr[i + 1] = 0x07;
		i = i + 2;
	}
	
	i = 0;
	
	while(str[i] != '\0') {
		vidptr[j] = str[i];
		vidptr[j+1] = 0x07;
		i++;
		j = j + 2;
	}
	
	return;
}
