FLAGS = '-Wall -Wextra -pedantic -Wstrict-prototypes -Werror -std=c99 -D_BSD_SOURCE'
env = Environment(CCFLAGS=FLAGS)
make = env.Program(target='cfr', source=['src/class.h', 'src/class.c', 'src/main.c'])

Default(make)
