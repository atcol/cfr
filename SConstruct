FLAGS = '-g -Wall -Wextra -pedantic -Wstrict-prototypes -Werror -ggdb -std=gnu99 -D_BSD_SOURCE'
env = Environment(CCFLAGS=FLAGS)
make = env.Program(target='cfr', source=['src/class.c', 'src/print.c', 'src/main.c'])

Default(make)
