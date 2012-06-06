SRC_DIR		= $(CURDIR)/src
CMAKELISTS	= $(SRC_DIR)/CMakeLists.txt
BUILD_DIR	= $(CURDIR)/build
MAKEFILE	= $(BUILD_DIR)/Makefile
TAGS		= tags

.PHONY: all
all: tags recursion

.PHONY: recursion
recursion: $(MAKEFILE)
	$(MAKE) -C $(BUILD_DIR)

$(MAKEFILE): $(CMAKELISTS)
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake $(SRC_DIR)

.PHONY: tags
tags:
	-ctags --c++-kinds=+p --fields=+iaS --extra=+q -R $(SRC_DIR)

.PHONY: clean
clean:
	rm -f tags gmon.out core vgcore.*
	-$(MAKE) -C $(BUILD_DIR) clean

.PHONY:
distclean: clean
	rm -rf $(BUILD_DIR)
