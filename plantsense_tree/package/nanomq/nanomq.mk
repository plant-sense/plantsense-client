################################################################################
#
# nanomq
#
################################################################################

NANOMQ_VERSION=0.23.0
NANOMQ_SITE=https://github.com/nanomq/nanomq.git
NANOMQ_GIT_SUBMODULES=YES
NANOMQ_INSTALL_STAGING=NO
NANOMQ_SITE_METHOD=git

# define NANOMQ_INSTALL_STAGING_CMDS
# 	pwd
# 	ls -la .
# endef

## DONE -- TODO:: SNNnanomq, etc/nanomq.conf

define NANOMQ_BUILD_CMDS
	mkdir $(@D)/build
	cmake -DNNG_TESTS=OFF -DCMAKE_C_COMPILER=$(TARGET_CC) -DCMAKE_CXX_COMPILER=$(TARGET_CC) -DCMAKE_LINKER=$(TARGET_LD) -B $(@D)/build -S $(@D) -G Ninja
	ninja -C $(@D)/build
endef

define NANOMQ_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/build/nanomq/nanomq $(TARGET_DIR)/bin
endef

define NANOMQ_USERS
	nanomq -1 nanomq -1 * - - - daemon
endef

define NANOMQ_PERMISSIONS
	/bin/nanomq f 4755 nanomq nanomq - - - - -
endef

$(eval $(generic-package))
