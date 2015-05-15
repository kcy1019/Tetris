#include "tetris.h"

static struct sigaction act, oact;

int main()
{
	int exit=0;
	putenv("TERM=xterm-256color");
	initscr();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	start_color();
	Init_Pairs();

	/**************************************************
	 * Another Fine Solution by kcy1019 aka WeissBlume
	 * (gkcy1019@gmail.com).
	 **************************************************/
	
	// Something like genetic :)***********************************************
	HOLE_MULTIPLIER         = -9;// + (rand() % 11) * (rand()%2 ? -1 : 1);
	HEIGHT_MULTIPLIER       = -155;// + (rand() % 11) * (rand()%2 ? -1 : 1);
	WALL_TOUCH_MULTIPLIER   = 23;// + (rand() % 11) * (rand()%2 ? -1 : 1);
	BLOCK_TOUCH_MULTIPLIER  = 32;// + (rand() % 11) * (rand()%2 ? -1 : 1);
	FLOOR_TOUCH_MULTIPLIER  = 59;// + (rand() % 17) * (rand()%2 ? -1 : 1);
	LINE_CLEAR_MULTIPLIER   = 1005;// + (rand() % 23) * (rand()%2 ? -1 : 1);
	LINE_CLEAR_EXPONENT     = 3;// + (rand() % 2 ? 1 : 0) * (rand()%2 ? -1 : 1);
	BLOCKADE_PENALTY        = -61;// + (rand() % 7) * (rand()%2 ? -1 : 1);
	LINE_DELTA_EXPONENT		= 0;// + (rand() % 2) * (rand()%2 ? -1 : 1);
	// *************************************************************************

	srand(time(NULL));

	while(!exit) {
		clear();
		switch(menu()) {
			case MENU_PLAY: play(1);break;
			case MENU_RANK: rank();break;
			case MENU_REC_PLAY: recommendedPlay(); break;
			case MENU_EXIT: exit=1;break;
			default: break;
		}
	}

	endwin();
	system("clear");
	return 0;
}

void InitTetris()
{
	int i,j;

	memset(field, 0, sizeof(field));
	for (j=0;j<HEIGHT;j++)
		for (i=0;i<WIDTH;i++)
			field[j][i]=0;

	for (i = 0; i < BLOCK_NUM; i++) {
		nextBlock[i] = rand() % 7;
	}
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;
	gameOver=0;
	timed_out=0;

	DrawOutline();
	DrawField();
	DrawBlock(blockY,blockX,nextBlock[0],blockRotate,' ');
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline()
{
	int i,j;
	DrawBox(0,0,HEIGHT,WIDTH + 1);

	move(2,WIDTH * 2 + 10);
	attron(COLOR_PAIR(BACKGROUND));
	printw("NEXT BLOCK");
	attroff(COLOR_PAIR(BACKGROUND));
	DrawBox(3,WIDTH + 5,5,8);
	DrawBox(9,WIDTH + 5,4,8);

	move(16,WIDTH * 2 + 10);
	attron(COLOR_PAIR(BACKGROUND));
	printw("SCORE");
	attroff(COLOR_PAIR(BACKGROUND));
	DrawBox(17,WIDTH + 5,1,8);
	
	move(5,WIDTH * 2 + 32);
	attron(COLOR_PAIR(BACKGROUND));
	printw("HOLD");
	attroff(COLOR_PAIR(BACKGROUND));
	DrawBox(6, WIDTH + 16, 5, 8);
}

int GetCommand()
{
	int command, earn = 0;
	
	command = wgetch(stdscr);
	switch(command) {
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		GetBottomMost(field, &blockY, &blockX, nextBlock[0], blockRotate);
		timed_out = 1;
		alarm(0);
		BlockDown(0);
		return KEY_DOWN;
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	case '`':
		Hold();
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command)
{
	int ret=1;
	int drawFlag=0;
	switch(command) {
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if ((drawFlag = CheckToRotate(field,nextBlock[0],(blockRotate+1)%4,&blockY,&blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if ((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if ((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if ((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		drawFlag = 1;
		break;
	}
	if (drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;
}

// #DrawField
void DrawField()
{
	int i,j;
	for (j=0;j<HEIGHT;j++) {
		for (i=0;i<WIDTH;i++) {
			if (field[j][i]) {
				attron(A_REVERSE);
				attron(COLOR_PAIR(field[j][i]));
				DrawTile(j + 1, i + 1, ' ');
				attroff(COLOR_PAIR(field[j][i]));
				attroff(A_REVERSE);
			}
			else {
				attron(COLOR_PAIR(BACKGROUND));
				DrawTile(j + 1, i + 1, '.');
				attroff(COLOR_PAIR(BACKGROUND));
			}
		}
	}
}

// #RDrawField
void RDrawField(char field[HEIGHT][WIDTH])
{
	int i,j;
	for (j=0;j<HEIGHT;j++) {
		for (i=0;i<WIDTH;i++) {
			if (field[j][i]) {
				attron(A_REVERSE);
				attron(COLOR_PAIR(field[j][i]));
				DrawTile(HEIGHT + j + 1, i + 1, ' ');
				attroff(COLOR_PAIR(field[j][i]));
				attroff(A_REVERSE);
			}
			else {
				attron(COLOR_PAIR(BACKGROUND));
				DrawTile(HEIGHT + j + 1, i + 1, '.');
				attroff(COLOR_PAIR(BACKGROUND));
			}
		}
	}
}


void PrintScore(int score)
{
//	move(11,WIDTH+11);
	attron(COLOR_PAIR(BACKGROUND));
	move(18, 2 * WIDTH + 11);
	printw("%10d",score);
	attroff(COLOR_PAIR(BACKGROUND));
}

void DrawNextBlock(int *nextBlock)
{
	int i, j, k;
	for ( k = 0; k < 2; k++ ) {
		for ( i = 0; i < 4; i++ ) {
			for ( j = 0; j < 4; j++ ) {
				if ( block[nextBlock[k + 1]][0][i][j] ) {
					attron(A_REVERSE);
					attron(COLOR_PAIR((nextBlock[k + 1] + 1)));
					DrawTile(4 + i + k * 6, j + WIDTH + 7, ' ');
					attroff(COLOR_PAIR((nextBlock[k + 1] + 1)));
					attroff(A_REVERSE);
				}
				else {
					DrawTile(4 + i + k * 6, j + WIDTH + 7, ' ');
				}
			}
		}
	}
}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile)
{
	int i,j;
	for (i=0;i<4;i++)
		for (j=0;j<4;j++) {
			if (block[blockID][blockRotate][i][j] && i+y>=0) {
				attron(A_REVERSE);
				attron(COLOR_PAIR((blockID + 1)));
				DrawTile(i + y + 1, j + x + 1, tile);
				attroff(COLOR_PAIR((blockID + 1)));
				attroff(A_REVERSE);
			}
		}
}

void DrawBox(int y,int x, int height, int width)
{
	int i,j;
	attron(COLOR_PAIR(BACKGROUND));
	move(y, x * 2);		//	addch(ACS_ULCORNER);
	addch('+');
	for (i=0;i<width * 2;i++)		//		addch(ACS_HLINE);
		addch('-');		//	addch(ACS_URCORNER);
	addch('+');
	for (j=0;j<height;j++) {
		move(y+j+1, x * 2);		//		addch(ACS_VLINE);
		addch('|');
		move(y+j+1, x * 2 + width * 2 + 1);		//		addch(ACS_VLINE);
		addch('|');
	}
	move(y+j+1, x * 2);		//	addch(ACS_LLCORNER);
	addch('+');
	for (i=0;i<width * 2;i++)		//		addch(ACS_HLINE);
		addch('-');		//	addch(ACS_LRCORNER);
	addch('+');
	attroff(COLOR_PAIR(BACKGROUND));
}

void play(int rec)
{
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();

	recommendX = -1;
	if(rec) recommendX = 0;
	CallRecommend();

	do {

		if (timed_out==0) {
			alarm(1);
			timed_out=1;
		}

		if (!gameOver) {
			command = GetCommand();
		}

		if (!gameOver && ProcessCommand(command)==QUIT) {
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,11);
			move(HEIGHT/2,WIDTH/2-4);
			attron(COLOR_PAIR(BACKGROUND));
			printw("      Good-bye!!    ");
			attroff(COLOR_PAIR(BACKGROUND));
			refresh();
			getch();

			return;
		}
		
	} while (!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,11);
	move(HEIGHT/2,WIDTH/2-4);
	init_pair(BACKGROUND, 156, 16);
	attron(COLOR_PAIR(BACKGROUND));
	printw("       GameOver!!    ");
	attroff(COLOR_PAIR(BACKGROUND));
	refresh();
	getch();
	newRank(score, 0);
}

char menu()
{
	int i, j, len, x, col = -1;

	for (i = 0; i < 8; i++) {
		len = strlen(Logo[i]);
		x = 0;
		for (j = 0; j < len; j++) {
			if ('A' <= Logo[i][j] && Logo[i][j] <= 'Z') {
				if (~col) attron(COLOR_PAIR(col));
				col = Logo[i][j] - 54;
				attron(COLOR_PAIR(col));
			}
			else {
				mvprintw(i, x++, "%c", Logo[i][j]);
			}
		}
	}
	printw("\n             Created By kcy1019 (also known as WeissBlume)\n");
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
//	printw("5. play without recommendation\n");
	attroff(COLOR_PAIR(col));
	return wgetch(stdscr);
}

// #CheckToMove
int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX)
{
	int i, j, fail = 0;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (block[currentBlock][blockRotate][i][j]) {
				fail |= f[i + blockY][j + blockX];
				fail |= i + blockY >= HEIGHT;
				fail |= i + blockY < 0;
				fail |= j + blockX < 0;
				fail |= j + blockX >= WIDTH;
				if (fail) return 0;
			}
		}
	}
	return !fail;
}

int CheckToRotate(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int* blockY, int* blockX)
{
	int fail = 0, i, j, bx = *blockX, by = *blockY, minx = 100, miny = 100, maxx = -1, maxy = -1;
	For4(i) {
		For4(j) {
			If_Block {
				if (f[i + *blockY][j + *blockX]) return 0;
				maxx = maxx > j ? maxx : j;
				maxy = maxy > i ? maxy : i;
				minx = minx < j ? minx : j;
				miny = miny < i ? miny : i;
			}
		}
	}

	while (minx + *blockX < 0) ++(*blockX);
	while (miny + *blockY < 0) ++(*blockY);
	while (maxx + *blockX >= WIDTH) --(*blockX);
	while (maxy + *blockY >= HEIGHT) --(*blockY);
	

	return 1;

	if (*blockX + maxx < WIDTH && *blockX + minx >= 0 && *blockY + miny >= 0 && *blockY + maxy < HEIGHT) {
		return 1;
	}
	else {
		*blockX = bx;
		*blockY = by;
		return 0;
	}
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX)
{
	DrawOutline();
	DrawField();
	DrawBlock(blockY, blockX, currentBlock, blockRotate, ' ');
	DrawHoldBlock(holdBlock);
	/*if (~recommendX)*/ DrawRecommend(recommendY, recommendX, currentBlock, recommendR);
	DrawShadow(blockY, blockX, currentBlock, blockRotate);
}

void BlockDown(int sig)
{
	int currentBlock = nextBlock[0], earn, i, j, recsc;
	if (prev3000 < score / 3000) {
		init_pair(BACKGROUND, rand() % 157 + 33 + (score / 3000), 16); 
		prev3000 = score / 3000;
	}

	if (CheckToMove(field, currentBlock, blockRotate, blockY + 1, blockX)) {
		++blockY;
		DrawChange(field, KEY_DOWN, currentBlock, blockRotate, blockY, blockX);
	}
	else {
		if (!CheckToMove(field, currentBlock, blockRotate, blockY, blockX)) {
			gameOver = 1;
			timed_out = 0;
			return;
		}
		For4(i) {
			For4(j) If_Block
				if (blockY + i < 0) {
						gameOver = 1;
						timed_out = 0;
						return;
				}
		}

		recommendX = recommendY = recommendR = 0;

		score += AddBlockToField(field, currentBlock, blockRotate, blockY, blockX, 0);
		earn = DeleteLine(field);
		score += earn * earn * 100;

		for (i = 0; i < BLOCK_NUM - 1; i++) {
			nextBlock[i] = nextBlock[i + 1];
		}
		nextBlock[BLOCK_NUM - 1] = rand() % 7;

		DrawNextBlock(nextBlock);
		blockX = WIDTH / 2 - 2;
		blockY = -1;
		recsc = CallRecommend();

		PrintScore(score);
		DrawField();
	}
	timed_out = 0;
}

//#addblocktofield
int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX, int mode)
{
	int dx[] = {-1, 0, 1}, dy[] = {0, 1, 0};
	int i, j, ret = 0, k, nx, ny, x = blockX, y = blockY;
	For4(i) {
		For4(j)
			If_Block {
				if (mode) {
					for (k = 0; k < 3; k++) {
						nx = x + j + dx[k], ny = y + i + dy[k];
						if (ny >= HEIGHT) {
							ret += FLOOR_TOUCH_MULTIPLIER;
						} else if (nx < 0 || nx >= WIDTH) {
							ret += WALL_TOUCH_MULTIPLIER;
						} else if (ny >= 0 && field[ny][nx]) {
							ret += BLOCK_TOUCH_MULTIPLIER;
						}
					}
					f[blockY + i][blockX + j] = currentBlock + 1;
				} else {
					f[blockY + i][blockX + j] = currentBlock + 1;
					ret += ((blockY + i + 1 >= HEIGHT || f[blockY + i + 1][blockX + j]))
						* 10;
				}
			}
	}

	dropped_cnt += (!mode);

	return ret;
}

int DeleteLine(char f[HEIGHT][WIDTH])
{
	int i, j, k, row_clear, score = 0, recursive = 0;
	for (i = HEIGHT - 1; i >= 0; i--) {
		row_clear = 1;
		for (j = 0; j < WIDTH; j++) {
			row_clear &= !!f[i][j];
		}
		recursive |= row_clear;
		if (row_clear) {
			score++;
			for (k = i; k > 0; k--) {
				for (j = 0; j < WIDTH; j++) {
					f[k][j] = f[k-1][j];
				}
			}
		}
	}
	if (recursive) score += DeleteLine(f);
	return score;
}

void GetBottomMost(char field[HEIGHT][WIDTH], int *y, int* x, int blockID, int blockRotate)
{
	int yptr, out = 0, i, j, k;

	for (yptr = 0; yptr + *y < HEIGHT; ++yptr) {
		For4(i) {
			For4(j) {
				if (*y + yptr + i >= 0 && block[blockID][blockRotate][i][j]) {
					if (field[i + *y + yptr][j + *x]) {
						out = 1;
					}
				}
				if (block[blockID][blockRotate][i][j] && i + *y + yptr >= HEIGHT) {
					out = 1;
				}
				if (out) break;
			}
			if (out) break;
		}
		if (out) break;
	}
	*y = *y + --yptr;
}

void DrawShadow(int y, int x, int blockID,int blockRotate)
{
	GetBottomMost(field, &y, &x, blockID, blockRotate);
	DrawBlock(y, x, blockID, blockRotate, '\\');
}

void createRankList()
{
	FILE* fp;
	rankptr ptr, pLast;
	int i, j, k;

	fp = fopen("rank.txt", "r");

	pLast = Head = (rankptr) calloc(sizeof(ranknode), 1);
	Head -> prev = Head -> next = NULL;

	if (fp != NULL) {

		fscanf(fp, "%d\n", &rankNum);

		for (i = 1; i <= rankNum; i++) {
			ptr = (rankptr) calloc(sizeof(ranknode), 1);
			fscanf(fp, "%d%*c%[^\n]\n", &(ptr -> key), ptr -> name);
			pLast -> next = ptr;
			ptr -> prev = pLast;
			pLast = ptr;
		}

		fclose(fp);
	} else {
		rankNum = 0;
	}
}

void RankTable(void)
{
	int i;
	for (i = 0; i < 8; i++) printw("  ");
	printw("name");
	for (i = 0; i < 8; i++) printw("  ");
	printw("|");
	printw("   score   \n");
	for (i = 0; i < 49; i++) printw("-");
	printw("\n");
}

void rank()
{
	FILE* fp;
	char buf[256];
	int cmd, i, j, k, N;
	rankptr ptr, pLast;
	clear();
	attron(COLOR_PAIR(LIME));
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by specific name\n");
	printw("3. delete a specific rank\n");

	while((cmd = wgetch(stdscr)) == ERR);

	switch(cmd) {
		case '1':

			MODE = 1;

			createRankList();
			N = rankNum;
			memset(buf, 0, sizeof(buf));
			printw("X: ");

			echo();

			LineInput(buf);
			if (*buf == 0) {
				X = 1;
			} else {
				X = atoi(buf);
			}
			printw("X: %d", X);

			memset(buf, 0, sizeof(buf));
			printw("\nY: ");
			LineInput(buf);
			if (*buf == 0) {
				Y = N;
			} else {
				Y = atoi(buf);
			}
			printw("Y: %d", Y);

			noecho();
			printw("\nX : %d, Y : %d\n", X, Y);
			if (X <= Y && 1 <= X && Y <= N) {

				RankTable();
				PrintList(Head);

			} else {
				printw("\nsearch failure: no rank in the list\n");
			}

			while (wgetch(stdscr) == ERR);

			break;
		case '2':
			createRankList();
			printw("input the name: ");
			found = 0;
			MODE = 2;
			echo();
			LineInput(target);
			noecho();
			RankTable();
			PrintList(Head);
			if (!found) {
				printw("\nsearch failure: no name in the list\n");
			}
			wgetch(stdscr);
			break;
		case '3':
			createRankList();
			printw("input the rank: ");
			echo();
			LineInput(buf);
			noecho();
			if (*buf < '0' || *buf > '9') {
				printw("\nsearch failure: the rank not in the list\n");
				wgetch(stdscr);
				return;
			}
			j = atoi(buf);
			i = 0;
			found = 0;
			pLast = Head;
			for (ptr = Head -> next; ptr != NULL; ptr = ptr -> next) {
				++i;
				if (i == j) {
					pLast = ptr -> prev;
					pLast -> next = ptr -> next;
					if (ptr -> next) ptr -> next -> prev = pLast;
					--rankNum;
					free(ptr);
					found = 1;
					printw("result: the rank deleted\n");
					break;
				}
			}
			if (!found) printw("\nsearch failure: the rank not in the list\n");
			else writeRankFile();
			wgetch(stdscr);
			break;
	}

	attroff(COLOR_PAIR(LIME));
}

void writeRankFile()
{
	int i;
	rankptr ptr;
	FILE* fp = fopen("rank.txt", "w");

	fprintf(fp, "%d\n", rankNum);

	for (ptr = Head -> next; ptr != NULL; ptr = ptr -> next) {
		fprintf(fp, "%d %s\n", ptr -> key, ptr -> name);
	}

	fclose(fp);
}

void newRank(int score, int mode)
{
	int wrote = 0;
	rankptr ptr, newnode = calloc(sizeof(ranknode), 1), prevptr;
	clear();
	newnode -> key = score;
	createRankList();
	rankNum++;
	prevptr = Head;

	for (ptr = Head -> next; ptr != NULL; ptr = ptr -> next) {
		if (ptr -> key < score) {
			ptr -> prev -> next = newnode;
			newnode -> next = ptr;
			newnode -> prev = ptr -> prev;
			ptr -> prev = newnode;
			wrote = 1;
			break;
		}
		prevptr = ptr;
	}

	if (!wrote) {
		prevptr -> next = newnode;
		newnode -> prev = prevptr;
	}

	if (mode) {
	/*	
		sprintf(newnode -> name, "<%d %d %d %d %d %d %d %d %d>",
				HOLE_MULTIPLIER,
				HEIGHT_MULTIPLIER,
				WALL_TOUCH_MULTIPLIER,
				BLOCK_TOUCH_MULTIPLIER,
				FLOOR_TOUCH_MULTIPLIER,
				LINE_CLEAR_MULTIPLIER,
				LINE_CLEAR_EXPONENT,
				BLOCKADE_PENALTY,
				LINE_DELTA_EXPONENT
				);
	*/	
		sprintf(newnode -> name, "Seed: %08X", seed);
	} else {
		attron(COLOR_PAIR(SKY));
		printw("What's your name? ");
		echo();
		LineInput(newnode -> name);
		noecho();
		attroff(COLOR_PAIR(SKY));
	}
	writeRankFile();
}

void DrawRecommend(int y, int x, int blockID,int blockRotate)
{
	DrawBlock(y, x, blockID, blockRotate, 'R');
}

int recommend(const recptr node)
{
	recnode node2;
	int i, j, k, l, ret =-1, rot, x, y, z, curscore, curearn, clev = node -> lev, rot_lim;

	switch(nextBlock[node -> lev]) {
		case 0:
		case 5:
		case 6:
			rot_lim = 2;
			break;
		case 4:
			rot_lim = 1;
			break;
		default:
			rot_lim = 4;
	}


	for (rot = 0; rot < rot_lim; rot++) {
		for (i = -2; i <= WIDTH + 2; i++) {
			if (!CheckToMove(node -> field, nextBlock[clev], rot, 0, i)) continue;
			
			k = 0;
			memcpy((&node2) -> field, node -> field, sizeof(field));
			(&node2) -> lev = clev + 1;

			GetBottomMost((&node2) -> field, &k, &i, nextBlock[clev], rot);

			curscore = curearn = 0;
			curscore = AddBlockToField((&node2) -> field, nextBlock[clev], rot, k, i, 1);
			
			curearn = DeleteLine((&node2) -> field);

			j = curearn * curearn;

			curscore += j * LINE_CLEAR_MULTIPLIER;

			if (((&node2) -> lev) < BLOCK_NUM) {
				curscore += recommend(&node2);
			}

			if (ret <= curscore) {
				
				if (clev == 0) {
					if (ret < curscore || k > recommendY) {
						recommendX = i;
						recommendY = k;
						recommendR = rot;
					}
				}
				ret = curscore;
#ifdef RDEBUG
				RDrawField((&node2) -> field);
				mvprintw(HEIGHT * 2, WIDTH * 2 + 2, "                                ");
				mvprintw(HEIGHT * 2, WIDTH * 2 + 2, "ret : %d, scr : %d", ret, curscore);
				mvprintw(HEIGHT * 2 - 2, WIDTH * 2 + 2, "rex : %d, rey : %d", recommendX, recommendY);
				mvprintw(HEIGHT * 2 - 1, WIDTH * 2 + 2, "cux : %d, cuy : %d", i, k);
#endif
			}

		} // Width loop 
	} // rotation loop

	return ret;
}

// #recommend
int modified_recommend(const recptr node)
{
	recnode node2;
	int i, j, k, l, ret =-1, rot, x, y, z, curscore, curearn, clev = node -> lev, rot_lim;

	switch(nextBlock[node -> lev]) {
		case 0:
		case 5:
		case 6:
			rot_lim = 2;
			break;
		case 4:
			rot_lim = 1;
			break;
		default:
			rot_lim = 4;
	}


	for (rot = 0; rot < rot_lim; rot++) {
		for (i = -2; i <= WIDTH + 2; i++) {
			if (!CheckToMove(node -> field, nextBlock[clev], rot, 0, i)) continue;
			
			k = 0;
			memcpy((&node2) -> field, node -> field, sizeof(field));
			(&node2) -> lev = clev + 1;

			GetBottomMost((&node2) -> field, &k, &i, nextBlock[clev], rot);

			curscore = curearn = 0;
			curscore = AddBlockToField((&node2) -> field, nextBlock[clev], rot, k, i, 1);
			
			curearn = DeleteLine((&node2) -> field);

			j = curearn;

			for (l = 1; l < LINE_CLEAR_EXPONENT; l++) {
				j *= curearn;
			}

			curscore += j * LINE_CLEAR_MULTIPLIER;

			if (((&node2) -> lev) < BLOCK_NUM) {
				curscore += modified_recommend(&node2);
			} else {
				// Here the magic occurs! XD
				curscore += CalcField((&node2) -> field);
			}

			if (ret <= curscore) {
				
				if (clev == 0) {
					if (ret < curscore || k > recommendY) {
						recommendX = i;
						recommendY = k;
						recommendR = rot;
					}
				}
				ret = curscore;
#ifdef RDEBUG
				RDrawField((&node2) -> field);
				mvprintw(HEIGHT * 2, WIDTH * 2 + 2, "                                ");
				mvprintw(HEIGHT * 2, WIDTH * 2 + 2, "ret : %d, scr : %d", ret, curscore);
				mvprintw(HEIGHT * 2 - 2, WIDTH * 2 + 2, "rex : %d, rey : %d", recommendX, recommendY);
				mvprintw(HEIGHT * 2 - 1, WIDTH * 2 + 2, "cux : %d, cuy : %d", i, k);
#endif
			}

		} // Width loop 
	} // rotation loop

	return ret;
}

void recommendedPlay()
{
	int command, ielap;
	double elap;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	alarm(0);
	timed_out = 0;
	CallRecommend();
	start_time = (double) clock();
	dropped_cnt = 0;
	timeout(0);

	do {
		switch(wgetch(stdscr)) {
			case 'q': case 'Q': command = QUIT; break;
			default: BlockDown(1); command = KEY_DOWN;
		}
		
		if (!gameOver) {
			blockY = recommendY;
			blockX = recommendX;
			blockRotate = recommendR;
		}

		elap = (double) clock() - start_time;
		elap /= CLOCKS_PER_SEC;
		ielap = (int) elap;
		
		mvprintw(HEIGHT - 2, WIDTH * 2 + 5, "Number of dropped Blocks: %d\n", dropped_cnt);
		mvprintw(HEIGHT - 1, WIDTH * 2 + 5, "Score-per-drop: %d\n", (int) (score / (dropped_cnt ? dropped_cnt : 1)));
		mvprintw(HEIGHT, WIDTH * 2 + 5, "Elapsed: %2dh %2dm %2ds", ielap / 3600, ielap % 3600 / 60, ielap % 60);
		mvprintw(HEIGHT + 1, WIDTH * 2 + 5, "Score-per-sec: %d\n", (int) (score / elap));

		if (!gameOver && ProcessCommand(command)==QUIT) {
			timeout(-1);
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,11);
			move(HEIGHT/2,WIDTH/2-4);
			attron(COLOR_PAIR(BACKGROUND));
			printw("      Good-bye!!    ");
			attroff(COLOR_PAIR(BACKGROUND));
			refresh();
			getch();

			return;
		}

	} while (!gameOver);

	timeout(-1);
	alarm(0);
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,11);
	move(HEIGHT/2,WIDTH/2-4);
	init_pair(BACKGROUND, 156, 16);
	attron(COLOR_PAIR(BACKGROUND));
	printw("       GameOver!!    ");
	attroff(COLOR_PAIR(BACKGROUND));
	refresh();
	getch();
	newRank(score, 0);
}

void DrawTile(int y, int x, char tile)
{
	move((int)y, (int)(x * 2));
	addch(tile);
	move((int)y, (int)(x * 2) + 1);
	addch(tile);
}
void Init_Pairs(void)
{
	init_pair(GRA0, 51, 16);
	init_pair(GRA1, 50, 16);
	init_pair(GRA2, 49, 16);
	init_pair(GRA3, 48, 16);
	init_pair(GRA4, 47, 16);
	init_pair(GRA5, 46, 16);
	init_pair(GRA6, 82, 16);
	init_pair(GRA7, 83, 16);
	init_pair(GRA8, 84, 16);
	init_pair(GRA9, 85, 16);

	init_pair(RED	, 198, 16);
	init_pair(VIOLET, 177, 16);
	init_pair(WHITE	,  15, 16);
	init_pair(ORANGE, 214, 16);
	init_pair(GREEN	,  82, 16);
	init_pair(BLUE	, 123, 16);
	init_pair(YELLOW, 226, 16);
	init_pair(LIME	, 156, 16);
	init_pair(SKY	, 153, 16);
	init_pair(LGREEN,  22, 16);
	init_pair(BRED	,  23, 16);

	BACKGROUND = LIME;
	// >> Background Color Sets
}

char* LineInput(char* line)
{
	char* ptr = line;
	while ((*ptr = wgetch(stdscr)) != '\n') {
		if (*ptr != ERR) ++ptr;
	}
	*ptr = 0;
	return ptr;
}

void PrintList(rankptr ptr)
{
	int cnt = 0, acnt = 0;
	char buffer[50] = "";
	for (ptr = ptr -> next; ptr != NULL; ptr = ptr -> next) {
		++cnt;
		if (MODE == 1 && X <= cnt && cnt <= Y) {
			sprintf(buffer, "%d", ptr -> key);
			PrintCenter(ptr -> name, 8 + cnt - X, 0, 35);
			PrintCenter(buffer, 8 + cnt - X, 36, 11);
		} else if ( MODE == 2 && !strcmp(target, ptr -> name)) {
			found = 1;
			sprintf(buffer, "%d", ptr -> key);
			PrintCenter(ptr -> name, 5 + acnt, 0, 35);
			PrintCenter(buffer, 5 + acnt, 36, 11);
			++acnt;
		}
	}
}

void PrintCenter(char* str, int y, int x, int width)
{
	int len = strlen(str), i, j, start = (width - len) / 2;
	mvprintw(y, x + start, "%s", str);
}

void Hold(void)
{
	int tmp, i;
	if (holdBlock < 0) {
		holdBlock = nextBlock[0];
		for (i = 0; i < BLOCK_NUM - 1; i++) {
			nextBlock[i] = nextBlock[i + 1];
		}
		nextBlock[BLOCK_NUM - 1] = rand() % 7;
	} else {
		tmp = holdBlock;
		holdBlock = nextBlock[0];
		nextBlock[0] = tmp;
	}

	DrawHoldBlock(holdBlock);
	DrawNextBlock(nextBlock);
	blockX = WIDTH / 2 - 2;
	blockY = -1;
	PrintScore(score);
	DrawField();

}

void DrawHoldBlock(int holdBlock)
{
	int currentBlock = holdBlock, i, j, blockRotate = 1;
	if (holdBlock < 0) {
		return;
	}
	For4(i) For4(j) DrawTile(i + 7, WIDTH + 17 + j, ' ');
	For4(i) {
		For4(j) {
			If_Block {
				attron(A_REVERSE);
				attron(COLOR_PAIR(SKY));
				DrawTile(i + 7, WIDTH + 17 + j, ' ');
				attroff(COLOR_PAIR(SKY));
				attroff(A_REVERSE);
			}
		}
	}
}

int CallRecommend(void)
{
	recnode better_than_your_brain;
	// Literally True :p
	recptr head = &better_than_your_brain;
	memcpy(head -> field, field, sizeof(field));
	// Create & Initialize the root node.
	head -> lev = 0;
	return modified_recommend(head);
}

int CalcField(char field[HEIGHT][WIDTH])
{
	int i, j, k, ret = 0, area, heights[WIDTH + 2], coeff = 0, edge, t, diff;

	memset(heights, -1, sizeof(heights));

	for (j = 0; j < WIDTH; j++) {
		coeff = edge = 0;
		for (i = 0; i < HEIGHT; i++) {
			if (field[i][j]) {
				heights[j] = ~heights[j] ? i : heights[j];
				edge = 1;
				ret += coeff * coeff * HOLE_MULTIPLIER;
				coeff = 0;
			} else {
				coeff += edge;
			}
		}
		ret += coeff * coeff * edge * HOLE_MULTIPLIER;
	}

	j = 1 << 30;
	k = -1;

	for (i = 0; i < WIDTH; i++) {
		k = k < heights[i] ? heights[i] : k;
		j = j > heights[i] ? heights[i] : i;
	}

	diff = (j - k);
	t = 1;
	
	for (i = 0; i < LINE_DELTA_EXPONENT; i++) {
		t *= diff;
	}

	ret += diff * HEIGHT_MULTIPLIER;

	return ret;

}
