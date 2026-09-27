CC = gcc
CFLAGS = -Wall -Wextra 
LDFLAGS = -ljpeg -lpng

SRCDIR = .
BUILDDIR = build
BINDIR = $(BUILDDIR)/bin
LIBDIR = $(BUILDDIR)/lib
TARGET = $(BINDIR)/main

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

DIRS = $(BUILDDIR) $(BINDIR) $(LIBDIR)

POST_BUILD = 

.PHONY: build debug clean

build: dirs $(TARGET)
build: CFLAGS += -O2
build: POST_BUILD += strip -s $(TARGET)
build: $(TARGET)

debug: dirs $(TARGET)
debug: CFLAGS += -O0 -g3 -ggdb -DDEBUG
debug: $(TARGET)

dirs:
	@echo "mkdir -p $(DIRS)"
	mkdir -p $(DIRS)

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $^ -o $@ $(LDFLAGS)
	$(POST_BUILD)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR) $(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR)

-include $(OBJECTS:.o=.d)
