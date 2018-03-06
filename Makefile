
all: findstrings

C_SOURCES = $(wildcard src/[^~]*.c)
C_OBJECTS=$(patsubst src/%.c,build/%.o,$(C_SOURCES))
C_FLAGS=-O2 -Wall -ffunction-sections -fdata-sections

build/%.o : src/%.c
	@echo Compile $<
	@mkdir -p build
	@type staticanalyzer > /dev/null 2>&1 && gcc $(C_FLAGS) $< -E -o $(patsubst src/%.c,build/%.i,$<) && staticanalyzer add $(patsubst src/%.c,build/%.i,$<) $< ; exit 0
	@gcc $(C_FLAGS) -c $< -o $(patsubst src/%.c,build/%.o,$<)

findstrings: $(C_OBJECTS)
	@echo Build application $@ from object code
	@gcc $(C_FLAGS) $(C_OBJECTS) -Wl,--gc-sections -o $@

clean:
	@rm -rf build
	@rm -f findstrings
	@echo Clean complete
