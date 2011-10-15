#############################################################
#
# quazip
#
#############################################################
QUAZIP_SOURCE:=quazip-0.4.3.tar.gz
QUAZIP_DIR:=$(BUILD_DIR)/quazip-0.4.3

$(DL_DIR)/$(QUAZIP_SOURCE):
	 $(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(QUAZIP_SOURCE)

quazip-source: $(DL_DIR)/$(QUAZIP_SOURCE)

$(QUAZIP_DIR)/.unpacked: $(DL_DIR)/$(QUAZIP_SOURCE)
	tar -C $(BUILD_DIR) -xzf $(DL_DIR)/$(QUAZIP_SOURCE)
	$(PATCH) $(QUAZIP_DIR) packages/quazip quazip-0.4.3\*.patch
	touch $(QUAZIP_DIR)/.unpacked

$(QUAZIP_DIR)/.configured: $(QUAZIP_DIR)/.unpacked
	(cd $(QUAZIP_DIR); \
		PKG_CONFIG_PATH=$(HOST_DIR)$(EPREFIX)/lib/pkgconfig \
		QMAKESPEC=$(HOST_DIR)$(EPREFIX)/mkspecs/qws/linux-arm-g++/ \
		PATH=$(HOST_DIR)$(EPREFIX)/bin:$(PATH) \
		qmake \
		PREFIX=$(HOST_DIR)$(EPREFIX) \
	);
	touch $(QUAZIP_DIR)/.configured

$(QUAZIP_DIR)/quazip/libquazip.so.1.0.0: $(QUAZIP_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(QUAZIP_DIR)
	touch -c $(QUAZIP_DIR)/quazip/libquazip.so.1.0.0

$(HOST_DIR)$(EPREFIX)/lib/libquazip.so.1.0.0: $(QUAZIP_DIR)/quazip/libquazip.so.1.0.0
	$(MAKE) -C $(QUAZIP_DIR) install
	touch -c $(HOST_DIR)$(EPREFIX)/lib/libquazip.so.1.0.0

$(TARGET_DIR)$(EPREFIX)/lib/libquazip.so.1.0.0: $(HOST_DIR)$(EPREFIX)/lib/libquazip.so.1.0.0
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libquazip.so* $(TARGET_DIR)$(EPREFIX)/lib
	-$(TARGET_STRIP) $(TARGET_DIR)$(EPREFIX)/lib/libquazip.so.1.0.0
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libquazip.so.1.0.0

quazip: qt $(TARGET_DIR)$(EPREFIX)/lib/libquazip.so.1.0.0

quazip-clean:
	$(MAKE) DESTDIR=$(HOST_DIR) CC=$(TARGET_CC) -C $(QUAZIP_DIR) uninstall
	-$(MAKE) -C $(QUAZIP_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/lib/libquazip.so.1.0.0

quazip-dirclean:
	rm -rf $(QUAZIP_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_QUAZIP)),y)
TARGETS+=quazip
endif
