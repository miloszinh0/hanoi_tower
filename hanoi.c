#include "primlib.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_DISKS 3
#define MAX_PEGS 7
#define PEG_WIDTH 40
#define PEG_HEIGHT 500
#define DISK_HEIGHT 20
#define MIN_DISK_WIDTH 40
#define DISK_PADDING 2
#define MAX_DISK_Y 10

#define SPEED_X 5
#define SPEED_Y 5

enum Stage{
	DISK_DOWN,
	DISK_MOVE_UP,
	DISK_UP,
	DISK_MOVE_DOWN
};

struct Disk{
	int x;
	int y;
	int width;
	int height;
	int level;
	enum color color;
};

struct Peg{
	int x;
	int y;
	int width;
	int height;
	int num;
	int stack[MAX_DISKS];
	int size;
	enum color color;
};

void drawPeg(struct Peg peg){
	gfx_filledRect(peg.x, peg.y, peg.x+peg.width-1, peg.y+peg.height-1, peg.color);
}

void drawDisk(struct Disk disk){
	gfx_filledRect(disk.x, disk.y, disk.x+disk.width-1, disk.y+disk.height-1, BLACK);
	gfx_filledRect(disk.x+DISK_PADDING, disk.y+DISK_PADDING, disk.x+disk.width-DISK_PADDING-1, disk.y+disk.height-DISK_PADDING-1, disk.color);
}

int findDisk(struct Disk disk[], int size, int level){
	for(int i = 0;i < MAX_DISKS; i++){
		if (disk[i].level == level){
			return i;
		}
	}
	return -1;
}

int removeDisk(struct Peg *peg){
	if((*peg).size>0){
		(*peg).size -= 1;
		return (*peg).stack[(*peg).size];
	}
	return 0;
}

int moveDiskUp(struct Disk *disk, int screenWidth){
	if(((*disk).y > MAX_DISK_Y)){
		(*disk).y -= SPEED_Y;
	}
	else{
		(*disk).y = MAX_DISK_Y;
		if(((*disk).x+(*disk).width/2)!= screenWidth/2){
			int speed = (SPEED_X * (screenWidth/2-((*disk).x+(*disk).width/2)))/abs(screenWidth/2-((*disk).x+(*disk).width/2));
			(*disk).x += speed;
			if(abs((*disk).x - screenWidth/2-(*disk).width/2) <= SPEED_X){
				(*disk).x = screenWidth/2-(*disk).width/2;
				
				return 1;
			}
		}
		else{
			return 1;
		}
	}
	return 0;
}

int insertDisk(struct Disk *disk, struct Peg *peg){
	if((*peg).size == 0){
		(*peg).stack[(*peg).size] = (*disk).level;
		(*peg).size += 1;
		return 1;
	}
	else{
		if((*disk).level<(*peg).stack[(*peg).size-1])
		{
			(*peg).stack[(*peg).size] = (*disk).level;
			(*peg).size += 1;
			return 1;
		}
	}
	return 0;
}

int moveDiskDown(struct Disk *disk, struct Peg *peg, int screenHeight){
	int speed;
	if((((*disk).x)+(*disk).width/2)!=((*peg).x+(*peg).width/2)){
		speed = SPEED_X * ((*peg).x+(*peg).width/2-(((*disk).x)+(*disk).width/2))/abs((*peg).x+(*peg).width/2-(((*disk).x)+(*disk).width/2));
		(*disk).x += speed;
		if (abs((*peg).x+(*peg).width/2-(((*disk).x)+(*disk).width/2))<SPEED_X){
			(*disk).x=((*peg).x+(*peg).width/2)-(*disk).width/2;
		}
	}
	else{
		if(abs(((*disk).y-(screenHeight -(*disk).height*(*peg).size)))<SPEED_Y){
			(*disk).y = (screenHeight - (*disk).height*(*peg).size);
			return 1;
		}
		else{
			(*disk).y += SPEED_Y;
		}
	}
	return 0;
}

void checkGameOver(struct Peg peg[], enum Stage stage, int *gameover){
	for(int i = 1; i < MAX_PEGS; i++){
		if (peg[i].size == MAX_DISKS && stage == 0){
			(*gameover) = 1;
		}
	}
}

void executeGameOver(int gameover){
	if(gameover){
		gfx_textout(gfx_screenWidth()/2, gfx_screenHeight()/2, "You won! Congratulations!", BLACK);
		gfx_updateScreen();
		SDL_Delay(3000);
		exit(5);
	}
}

void drawScreen(struct Peg peg[], struct Disk disk[], int gameover){
	gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1,
					   WHITE);
	for(int i = 0; i < MAX_PEGS; i++){
		drawPeg(peg[i]);
		}
	for(int i = 0; i < MAX_DISKS; i++){
		drawDisk(disk[i]);
	}
	executeGameOver(gameover);
	gfx_updateScreen();
}

void manageStages(struct Peg peg[], struct Disk disk[], int *move, enum Stage *stage, int *gameover, int moving_disk, int *id, int *disk_num){
	if((*stage) == DISK_DOWN && (*move)){
			(*id) = removeDisk(&peg[moving_disk]);
			(*disk_num) = findDisk(disk, MAX_DISKS, (*id));
			(*stage) = DISK_MOVE_UP;
		}
	else if((*stage) == DISK_MOVE_UP){
		if(moveDiskUp(&disk[(*disk_num)], gfx_screenWidth())){
			(*stage) = DISK_UP;
		}
	}
	else if((*stage) == DISK_UP && (*move)){
		if(insertDisk(&disk[(*disk_num)], &peg[moving_disk])){
			(*stage) = DISK_MOVE_DOWN;
		}
	}
	else if((*stage) == DISK_MOVE_DOWN){
		if(moveDiskDown(&disk[(*disk_num)], &peg[moving_disk], gfx_screenHeight())){
			(*stage) = DISK_DOWN;
		}
	}
	for(int i = 1; i < MAX_PEGS; i++){
		if (peg[i].size == MAX_DISKS && (*stage) == DISK_DOWN){
			(*gameover) = 1;
		} 
	}
}

void controlKeys(int *move, int *moving_disk, enum Stage stage){
	(*move) = 0;
	for(int i = 0; i < MAX_PEGS; i++){
		if(gfx_isKeyDown(SDLK_1+i)){
			if(stage == DISK_DOWN || stage == DISK_UP){
				(*moving_disk) = i;
				(*move) = 1;
			}
		}
	}
	if(gfx_isKeyDown(SDLK_SPACE)){
		exit(4);
	}
}


int main(int argc, char* argv[])
{
	if (gfx_init()) {
		exit(3);
	}

	struct Peg peg[MAX_PEGS];
	struct Disk disk[MAX_DISKS];

	const int screenWidth = gfx_screenWidth();
	const int screenHeight = gfx_screenHeight();
	const int peg_distance = (screenWidth-MAX_PEGS*PEG_WIDTH)/(MAX_PEGS);
	const int width_diff = (peg_distance - MIN_DISK_WIDTH)/MAX_DISKS;

	enum Stage stage = DISK_DOWN;
	int moving_disk;
	int id;
	int disk_num;
	int gameover = 0;
	int move = 0;

	for(int i = 0; i < MAX_PEGS; i++){
		peg[i].width = PEG_WIDTH;
		peg[i].height = PEG_HEIGHT;
		peg[i].x = peg_distance/2 + (peg_distance+peg[i].width)*i;
		peg[i].y = screenHeight-peg[i].height;
		peg[i].num = i+1;
		peg[i].size = 0;
		peg[i].color = RED;
	}
	
	for(int i = MAX_DISKS-1; i >=0; i=i-1){
		disk[i].width = MIN_DISK_WIDTH+width_diff*i;
		disk[i].height = DISK_HEIGHT;
		disk[i].x = peg[0].x+peg[0].width/2-disk[i].width/2;
		disk[i].y = screenHeight-(MAX_DISKS-i)*DISK_HEIGHT;
		disk[i].level = i;
		disk[i].color = BLUE;
		peg[0].stack[peg[0].size] = disk[i].level;
		peg[0].size += 1;
	}

	while(1) {
		controlKeys(&move, &moving_disk, stage);
		manageStages(peg, disk, &move, &stage, &gameover, moving_disk, &id, &disk_num);
		checkGameOver(peg, stage, &gameover);
		drawScreen(peg, disk, gameover);
	}
	return 0;
}
