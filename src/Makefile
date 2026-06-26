CC = gcc
CFLAGS = -Wall -Wextra 
LDFLAGS =

SRCDIR = .
BUILDDIR = build
BINDIR = $(BUILDDIR)/bin
LIBDIR = $(BUILDDIR)/lib
TARGET = $(BINDIR)/main

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

DIRS = $(BUILDDIR) $(BINDIR) $(LIBDIR)

.PHONY: build debug clean

build: dirs $(TARGET)
build: CFLAGS += -O2
build: $(TARGET)

debug: dirs $(TARGET)
debug: CFLAGS += -O0 -g -DDEBUG
debug: $(TARGET)

dirs:
	@echo "mkdir -p $(DIRS)"
	mkdir -p $(DIRS)
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR) $(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR)

-include $(OBJECTS:.o=.d)
