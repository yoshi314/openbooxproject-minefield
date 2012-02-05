#############################################################
#
# ALSA library
#
#############################################################
ALSA_LIB_SOURCE:=alsa-lib-1.0.16.tar.bz2
ALSA_LIB_DIR:=$(BUILD_DIR)/alsa-lib-1.0.16

$(DL_DIR)/$(ALSA_LIB_SOURCE):
	 $(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(ALSA_LIB_SOURCE)

$(ALSA_LIB_DIR)/.unpacked: $(DL_DIR)/$(ALSA_LIB_SOURCE)
	tar -C $(BUILD_DIR) -xjf $(DL_DIR)/$(ALSA_LIB_SOURCE)
	touch $(ALSA_LIB_DIR)/.unpacked

$(ALSA_LIB_DIR)/.configured: $(ALSA_LIB_DIR)/.unpacked
	(cd $(ALSA_LIB_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) -lm" \
		./configure \
		--target=arm-none-linux-gnueabi \
		--host=arm-none-linux-gnueabi \
		--build=$(GNU_HOST_NAME) \
		--prefix=/ \
		--exec-prefix=$(EPREFIX) \
		--includedir=$(EPREFIX)/include \
		--localstatedir=/var \
		--datarootdir=/usr/share \
		--enable-shared=yes \
		--enable-static=yes \
		--with-softfloat \
		--with-versioned=no \
		--disable-python \
	);
	touch $(ALSA_LIB_DIR)/.configured

$(ALSA_LIB_DIR)/src/.libs/libasound.so: $(ALSA_LIB_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(ALSA_LIB_DIR)
	touch -c $(ALSA_LIB_DIR)/src/.libs/libasound.so

$(HOST_DIR)$(EPREFIX)/lib/libasound.so: $(ALSA_LIB_DIR)/src/.libs/libasound.so
	$(MAKE) DESTDIR=$(HOST_DIR) -C $(ALSA_LIB_DIR) install
	touch -c $(HOST_DIR)$(EPREFIX)/lib/libasound.so

$(TARGET_DIR)$(EPREFIX)/lib/libasound.so: $(HOST_DIR)$(EPREFIX)/lib/libasound.so
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libasound.so* $(TARGET_DIR)$(EPREFIX)/lib
	-$(TARGET_STRIP) $(TARGET_DIR)$(EPREFIX)/lib/libasound.so*
	cp -dpR $(HOST_DIR)/usr/share/alsa $(TARGET_DIR)/usr/share
	cp -dpf $(BASE_DIR)/packages/alsa/asound.conf $(TARGET_DIR)/etc
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libasound.so

alsa_lib: $(TARGET_DIR)$(EPREFIX)/lib/libasound.so

alsa_lib-source: $(DL_DIR)/$(ALSA_LIB_SOURCE)

alsa_lib-clean:
	$(MAKE) DESTDIR=$(HOST_DIR) CC=$(TARGET_CC) -C $(ALSA_LIB_DIR) uninstall
	-$(MAKE) -C $(ALSA_LIB_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/lib/libasound.so*

alsa_lib-dirclean:
	rm -rf $(ALSA_LIB_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_ALSA_LIB)),y)
TARGETS+=alsa_lib
endif
