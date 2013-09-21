#ONYX_LIB_NAME=onyx-intl-booxsdk-f5a710c
ONYX_LIB_NAME=onyx-intl-booxsdk
ONYX_LIB_SOURCE=$(ONYX_LIB_NAME)
ONYX_LIB_DIR=$(BUILD_DIR)/$(ONYX_LIB_NAME)
ONYX_LIB_REPO=https://github.com/onyx-intl/booxsdk

$(DL_DIR)/$(ONYX_LIB_SOURCE):
	mkdir -p $(DL_DIR)
	#$(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(ONYX_LIB_SOURCE)
	[ ! -d "$(DL_DIR)/ONYX_LIB_NAME" ] && git clone $(ONYX_LIB_REPO) $(DL_DIR)/$(ONYX_LIB_NAME)
	cd $(DL_DIR)/$(ONYX_LIB_NAME) && git pull

$(ONYX_LIB_DIR)/.unpacked: $(DL_DIR)/$(ONYX_LIB_SOURCE)
	mkdir -p $(BUILD_DIR)
	rsync -avr $(DL_DIR)/$(ONYX_LIB_NAME) $(BUILD_DIR)/
	$(PATCH) $(ONYX_LIB_DIR) packages/onyx onyx-intl-booxsdk\*head.patch
	cp -dpf packages/onyx/*.png $(ONYX_LIB_DIR)/code/src/ui/images
	touch $(ONYX_LIB_DIR)/.unpacked

$(ONYX_LIB_DIR)/.configured: $(ONYX_LIB_DIR)/.unpacked
	(cd $(ONYX_LIB_DIR);\
	export QMAKESPEC=$(HOST_DIR)$(EPREFIX)/mkspecs/qws/linux-arm-g++/;\
	export PATH=$(HOST_DIR)$(EPREFIX)/bin:$(PATH);\
	export PKG_CONFIG_LIBDIR=$(EPREFIX)/lib/pkgconfig;\
	cmake \
	-DBUILD_FOR_ARM:BOOL=ON \
	-DONYX_SDK_ROOT:PATH=$(EPREFIX) \
	.\
	);
	touch $(ONYX_LIB_DIR)/.configured

$(ONYX_LIB_DIR)/libs/libonyx_ui.so: $(ONYX_LIB_DIR)/.configured
	$(MAKE) -C $(ONYX_LIB_DIR) -j3 all


$(HOST_DIR)$(EPREFIX)/lib/libonyx_ui.so: $(ONYX_LIB_DIR)/libs/libonyx_ui.so
	cp -dpR $(ONYX_LIB_DIR)/code/include/onyx $(HOST_DIR)$(EPREFIX)/include/
	cp -dpf $(ONYX_LIB_DIR)/libs/*.so $(HOST_DIR)$(EPREFIX)/lib/
	mkdir -p $(HOST_DIR)$(EPREFIX)/lib/static/
	cp -dpf $(ONYX_LIB_DIR)/libs/*.a $(HOST_DIR)$(EPREFIX)/lib/static/
	touch -c $(HOST_DIR)$(EPREFIX)/lib/libonyx_ui.so

$(TARGET_DIR)$(EPREFIX)/lib/libonyx_ui.so: $(HOST_DIR)$(EPREFIX)/lib/libonyx_ui.so
	$(TARGET_STRIP) --strip-unneeded $(ONYX_LIB_DIR)/libs/*.so
	cp -dpf $(ONYX_LIB_DIR)/libs/*.so $(TARGET_DIR)$(EPREFIX)/lib/
	$(TARGET_STRIP) --strip-unneeded $(ONYX_LIB_DIR)/bin/*
	cp -dpf $(ONYX_LIB_DIR)/bin/* $(TARGET_DIR)$(EPREFIX)/bin/
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libonyx_ui.so

onyx-lib: zlib qt onyx-cmake $(TARGET_DIR)$(EPREFIX)/lib/libonyx_ui.so

onyx-lib-source: $(DL_DIR)/$(ONYX_LIB_SOURCE)

onyx-lib-clean:
	-$(MAKE) -C $(ONYX_LIB_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/lib/libonyx_ui.so
	-@rm -f $(HOST_DIR)$(EPREFIX)/lib/libonyx_ui.so
	-@rm -f $(ONYX_LIB_DIR)/.configured
onyx-lib-dirclean:
	rm -rf $(ONYX_LIB_DIR)


#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_ONYX_LIB)),y)
TARGETS+=onyx-lib
endif
