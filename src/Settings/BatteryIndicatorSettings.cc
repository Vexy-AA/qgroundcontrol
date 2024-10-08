/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "BatteryIndicatorSettings.h"

#include <QtQml/QQmlEngine>

DECLARE_SETTINGGROUP(BatteryIndicator, "BatteryIndicator")
{
    qmlRegisterUncreatableType<BatteryIndicatorSettings>("QGroundControl.SettingsManager", 1, 0, "BatteryIndicatorSettings", "Reference only");
}

DECLARE_SETTINGSFACT(BatteryIndicatorSettings, display)
