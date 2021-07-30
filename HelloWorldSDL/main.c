#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "log.h"

int main(int argc, char **argv)
{
	LOG_WARNING ("Warning");
	LOG_DEBUG ("Debug");
	LOG_INFO ("Info");
	LOG_ERROR ("Error");
	return EXIT_SUCCESS;
}