ONYX_APP_NAME=onyx-intl-boox-opensource-de7830e
ONYX_APP_SOURCE=$(ONYX_APP_NAME).tar.gz
ONYX_APP_DIR=$(BUILD_DIR)/$(ONYX_APP_NAME)

$(DL_DIR)/$(ONYX_APP_SOURCE):
	mkdir -p $(DL_DIR)
	$(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(ONYX_APP_SOURCE)

$(ONYX_APP_DIR)/.unpacked: $(DL_DIR)/$(ONYX_APP_SOURCE)
	mkdir -p $(BUILD_DIR)
	tar -C $(BUILD_DIR) -zxf $(DL_DIR)/$(ONYX_APP_SOURCE)
	$(PATCH) $(ONYX_APP_DIR) packages/onyx onyx-intl-boox-opensource-\*.patch
	touch $(ONYX_APP_DIR)/.unpacked

$(ONYX_APP_DIR)/.configured: $(ONYX_APP_DIR)/.unpacked
	(cd $(ONYX_APP_DIR);\
	export QMAKESPEC=$(HOST_DIR)$(EPREFIX)/mkspecs/qws/linux-arm-g++/;\
	export PATH=$(HOST_DIR)$(EPREFIX)/bin:$(PATH);\
	cmake \
	-DBUILD_FOR_ARM:BOOL=ON \
	-DONYX_SDK_ROOT:PATH=$(EPREFIX) \
	.\
	);
	touch $(ONYX_APP_DIR)/.configured

$(ONYX_APP_DIR)/bin/onyx_reader: $(ONYX_APP_DIR)/.configured
	$(MAKE) -C $(ONYX_APP_DIR) all

$(TARGET_DIR)$(EPREFIX)/bin/onyx_reader: $(ONYX_APP_DIR)/bin/onyx_reader
	$(TARGET_STRIP) --strip-unneeded $(ONYX_APP_DIR)/bin/*
	cp -dpf $(ONYX_APP_DIR)/bin/* $(TARGET_DIR)$(EPREFIX)/bin/
	touch -c $(TARGET_DIR)$(EPREFIX)/bin/onyx_reader

onyx-app: libiconv curl freetype onyx-lib $(TARGET_DIR)$(EPREFIX)/bin/onyx_reader

onyx-app-source: $(DL_DIR)/$(ONYX_APP_SOURCE)

onyx-app-clean:
	-$(MAKE) -C  $(ONYX_APP_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/bin/onyx_reader
	-@rm -f $(HOST_DIR)$(EPREFIX)/bin/onyx_reader
	-@rm -f $(ONYX_APP_DIR)/.configured

onyx-app-dirclean:
	rm -rf $(ONYX_APP_DIR)


#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_ONYX_APP)),y)
TARGETS+=onyx-app
endif
