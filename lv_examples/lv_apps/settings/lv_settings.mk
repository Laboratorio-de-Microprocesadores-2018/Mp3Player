CSRCS += lv_settings.c

DEPPATH += --dep-path $(LVGL_DIR)/lv_apps/settings
VPATH += :$(LVGL_DIR)/lv_apps/settings

CFLAGS += "-I$(LVGL_DIR)/lv_apps/settings"
