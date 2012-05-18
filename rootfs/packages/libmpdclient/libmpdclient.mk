LIBMPDCLIENT_SOURCE=libmpdclient-2.7.tar.bz2
LIBMPDCLIENT_DIR=$(BUILD_DIR)/libmpdclient-2.7

$(DL_DIR)/$(LIBMPDCLIENT_SOURCE):
	mkdir -p $(DL_DIR)
	$(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(LIBMPDCLIENT_SOURCE)

$(LIBMPDCLIENT_DIR)/.unpacked: $(DL_DIR)/$(LIBMPDCLIENT_SOURCE)
	mkdir -p $(BUILD_DIR)
	tar -C $(BUILD_DIR) -xjf $(DL_DIR)/$(LIBMPDCLIENT_SOURCE)
	touch $(LIBMPDCLIENT_DIR)/.unpacked

$(LIBMPDCLIENT_DIR)/.configured: $(LIBMPDCLIENT_DIR)/.unpacked
	(cd $(LIBMPDCLIENT_DIR);\
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -std=gnu99" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		./configure \
		--target=arm-none-linux-gnueabi \
		--host=arm-none-linux-gnueabi \
		--build=$(GNU_HOST_NAME) \
		--prefix=/ \
		--exec-prefix=$(EPREFIX) \
		--includedir=$(EPREFIX)/include \
		--localstatedir=/var \
		--enable-shared \
		--enable-static \
	);
	touch $(LIBMPDCLIENT_DIR)/.configured

$(LIBMPDCLIENT_DIR)/src/.libs/libmpdclient.so.2.0.5: $(LIBMPDCLIENT_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(LIBMPDCLIENT_DIR) all

$(HOST_DIR)$(EPREFIX)/lib/libmpdclient.so.2.0.5: $(LIBMPDCLIENT_DIR)/src/.libs/libmpdclient.so.2.0.5
	$(MAKE) DESTDIR=$(HOST_DIR) -C $(LIBMPDCLIENT_DIR) install
	touch -c $(HOST_DIR)$(EPREFIX)/lib/libmpdclient.so.2.0.5

$(TARGET_DIR)$(EPREFIX)/lib/libmpdclient.so.2.0.5: $(HOST_DIR)$(EPREFIX)/lib/libmpdclient.so.2.0.5
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libmpdclient.so* $(TARGET_DIR)$(EPREFIX)/lib/
	$(TARGET_STRIP) --strip-unneeded $(TARGET_DIR)$(EPREFIX)/lib/libmpdclient.so*
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libmpdclient.so.2.0.5

libmpdclient: $(TARGET_DIR)$(EPREFIX)/lib/libmpdclient.so.2.0.5

libmpdclient-source: $(DL_DIR)/$(LIBMPDCLIENT_SOURCE)

libmpdclient-clean:
	-$(MAKE) -C  $(LIBMPDCLIENT_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/lib/libmpdclient.so*
	-@rm -f $(HOST_DIR)$(EPREFIX)/lib/libmpdclient.so*
	-@rm -f $(HOST_DIR)$(EPREFIX)/lib/libmpdclient.a
	-@rm -f $(LIBMPDCLIENT_DIR)/.configured

libmpdclient-dirclean:
	rm -rf $(LIBMPDCLIENT_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_LIBMPDCLIENT)),y)
TARGETS+=libmpdclient
endif
