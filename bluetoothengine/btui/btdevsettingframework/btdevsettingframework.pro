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

TEMPLATE = lib
TARGET = btdevsettingframework

MOC_DIR = moc
DEFINES += BUILD_BTDEVSETTINGFRAMEWORK
    
CONFIG += qt \
    hb \
    dll

INCLUDEPATH += ../../../btservices_plat/bt_remote_device_setting_api/inc

HEADERS += \
       ../../../btservices_plat/bt_remote_device_setting_api/inc/btabstractdevsetting.h


SOURCES += src/btabstractdevsetting.cpp \
		   src/btdevsettingpluginloader.cpp

defFilePath = .

symbian: { 
    SYMBIAN_PLATFORMS = WINSCW \
        ARMV5
        
        BLD_INF_RULES.prj_exports += \
          "$${LITERAL_HASH}include<platform_paths.hrh>" \
          "rom/btdevsettingframework.iby CORE_MW_LAYER_IBY_EXPORT_PATH(btdevsettingframework.iby)"
		  
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.UID3 = 0xEEEEEEEE
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    
    INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE
}
