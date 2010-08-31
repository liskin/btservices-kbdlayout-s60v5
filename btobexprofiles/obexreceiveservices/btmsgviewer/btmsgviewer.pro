#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

TEMPLATE = app
TARGET = btmsgviewer
CONFIG += hb \
    service
symbian:
 { 
    TARGET.UID3 = 0x2002ED70
    TARGET.CAPABILITY = ALL \
        -TCB
}
LIBS += -lhbcore \
    -lxqservice \
    -lxqserviceutil \
    -lbluetooth \
    -lmsgs \
    -lapmime \
    -lefsrv \
    -lapgrfx \
    -lxqutils
	
SERVICE.FILE = service_conf.xml
libFiles.sources = xqservice.dll
SERVICE.OPTIONS = embeddable \
    hidden
libFiles.path = "!:\sys\bin"
DEPLOYMENT += libFiles
HEADERS += ./inc/btmsgviewer.h \
		   ./inc/btmsgviewerutils.h 
SOURCES += ./src/btmsgviewer.cpp \
    	   ./src/main.cpp \
    	   ./src/btmsgviewerutils.cpp
BLD_INF_RULES.prj_exports += "$${LITERAL_HASH}include <platform_paths.hrh>" \
    "./rom/btmsgviewer.iby CORE_APP_LAYER_IBY_EXPORT_PATH(btmsgviewer.iby)"
