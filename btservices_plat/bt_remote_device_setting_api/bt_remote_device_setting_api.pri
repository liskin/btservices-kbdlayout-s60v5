#
# Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
# 
# Description:
#
#

symbian*: {
    # Build.inf rules
            BLD_INF_RULES.prj_exports += \
          "$${LITERAL_HASH}include<platform_paths.hrh>" \
          "bt_remote_device_setting_api/inc/btdevsettingglobal.h MW_LAYER_PLATFORM_EXPORT_PATH(btdevsettingglobal.h)" \
          "bt_remote_device_setting_api/inc/btdevsettinginterface.h MW_LAYER_PLATFORM_EXPORT_PATH(btdevsettinginterface.h)" \
          "bt_remote_device_setting_api/inc/btabstractdevsetting.h MW_LAYER_PLATFORM_EXPORT_PATH(btabstractdevsetting.h)" \
          "bt_remote_device_setting_api/inc/btdevsettingpluginloader.h MW_LAYER_PLATFORM_EXPORT_PATH(btdevsettingpluginloader.h)"
}
