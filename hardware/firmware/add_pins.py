import glob

for f in glob.glob('lib/platform/esp32/drivers/*.cpp'):
    with open(f, 'r') as file:
        content = file.read()
    if '#include "pins.h"' not in content:
        content = content.replace('#include "logger.h"', '#include "logger.h"\n#include "pins.h"')
        with open(f, 'w') as file:
            file.write(content)
