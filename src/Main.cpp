#include <Main.h>

#define TEST false

int main(int argc, char** args)
{
#if !TEST

	WeaponTool tool;
	tool.Run();

#else

	//Here goes some test code
	
#endif

	return 0;
}