#include "main.h"
#include "MainLogic.h"

int main(int argc, char** argv) 
{	
    MainLogic ml;

    printf("Start Hitek linux UI Application! ver %s\n", HITEK_UI_VER);
    int ret = ml.StartSystem(argc, argv);

    return ret;
}
