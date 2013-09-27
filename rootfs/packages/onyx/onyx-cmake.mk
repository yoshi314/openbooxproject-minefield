#############################################################
#
# Onyx cmake modules
#
#############################################################
ONYX_CMAKE_NAME=onyx-intl-cmake_modules
ONYX_CMAKE_SOURCE=$(ONYX_CMAKE_NAME).fetch
ONYX_CMAKE_HOST_DIR:=$(HOST_DIR)$(EPREFIX)/$(ONYX_CMAKE_NAME)
ONYX_CMAKE_REPO=https://github.com/onyx-intl/cmake_modules.git

$(DL_DIR)/$(ONYX_CMAKE_SOURCE):
	[ ! -d "$(DL_DIR)/$(ONYX_CMAKE_NAME)" ] && git clone $(ONYX_CMAKE_REPO) $(DL_DIR)/$(ONYX_CMAKE_NAME)
	cd $(DL_DIR)/$(ONYX_CMAKE_NAME) && git pull
	touch $(DL_DIR)/$(ONYX_CMAKE_SOURCE).fetch

$(ONYX_CMAKE_HOST_DIR)/.unpacked: $(DL_DIR)/$(ONYX_CMAKE_SOURCE)
	cd $(DL_DIR)/$(ONYX_CMAKE_NAME) && git pull
	rsync -avr $(DL_DIR)/$(ONYX_CMAKE_NAME) $(HOST_DIR)$(EPREFIX)/
	(cd $(HOST_DIR)$(EPREFIX); ln -sf $(ONYX_CMAKE_NAME) cmake_modules)
	$(PATCH) $(ONYX_CMAKE_HOST_DIR) packages/onyx onyx-cmake\*.patch
	touch $(ONYX_CMAKE_HOST_DIR)/.unpacked

onyx-cmake: $(ONYX_CMAKE_HOST_DIR)/.unpacked

onyx-cmake-host-clean:
	rm -rf $(ONYX_CMAKE_HOST_DIR)

#TARGETS+=onyx-cmake
