#############################################################
#
# QtMpdClient
#
#############################################################
.PHONY: qtmpdclient-update

QTMPDCLIENT_DIR:=$(BUILD_DIR)/QtMpdClient

$(QTMPDCLIENT_DIR)/.git:
	(cd $(BUILD_DIR);git clone git://openbooxproject.git.sourceforge.net/gitroot/openbooxproject/QtMpdClient)

qtmpdclient-update: $(QTMPDCLIENT_DIR)/.git
	@if [ "`(cd $(QTMPDCLIENT_DIR);git tag -l '$(OBX_VERSION)')`" == "$(OBX_VERSION)" ] ; then \
		(cd $(QTMPDCLIENT_DIR);git checkout tags/$(OBX_VERSION)); \
	else \
		(cd $(QTMPDCLIENT_DIR);git checkout master;git pull); \
	fi;

$(QTMPDCLIENT_DIR)/.configured: qtmpdclient-update
	(cd $(QTMPDCLIENT_DIR); \
		PKG_CONFIG_PATH=$(HOST_DIR)$(EPREFIX)/lib/pkgconfig \
		QMAKESPEC=$(HOST_DIR)$(EPREFIX)/mkspecs/qws/linux-arm-g++/ \
		PATH=$(HOST_DIR)$(EPREFIX)/bin:$(PATH) \
		qmake \
	);
	touch $(QTMPDCLIENT_DIR)/.configured

$(QTMPDCLIENT_DIR)/libQtMpdClient.so.0.2.0: $(QTMPDCLIENT_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(QTMPDCLIENT_DIR)
	touch -c $(QTMPDCLIENT_DIR)/libQtMpdClient.so.0.2.0

$(HOST_DIR)$(EPREFIX)/lib/libQtMpdClient.so.0.2.0: $(QTMPDCLIENT_DIR)/libQtMpdClient.so.0.2.0
	mkdir -p $(HOST_DIR)$(EPREFIX)/include/QtMpdClient
	cp -dpf $(QTMPDCLIENT_DIR)/QMpdClient $(HOST_DIR)$(EPREFIX)/include/QtMpdClient
	cp -dpf $(QTMPDCLIENT_DIR)/*.h $(HOST_DIR)$(EPREFIX)/include/QtMpdClient
	cp -dpf $(QTMPDCLIENT_DIR)/libQtMpdClient.so* $(HOST_DIR)$(EPREFIX)/lib
	touch -c $(HOST_DIR)$(EPREFIX)/lib/libQtMpdClient.so.0.2.0

$(TARGET_DIR)$(EPREFIX)/lib/libQtMpdClient.so.0.2.0: $(HOST_DIR)$(EPREFIX)/lib/libQtMpdClient.so.0.2.0
	cp -dpf $(HOST_DIR)$(EPREFIX)/lib/libQtMpdClient.so* $(TARGET_DIR)$(EPREFIX)/lib
	-$(TARGET_STRIP) $(TARGET_DIR)$(EPREFIX)/lib/libQtMpdClient.so.0.2.0
	touch -c $(TARGET_DIR)$(EPREFIX)/lib/libQtMpdClient.so.0.2.0

qtmpdclient: libmpdclient qt $(TARGET_DIR)$(EPREFIX)/lib/libQtMpdClient.so.0.2.0

qtmpdclient-clean:
	-$(MAKE) -C $(QTMPDCLIENT_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/lib/libQtMpdClient.so.0.2.0

qtmpdclient-dirclean:
	rm -rf $(QTMPDCLIENT_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_QTMPDCLIENT)),y)
TARGETS+=qtmpdclient
endif
