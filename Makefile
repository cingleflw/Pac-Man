CXX       := g++
CXX_FLAGS := -std=c++20

BIN     := bin
DOCS    := docs
SRC     := .
INCLUDE := .

LIBRARIES   := -lSDL3 -lSDL3_image
EXECUTABLE  := game.exe


all: $(BIN)/$(EXECUTABLE) copy-assets

copy-assets:
	@mkdir -p $(BIN)/assets
	@cp -r assets/* $(BIN)/assets/

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN):
	@mkdir -p $(BIN)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp | $(BIN)
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ -o $@ $(LIBRARIES)

clean:
	rm -rf $(BIN)

docs: $(SRC)/*.cpp
	doxygen Doxyfile

clean-docs:
	rm -rf $(DOCS)