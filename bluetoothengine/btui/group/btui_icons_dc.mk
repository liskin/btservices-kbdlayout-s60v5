#
# Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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

ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=\epoc32\release\$(PLATFORM)\$(CFG)\z
else
ZDIR=\epoc32\data\z
endif

# ----------------------------------------------------------------------------
# TODO: Configure these
# ----------------------------------------------------------------------------

TARGETDIR=$(ZDIR)\resource\apps
ICONTARGETFILENAME=$(TARGETDIR)\btui.mif

HEADERDIR=\epoc32\include
HEADERFILENAME=$(HEADERDIR)\btui.mbg

do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

# ----------------------------------------------------------------------------
# TODO: Configure these.
#
# NOTE 1: DO NOT DEFINE MASK FILE NAMES! They are included automatically by
# MifConv if the mask detph is defined.
#
# NOTE 2: Usually, source paths should not be included in the bitmap
# definitions. MifConv searches for the icons in all icon directories in a
# predefined order, which is currently \s60\icons, \s60\bitmaps2, \s60\bitmaps.
# The directory \s60\icons is included in the search only if the feature flag
# __SCALABLE_ICONS is defined.
# ----------------------------------------------------------------------------

RESOURCE :
	mifconv $(ICONTARGETFILENAME) /h$(HEADERFILENAME) \
		/c8,8 qgn_prop_bt_devices_tab3.svg \
		/c8,8 qgn_prop_bt_set_tab3.svg \
		/c8,8 qgn_prop_set_apps_bt.svg \
		/c8,8 qgn_prop_bt_blocked_tab3.svg \
		/c8,8 qgn_prop_set_apps_bt_off.svg \
		/c8,8 qgn_prop_set_apps_bt_on_shown.svg \
		/c8,8 qgn_prop_set_apps_bt_on_hidden.svg \
		/c8,8 qgn_prop_set_apps_bt_conn_shown.svg \
		/c8,8 qgn_prop_set_apps_bt_conn_hidden.svg \
		/c8,8 qgn_prop_bt_audio_connect.svg \
		/c8,8 qgn_prop_bt_computer_connect.svg \
		/c8,8 qgn_prop_bt_car_kit_connect.svg \
		/c8,8 qgn_prop_bt_keyboard_connect.svg

FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(HEADERFILENAME)&& \
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing
  
