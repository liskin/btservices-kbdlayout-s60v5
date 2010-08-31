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
# Description:  Global confirmation query notification plugin
#

TEMPLATE = lib
TARGET = btdevicedialogplugin
CONFIG += hb plugin
INCLUDEPATH += . ../inc ../../btui/inc
DEPENDPATH += .
DESTDIR = $${HB_BUILD_DIR}/plugins/devicedialogs

MOC_DIR = moc
OBJECTS_DIR = obj

# dependencies
HEADERS += inc/btdevicedialoginputwidget.h \
    inc/btdevicedialogquerywidget.h \
    inc/btdevicedialognotifwidget.h \
    inc/btdevicedialogpluginerrors.h \
    inc/btdevicedialogplugin.h \
    inc/btdevicesearchdialogwidget.h \
    inc/btmoredevicesdialogwidget.h \
    inc/btsenddialogwidget.h \
    inc/btdevicedialogplugintrace.h \
    inc/btdevicedialogrecvquerywidget.h \
    inc/btrecvprgrsdialogwidget.h \
    inc/btrecvcompleteddialogwidget.h

    
SOURCES += src/btdevicedialogplugin.cpp \
    src/btdevicedialoginputwidget.cpp \
    src/btdevicedialogquerywidget.cpp \
    src/btdevicedialognotifwidget.cpp \
    src/btdevicesearchdialogwidget.cpp \
    src/btmoredevicesdialogwidget.cpp \
    src/btsenddialogwidget.cpp \
    src/btdevicedialogrecvquerywidget.cpp \
    src/btrecvprgrsdialogwidget.cpp \
    src/btrecvcompleteddialogwidget.cpp

RESOURCES += btdevicedialogplugin.qrc
    
symbian: { 
    SYMBIAN_PLATFORMS = WINSCW ARMV5
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    TARGET.UID3 = 0x2002E6DF
    
    LIBS += -lbtdevice \
    		-lhbcore \
	       	-lxqservice \
    		-lxqserviceutil \
    
    hblib.sources = Hb.dll
    hblib.path = \sys\bin
    hblib.depends = "(0xEEF9EA38), 1, 0, 0, {\"Hb\"}"
    pluginstub.sources = $${TARGET}.dll
    pluginstub.path = /resource/plugins/devicedialogs
    DEPLOYMENT += pluginstub
}
!local { 
    target.path = $${HB_PLUGINS_DIR}/devicedialogs
    INSTALLS += target
}

BLD_INF_RULES.prj_exports += \
  "$${LITERAL_HASH}include <platform_paths.hrh>" \
  	"rom/btdevicedialogplugin.iby CORE_MW_LAYER_IBY_EXPORT_PATH(btdevicedialogplugin.iby)" \
    "qmakepluginstubs/$${TARGET}.qtplugin /epoc32/data/z/pluginstub/$${TARGET}.qtplugin"
