#!/bin/bash

### Strip the .logger section out of the executable, and convert its data to a raw binary format for post-processing
objcopy ./a.out --only-section=.logger ./logger.bin
objcopy ./logger.bin --only-section=.logger -O binary --set-section-flags .logger=alloc --change-section-address .logger=0
