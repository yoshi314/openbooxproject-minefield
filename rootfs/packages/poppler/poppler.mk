#############################################################
#
# poppler
#
#############################################################
POPPLER_SOURCE:=poppler-0.16.7.tar.gz
POPPLER_DIR:=$(BUILD_DIR)/poppler-0.16.7

$(DL_DIR)/$(POPPLER_SOURCE):
	 $(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(POPPLER_SOURCE)

poppler-source: $(DL_DIR)/$(POPPLER_SOURCE)

$(POPPLER_DIR)/.unpacked: $(DL_DIR)/$(POPPLER_SOURCE)
	tar -C $(BUILD_DIR) -xzf $(DL_DIR)/$(POPPLER_SOURCE)
	touch $(POPPLER_DIR)/.unpacked

$(POPPLER_DIR)/.configured: $(POPPLER_DIR)/.unpacked
	(cd $(POPPLER_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		PKG_CONFIG_PATH=$(EPREFIX)/lib/pkgconfig \
		QMAKESPEC=$(HOST_DIR)$(EPREFIX)/mkspecs/qws/linux-arm-g++/ \
		PATH=$(HOST_DIR)$(EPREFIX)/bin:$(PATH) \
		./configure \
		--target=arm-none-linux-gnueabi \
		--host=arm-none-linux-gnueabi \
		--build=$(GNU_HOST_NAME) \
		--prefix=/ \
		--exec-prefix=$(EPREFIX) \
		--includedir=$(EPREFIX)/include \
		--localstatedir=/var \
		--datarootdir=/usr/share \
		--enable-fixedpoint \
		--disable-libpng \
		--disable-splash-output \
		--disable-poppler-cpp \
		--disable-utils \
		--disable-cms \
	);
	touch $(POPPLER_DIR)/.configured

$(POPPLER_DIR)/qt4/src/.libs/libpoppler-qt4.so: $(POPPLER_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(POPPLER_DIR)
	touch -c $(POPPLER_DIR)/qt4/src/.libs/libpoppler-qt4.so

$(HOST_DIR)$(EPREFIX)/lib/libpoppler-qt4.so: $(POPPLER_DIR)/qt4/src/.libs/libpoppler-qt4.so
	$(MAKE) DESTDIR=$(HOST_DIR) -C $(POPPLER_DIR) install

	mv $(HOST_DIR)$(EPREFIX)/lib/libpoppler-qt4.la $(HOST_DIR)$(EPREFIX)/lib/libpoppler-qt4.la.old
	$(SED) "s,^libdir=.*,libdir=\'$(HOST_DIR)$(EPREFIX)/lib\',g" $(HOST_DIR)$(EPREFIX)/lib/libpoppler-qt4.la.old > $(HOST_DIR)$(EPREFIX)/lib/libpoppler-qt4.la

	touch -c $(HOST_DIR)$(EPREFIX)/lib/libpoppler-qt4.so

$(TARGET_DIR)$(EPREFIX)/lib/libpoppler-qt4.so: $(HOST_DIR)$(EPREFIX)/lib/libpoppler-qt4.so
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libpoppler.so* $(TARGET_DIR)$(EPREFIX)/lib
	-$(TARGET_STRIP) $(TARGET_DIR)$(EPREFIX)/lib/libpoppler.so*
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libpoppler-qt4.so* $(TARGET_DIR)$(EPREFIX)/lib
	-$(TARGET_STRIP) $(TARGET_DIR)$(EPREFIX)/lib/libpoppler-qt4.so*
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libpoppler-qt4.so

poppler: fontconfig $(TARGET_DIR)$(EPREFIX)/lib/libpoppler-qt4.so

poppler-clean:
	$(MAKE) DESTDIR=$(HOST_DIR) CC=$(TARGET_CC) -C $(POPPLER_DIR) uninstall
	-$(MAKE) -C $(POPPLER_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/lib/libpoppler-qt4.so*

poppler-dirclean:
	rm -rf $(POPPLER_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_POPPLER)),y)
TARGETS+=poppler
endif
