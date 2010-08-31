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
TARGET = obexhighway
CONFIG += hb  \
    service

MOC_DIR = moc
OBJECTS_DIR = obj

HEADERS += inc/btsendserviceinterface.h \
    inc/btsssendlisthandler.h \
    inc/btsendingservicedebug.h \
    inc/btsendserviceprovider.h 

SOURCES += src/main.cpp \
	src/btsendserviceinterface.cpp \
    src/btsssendlisthandler.cpp \
    src/btsendserviceprovider.cpp

    
INCLUDEPATH += . ../../inc

SERVICE.FILE = xml/obexhighway_conf.xml
SERVICE.OPTIONS = embeddable \
    hidden

LIBS += -lhbcore \
    -lxqservice \
    -lxqserviceutil \
	-lobexservicesendutils \
	-lbtfeatures \
	-lflogger \
	-lxqutils
    
symbian {
	TARGET.UID3 = 0x2002EA5A
	TARGET.CAPABILITY = All -TCB
	}
BLD_INF_RULES.prj_exports += \
  "$${LITERAL_HASH}include <platform_paths.hrh>" \
  "./rom/obexhighway.iby CORE_MW_LAYER_IBY_EXPORT_PATH(obexhighway.iby)"
