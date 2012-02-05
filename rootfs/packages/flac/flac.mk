FLAC_SOURCE=flac-1.2.1.tar.gz
FLAC_DIR=$(BUILD_DIR)/flac-1.2.1
FLAC_CFLAGS=-fPIC

$(DL_DIR)/$(FLAC_SOURCE):
	mkdir -p $(DL_DIR)
	$(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(FLAC_SOURCE)

$(FLAC_DIR)/.unpacked: $(DL_DIR)/$(FLAC_SOURCE)
	mkdir -p $(BUILD_DIR)
	tar -C $(BUILD_DIR) -xzf $(DL_DIR)/$(FLAC_SOURCE)
	touch $(FLAC_DIR)/.unpacked

$(FLAC_DIR)/.configured: $(FLAC_DIR)/.unpacked
	(cd $(FLAC_DIR);\
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		./configure \
		--target=arm-none-linux-gnueabi \
		--host=arm-none-linux-gnueabi \
		--build=$(GNU_HOST_NAME) \
		--prefix=/ \
		--exec-prefix=$(EPREFIX) \
		--includedir=$(EPREFIX)/include \
		--localstatedir=/var \
		--disable-cpplibs \
		--disable-ogg \
		--disable-xmms-plugin \
		--enable-shared \
		--enable-static \
	);
	touch $(FLAC_DIR)/.configured

$(FLAC_DIR)/src/libFLAC/.libs/libFLAC.so.8.2.0: $(FLAC_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(FLAC_DIR) all

$(HOST_DIR)$(EPREFIX)/lib/libFLAC.so.8.2.0: $(FLAC_DIR)/src/libFLAC/.libs/libFLAC.so.8.2.0
	$(MAKE) DESTDIR=$(HOST_DIR) -C $(FLAC_DIR) install
	touch -c $(HOST_DIR)$(EPREFIX)/lib/libFLAC.so.8.2.0

$(TARGET_DIR)$(EPREFIX)/lib/libFLAC.so.8.2.0: $(HOST_DIR)$(EPREFIX)/lib/libFLAC.so.8.2.0
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libFLAC.so* $(TARGET_DIR)$(EPREFIX)/lib/
	$(TARGET_STRIP) --strip-unneeded $(TARGET_DIR)$(EPREFIX)/lib/libFLAC.so*
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libFLAC.so.8.2.0

flac: $(TARGET_DIR)$(EPREFIX)/lib/libFLAC.so.8.2.0

flac-source: $(DL_DIR)/$(FLAC_SOURCE)

flac-clean:
	-$(MAKE) -C  $(FLAC_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/lib/libFLAC.so.8.2.0
	-@rm -f $(HOST_DIR)$(EPREFIX)/lib/libFLAC.so.8.2.0
	-@rm -f $(HOST_DIR)$(EPREFIX)/lib/libFLAC.a
	-@rm -f $(FLAC_DIR)/.configured

flac-dirclean:
	rm -rf $(FLAC_DIR)


#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_FLAC)),y)
TARGETS+=flac
endif
