CC                   = gcc
AS                   = gcc
LD                   = gcc
CFLAGS              += -D_GNU_SOURCE -O0          
INCLUDE              = -I.
LDFLAGS             += $(SUBDIRS:%=-L %) -lpthread -lstdc++

SOURCES              = $(shell ls *.c)
OBJECTS              = $(SOURCES:.c=.o) asm.o
OUTPUT               = gvi

BUILDDIRS            = $(SUBDIRS:%=build-%)
CLEANDIRS            = $(SUBDIRS:%=clean-%)

.SILENT:
all: $(OUTPUT)

$(OUTPUT): $(BUILDDIRS) $(OBJECTS)
	# echo "$(INDENT)[LD]" $(OBJECTS) $(LIBS) -o $(OUTPUT) 
	$(LD) $(OBJECTS) -no-pie  $(LDFLAGS) -o $(OUTPUT) 

%.o : %.c
	echo "$(INDENT)[CC] " $<
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

%.o : %.S
	# echo "$(INDENT)[AS] " $<
	$(AS) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean: $(CLEANDIRS)
	# echo "$(INDENT)[RM]" $(OBJECTS) $(OUTPUT) dummy.txt
	rm -f $(OBJECTS) $(OUTPUT) dummy.txt
