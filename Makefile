# GNU Make workspace makefile autogenerated by Premake

ifndef config
  config=debug
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),debug)
  binimg_config = debug
endif
ifeq ($(config),release)
  binimg_config = release
endif

PROJECTS := binimg

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

binimg:
ifneq (,$(binimg_config))
	@echo "==== Building binimg ($(binimg_config)) ===="
	@${MAKE} --no-print-directory -C . -f binimg.make config=$(binimg_config)
endif

clean:
	@${MAKE} --no-print-directory -C . -f binimg.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  debug"
	@echo "  release"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   binimg"
	@echo ""
	@echo "For more information, see https://github.com/premake/premake-core/wiki"