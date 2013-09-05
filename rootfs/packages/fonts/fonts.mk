#############################################################
#
# Extra fonts
#
#############################################################
FONTS_DIR=$(EXTRAS_DIR)/fonts

$(FONTS_DIR)/.installed:
	[ -d "$(TARGET_DIR)/opt/onyx/arm/lib/fonts/" ] || mkdir $(TARGET_DIR)/opt/onyx/arm/lib/fonts/
	cp $(FONTS_DIR)/*.ttf $(TARGET_DIR)/opt/onyx/arm/lib/fonts/
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
