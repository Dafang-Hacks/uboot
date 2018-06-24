# make boot logo and charge logo rule
ifneq ($(wildcard $(TOPDIR)/board/$(BOARDDIR)/logo/$(BOARD).jpg),)
	BOOT_LOGO_JPG ?= $(TOPDIR)/board/$(BOARDDIR)/logo/$(BOARD).jpg
endif
ifneq ($(wildcard $(TOPDIR)/board/$(BOARDDIR)/logo/$(VENDOR).jpg),)
	BOOT_LOGO_JPG ?= $(TOPDIR)/board/$(BOARDDIR)/logo/$(VENDOR).jpg
endif
ifneq ($(wildcard $(TOPDIR)/board/$(BOARDDIR)/logo/*.jpg),)
	BOOT_LOGO_JPG ?= $(TOPDIR)/board/$(BOARDDIR)/logo/ingenic.jpg
endif
BOOT_LOGO_JPG ?= $(TOPDIR)/tools/logos/ingenic.jpg

ifneq ($(wildcard $(TOPDIR)/board/$(BOAwDDIR)/charge_logo/*.jpg),)
DIR_PREFIX = $(TOPDIR)/board/$(BOARDDIR)/charge_logo
else
DIR_PREFIX = $(TOPDIR)/tools/charge_logo
endif
CHARGE_LOGO_JPG = $(shell ls  $(DIR_PREFIX)/*.jpg)


BOOT_RLE_OBJ   := $(BOOT_LOGO_JPG:.jpg=.rle)
CHARGE_RLE_OBJS   := $(CHARGE_LOGO_JPG:.jpg=.rle)


$(RLE_BOOT_LOGO_H):	$(obj)bin2array $(BOOT_RLE_OBJ)
	$(obj)./bin2array --one $(BOOT_RLE_OBJ) $@

$(RLE_CHARGE_LOGO_H):	$(obj)bin2array $(CHARGE_RLE_OBJS)
	$(obj)./bin2array --mult $(CHARGE_RLE_OBJS)  $@

%.rle: %.jpg
	$(obj)./img2rle  $< $@

#$(obj)img2rle:	img2rle.c
#	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) $^ -ljpeg -lpng12 -o $@
#	$(HOSTSTRIP) $@

$(obj)bin2array: bin2array.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
