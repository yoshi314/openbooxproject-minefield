#############################################################
#
# OpenBOOX Explorer
#
#############################################################
.PHONY: obx_explorer-update

OBX_EXPLORER_DIR:=$(BUILD_DIR)/obx_explorer

$(OBX_EXPLORER_DIR)/.git:
#	(cd $(BUILD_DIR);git clone git://openbooxproject.git.sourceforge.net/gitroot/openbooxproject/obx_explorer)
	(cd $(BUILD_DIR);git clone file:///home/yoshi/src/onyx/obx_explorer -b test01)

obx_explorer-update: $(OBX_EXPLORER_DIR)/.git
	@if [ "`(cd $(OBX_EXPLORER_DIR);git tag -l '$(OBX_VERSION)')`" == "$(OBX_VERSION)" ] ; then \
		(cd $(OBX_EXPLORER_DIR);git checkout tags/$(OBX_VERSION)); \
	else \
		(cd $(OBX_EXPLORER_DIR);git checkout master;git pull); \
	fi;

$(OBX_EXPLORER_DIR)/.configured: obx_explorer-update
	(cd $(OBX_EXPLORER_DIR); \
		PKG_CONFIG_PATH=$(HOST_DIR)$(EPREFIX)/lib/pkgconfig \
		QMAKESPEC=$(HOST_DIR)$(EPREFIX)/mkspecs/qws/linux-arm-g++/ \
		PATH=$(HOST_DIR)$(EPREFIX)/bin:$(PATH) \
		qmake \
	);
	touch $(OBX_EXPLORER_DIR)/.configured

$(OBX_EXPLORER_DIR)/obx_explorer: $(OBX_EXPLORER_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(OBX_EXPLORER_DIR)
	touch -c $(OBX_EXPLORER_DIR)/obx_explorer

$(TARGET_DIR)$(EPREFIX)/bin/obx_explorer: $(OBX_EXPLORER_DIR)/obx_explorer
	cp -dpf $(OBX_EXPLORER_DIR)/obx_explorer $(TARGET_DIR)$(EPREFIX)/bin
	-$(TARGET_STRIP) $(TARGET_DIR)$(EPREFIX)/bin/obx_explorer
	(cd $(TARGET_DIR)$(EPREFIX)/bin;ln -s obx_explorer explorer)
	touch -c $(TARGET_DIR)$(EPREFIX)/bin/obx_explorer

obx_explorer: onyx-lib qtmpdclient $(TARGET_DIR)$(EPREFIX)/bin/obx_explorer

obx_explorer-clean:
	-$(MAKE) -C $(OBX_EXPLORER_DIR) clean
	-@rm -f $(TARGET_DIR)$(EPREFIX)/bin/obx_explorer

obx_explorer-dirclean:
	rm -rf $(OBX_EXPLORER_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(PACKAGE_OBX_EXPLORER)),y)
TARGETS+=obx_explorer
endif
