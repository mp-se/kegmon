import { config, status } from '@/modules/pinia'

/**
 * Used in menybar to show the total amount of items that require user action.
 *
 * @returns number of items that needs attention
 */
export function deviceBadge() {
  return deviceSettingBadge() + deviceHardwareBadge() + deviceCalibrationBadge() + deviceWifiBadge()
}

export function deviceSettingBadge() {
  return deviceMdnsBadge()
}

export function deviceMdnsBadge() {
  return config.mdns === '' ? 1 : 0
}

export function deviceHardwareBadge() {
  return 0
}

export function deviceCalibrationBadge() {
  return status.scale_factor1 == 0 && status.scale_factor2 == 0 ? 1 : 0
}

export function deviceWifiBadge() {
  return deviceWifi1Badge() | deviceWifi2Badge() ? 1 : 0
}

export function deviceWifi1Badge() {
  if (config.wifi_ssid === '') return 1
  return 0
}

export function deviceWifi2Badge() {
  if (config.wifi_ssid2 === '' && config.wifi_ssid === '') return 1
  return 0
}
