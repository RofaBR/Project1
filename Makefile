CDIR = client
SDIR = server
LIBMX_DIR = libs/libmx
LIBMX = $(LIBMX_DIR)/libmx.a

all: $(LIBMX) build

$(LIBMX):
	$(MAKE) -C $(LIBMX_DIR)

build:
	$(MAKE) -C $(CDIR)
	$(MAKE) -C $(SDIR)

clean:
	$(MAKE) -C $(LIBMX_DIR) clean
	$(MAKE) -C $(CDIR) clean
	$(MAKE) -C $(SDIR) clean

re: clean all

.PHONY: all build clean re
