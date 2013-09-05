#############################################################
#
# Extra fonts
#
#############################################################
FONTS_DIR=$(EXTRAS_DIR)/fonts

$(FONTS_DIR)/.installed:
	[ -d "$(TARGET_DIR)/$(EPREFIX)/lib/fonts/" ] || mkdir $(TARGET_DIR)/$(EPREFIX)/lib/fonts/
	cp $(FONTS_DIR)/*.ttf $(TARGET_DIR)/$(EPREFIX)/lib/fonts/
	touch $(FONTS_DIR)/.installed

fonts: $(FONTS_DIR)/.installed

fonts-clean:
	-rm -rf $(FONTS_DIR)/.installed

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_FONTS)),y)
	TARGETS+=fonts
endif
