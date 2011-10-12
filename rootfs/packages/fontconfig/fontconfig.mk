#############################################################
#
# fontconfig
#
#############################################################
FONTCONFIG_VERSION:=2.4.2
FONTCONFIG_SOURCE:=fontconfig-2.4.2.tar.gz
FONTCONFIG_DIR:=$(BUILD_DIR)/fontconfig-2.4.2

$(DL_DIR)/$(FONTCONFIG_SOURCE):
	$(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(FONTCONFIG_SOURCE)

fontconfig-source: $(DL_DIR)/$(FONTCONFIG_SOURCE)

$(FONTCONFIG_DIR)/.unpacked: $(DL_DIR)/$(FONTCONFIG_SOURCE)
	tar -C $(BUILD_DIR) -xzf $(DL_DIR)/$(FONTCONFIG_SOURCE)
#	$(PATCH) $(FONTCONFIG_DIR) $(BASE_DIR)/packages/fontconfig/ \*.patch*
	touch $(FONTCONFIG_DIR)/.unpacked

$(FONTCONFIG_DIR)/.configured: $(FONTCONFIG_DIR)/.unpacked
	chmod 0777 $(HOST_DIR)$(EPREFIX)/bin/freetype-config 
	(cd $(FONTCONFIG_DIR); \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	BUILD_CFLAGS="-I$(FREETYPE_HOST_DIR)/include/freetype2 -I$(FREETYPE_HOST_DIR)/include" \
	ac_cv_func_mmap_fixed_mapped=yes \
	./configure \
		--target=arm-none-linux-gnueabi \
		--host=arm-none-linux-gnueabi \
		--build=$(GNU_HOST_NAME) \
		--with-arch=arm-none-linux-gnueabi \
		--prefix=/ \
		--exec-prefix=$(EPREFIX) \
		--sysconfdir=$(EPREFIX)/etc \
		--includedir=$(EPREFIX)/include \
		--localstatedir=/var \
		--datarootdir=/usr/share \
		--with-expat=$(HOST_DIR)$(EPREFIX) \
		--with-expat-includes=$(HOST_DIR)$(EPREFIX)/include \
		--with-expat-lib=$(HOST_DIR)$(EPREFIX)/lib \
		--with-freetype-config="$(HOST_DIR)$(EPREFIX)/bin/freetype-config" \
		--disable-docs \
	);
	touch $(FONTCONFIG_DIR)/.configured

$(FONTCONFIG_DIR)/.compiled: $(FONTCONFIG_DIR)/.configured
	-$(MAKE) -C $(FONTCONFIG_DIR)
	touch $(FONTCONFIG_DIR)/.compiled

$(HOST_DIR)$(EPREFIX)/lib/libfontconfig.so: $(FONTCONFIG_DIR)/.compiled
	$(MAKE) DESTDIR=$(HOST_DIR) -C $(FONTCONFIG_DIR) install
	
	mv $(HOST_DIR)$(EPREFIX)/lib/libfontconfig.la $(HOST_DIR)$(EPREFIX)/lib/libfontconfig.la.old
	$(SED) "s,^libdir=.*,libdir=\'$(HOST_DIR)$(EPREFIX)/lib\',g" $(HOST_DIR)$(EPREFIX)/lib/libfontconfig.la.old > $(HOST_DIR)$(EPREFIX)/lib/libfontconfig.la

	touch -c $(HOST_DIR)$(EPREFIX)/lib/libfontconfig.so

$(TARGET_DIR)$(EPREFIX)/lib/libfontconfig.so: $(HOST_DIR)$(EPREFIX)/lib/libfontconfig.so
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libfontconfig.so* $(TARGET_DIR)$(EPREFIX)/lib/
	mkdir -p $(TARGET_DIR)$(EPREFIX)/etc/fonts
	cp $(HOST_DIR)$(EPREFIX)/etc/fonts/fonts.conf $(TARGET_DIR)$(EPREFIX)/etc/fonts/
	-$(TARGET_STRIP) --strip-unneeded $(TARGET_DIR)$(EPREFIX)/lib/libfontconfig.so
	mkdir -p $(TARGET_DIR)/var/cache/fontconfig
	mkdir -p $(TARGET_DIR)$(EPREFIX)/bin
	cp -a $(HOST_DIR)$(EPREFIX)/bin/fc-cache $(TARGET_DIR)$(EPREFIX)/bin/
	-$(TARGET_STRIP) --strip-unneeded $(TARGET_DIR)$(EPREFIX)/bin/fc-cache
	cp -a $(HOST_DIR)$(EPREFIX)/bin/fc-list $(TARGET_DIR)$(EPREFIX)/bin/
	-$(TARGET_STRIP) --strip-unneeded $(TARGET_DIR)$(EPREFIX)/bin/fc-list

fontconfig: freetype $(TARGET_DIR)$(EPREFIX)/lib/libfontconfig.so

fontconfig-clean:
	$(MAKE) DESTDIR=$(TARGET_DIR) CC=$(TARGET_CC) -C $(FONTCONFIG_DIR) uninstall
	-$(MAKE) -C $(FONTCONFIG_DIR) clean

fontconfig-dirclean:
	rm -rf $(FONTCONFIG_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_FONTCONFIG)),y)
TARGETS+=fontconfig
endif
