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


TEMPLATE = subdirs

# Directories
SUBDIRS += btuimodel \
btdevsettingframework \
btuidelegate \
btcpplugin \
tsrc

CONFIG += ordered

symbian: {
	SYMBIAN_PLATFORMS = WINSCW \
	        ARMV5
    BLD_INF_RULES.PRJ_EXPORTS += \
				"$${LITERAL_HASH}include<platform_paths.hrh>" \
				"rom/btui.iby CORE_MW_LAYER_IBY_EXPORT_PATH(btui.iby)" \
				"rom/btui_resources.iby LANGUAGE_APP_LAYER_IBY_EXPORT_PATH(btui_resources.iby)" \
				"inc/btuiiconutil.h       MW_LAYER_PLATFORM_EXPORT_PATH(btservices/btuiiconutil.h)" \
				"inc/btuidevtypemap.h       MW_LAYER_PLATFORM_EXPORT_PATH(btservices/btuidevtypemap.h)"
} 
