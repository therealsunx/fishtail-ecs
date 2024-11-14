# The Real ECS a.k.a trecs

TR-ECS is an Entity Component System library written in C/C++. It is a header only library, so simply copy the header into your project to use it.

It is mostly inline-d to make your program as fast as possible.

It is suitable for making games and stuff. It prevents you from the pitfalls of OOP and inheritance while creating different objects for your games.

Learn more about ECS in [wikipedia](https://en.wikipedia.org/wiki/Entity_component_system).

## Usage & Development

If you want to use it (after it is complete, of course), then simply copy the `include/trecs.h` header file inside your C++ project and you're ready to roll.

If want to continue developing it from its current state, then clone the repo, and go on using the ECS-registry. Everything that you need should be inside the `include/trecs.h` header file.

`Makefile` is attached with the project, so to build the project, run `make` command and run the executable inside  the `build` directory by `./build/trecs`

Yes, I am building the project in linux but after a simple modification like changing the `/` in `Makefile` to `\` will be enough to set the project in Windows. ;)
