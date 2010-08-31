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
TARGET = btindicator
CONFIG += hb plugin

HEADERS += inc/btindicatorplugin.h \
    inc/btindicator.h \
    inc/btindicatorconstants.h
	
SOURCES += src/btindicatorplugin.cpp \
    src/btindicator.cpp
    
MOC_DIR = moc
OBJECTS_DIR = obj    

LIBS += -lxqservice -lcpframework
symbian { 
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    TARGET.UID3 = 0x200315E9
    pluginstub.sources = $${TARGET}.dll
    pluginstub.path = /resource/plugins/indicators
    DEPLOYMENT += pluginstub
	
BLD_INF_RULES.prj_exports += "$${LITERAL_HASH}include <platform_paths.hrh>" \
    "qmakepluginstubs/$${TARGET}.qtplugin /epoc32/data/z/pluginstub/$${TARGET}.qtplugin" \
    "rom/btindicator.iby CORE_MW_LAYER_IBY_EXPORT_PATH(btindicator.iby)" \
	"inc/btindicatorconstants.h |../inc/btindicatorconstants.h"
}

