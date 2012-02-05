LIBID3TAG_SOURCE=libid3tag-0.15.1b.tar.gz
LIBID3TAG_DIR=$(BUILD_DIR)/libid3tag-0.15.1b
LIBID3TAG_CFLAGS:=-fPIC 

$(DL_DIR)/$(LIBID3TAG_SOURCE):
	$(WGET) -P $(DL_DIR) $(DOWNLOAD_SITE)/$(LIBID3TAG_SOURCE)

$(LIBID3TAG_DIR)/.unpacked: $(DL_DIR)/$(LIBID3TAG_SOURCE)
	tar -C $(BUILD_DIR) -zxf $(DL_DIR)/$(LIBID3TAG_SOURCE)
	touch $(LIBID3TAG_DIR)/.unpacked

$(LIBID3TAG_DIR)/.configured: $(LIBID3TAG_DIR)/.unpacked
	(cd $(LIBID3TAG_DIR); \
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
		--disable-debugging \
	);
	touch $(LIBID3TAG_DIR)/.configured

$(LIBID3TAG_DIR)/.libs/libid3tag.so.0.3.0: $(LIBID3TAG_DIR)/.configured
	$(MAKE) LDSHARED="$(TARGET_CROSS)gcc -Wl,-soname -Wl,libid3tag.so.0 -shared -shared-libgcc" -C $(LIBID3TAG_DIR) all

$(HOST_DIR)$(EPREFIX)/lib/libid3tag.so.0.3.0: $(LIBID3TAG_DIR)/.libs/libid3tag.so.0.3.0
	$(MAKE) DESTDIR=$(HOST_DIR) -C $(LIBID3TAG_DIR) install
	touch -c $(HOST_DIR)$(EPREFIX)/lib/libid3tag.so.0.3.0
	
$(TARGET_DIR)$(EPREFIX)/lib/libid3tag.so.0.3.0: $(HOST_DIR)$(EPREFIX)/lib/libid3tag.so.0.3.0
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libid3tag.so* $(TARGET_DIR)$(EPREFIX)/lib/
	-$(TARGET_STRIP) --strip-unneeded $(TARGET_DIR)$(EPREFIX)/lib/libid3tag.so*
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libid3tag.so.0.3.0

libid3tag: zlib $(TARGET_DIR)$(EPREFIX)/lib/libid3tag.so.0.3.0

libid3tag-source: $(DL_DIR)/$(LIBID3TAG_SOURCE)

libid3tag-clean:
	-$(MAKE) -C $(LIBID3TAG_DIR) clean
	-@rm -rf $(TARGET_DIR)$(EPREFIX)/lib/libid3tag*
	-@rm -rf $(HOST_DIR)$(EPREFIX)/lib/libid3tag*
	-@rm -rf $(LIBID3TAG_DIR)/.configured

libid3tag-dirclean:
	rm -rf $(LIBID3TAG_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_LIBID3TAG)),y)
TARGETS+=libid3tag
endif
