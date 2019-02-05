#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "filemap.h"
#include "ll.h"
#include "loggerparser.h"
#include "logline.h"

int main(void)
{
	LoggerParser parser("logger.bin");
	if (!parser.Init()) {
        printf("error opening logger.bin\n");
		return -1;
	}
	parser.Parse();
	parser.Serialize();
	return 0;
}
