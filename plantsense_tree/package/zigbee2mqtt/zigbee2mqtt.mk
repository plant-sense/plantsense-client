################################################################################
#
# zigbee2mqtt
#
################################################################################

# ZIGBEE2MQTT_VERSION=
ZIGBEE2MQTT_SITE=https://github.com/Koenkk/zigbee2mqtt.git
ZIGBEE2MQTT_SITE_METHOD = git
ZIGBEE2MQTT_VERSION = 1.41.0

define ZIGBEE2MQTT_BUILD_CMDS
	npm ci --prefix $(@D) --arch=arm64
	npm run build --prefix $(@D)
	CC=$(TARGET_CC); CXX=$(TARGET_CC); node-gyp -C $(@D)/node_modules/sd-notify clean configure --arch=arm64 rebuild
endef

define ZIGBEE2MQTT_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/opt/zigbee2mqtt
	rsync --perms -rvl $(@D)/* $(TARGET_DIR)/opt/zigbee2mqtt --exclude="*/@serialport/bindings-cpp/prebuilds" > ~/rsync_log_1.log
	rsync --perms -rvl "$(@D)/node_modules/@serialport/bindings-cpp/prebuilds/linux-arm64" "$(TARGET_DIR)/opt/zigbee2mqtt/node_modules/@serialport/bindings-cpp/prebuilds/linux-arm64/" > ~/rsync_log_2.log
	# cp -r $(@D)/data $(TARGET_DIR)/opt/zigbee2mqtt/data
	# cp -r $(@D)/dist $(TARGET_DIR)/opt/zigbee2mqtt/dist
	# cp -r $(@D)/lib $(TARGET_DIR)/opt/zigbee2mqtt/lib
	# cp -r $(@D)/node_modules $(TARGET_DIR)/opt/zigbee2mqtt/node_modules
	# cp -r $(@D)/scripts $(TARGET_DIR)/opt/zigbee2mqtt/scripts
	# cp -r $(@D)/images $(TARGET_DIR)/opt/zigbee2mqtt/images
	## 
endef

$(eval $(generic-package))