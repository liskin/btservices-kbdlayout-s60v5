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

import os
import sys
import string
from xml.dom.minidom import Document
import re

TRUE    = 1
FALSE   = 0

#Parameter Parsing
NONEOPTION      = 0
PATHOPTION      = 1
IMAGEPATHOPTION = 2
TARGETOPTION    = 3
PLUGINRSCOPTION = 4
HARNESSOPTION   = 5
STIFTYPEOPTION  = 6

pluginrscs = []
testspath = ""
imagepath = ""
target = ""
targethost = ""
harness = ""
stiftype = ""

index = 1
argnumber = len(sys.argv)
if((argnumber < 2) or (sys.argv[1] == "HELP") or (sys.argv[1] == "help") or (sys.argv[1] == "Help")or (sys.argv[1] == "-HELP") or (sys.argv[1] == "-help") or (sys.argv[1] == "-Help")):
    print "\n"
    print "********** DropGenerator Tool Usage Manual ********** "
    print ""
    print " -------- THIS TOOL IS USED FOR: --------"
    print "     - Building the test module "
    print "     - Create the ATS3Drop folder under tsrc folder"
    print "     - Generate test.xml file under tsrc folder"
    print "     - Generate testdrop.zip file including ATS3Drop and test.xml"
    print ""
    print " -------- PARAMETERS TO BE PASSED TO THE TOOL: -------- \n"
    print " dropgenerator.py -PATH <complete path of tests folder>"
    print "                 -IMAGEPATH <complete path of the images>"
    print "                 -TARGET <target hardware type eg: devlon>"
    print "                 [-PLUGINRSCS <list of ecom plugin resources files>]"
    print "                 -HARNESS <test harness used eg: STIF>"
    print "                 -STIFTYPE <type of STIF test module used, if Harness is STIF>"
    print "                     Supported STIF Types: Testscripter, Hardcoded"
    sys.exit()

# read paramters
option = NONEOPTION
for index in range(len(sys.argv)):
    param = sys.argv[index]
    if((param == "-PATH") or (param == "-path") or (param == "-Path")):
        option = PATHOPTION
        param = sys.argv[index]
        continue
    if((param == "-IMAGEPATH") or (param == "-imagepath") or (param == "-Imagepath")):
        option = IMAGEPATHOPTION
        param = sys.argv[index]
        continue    
    elif((param == "-TARGET") or (param == "-target") or (param == "-Target")):
        option = TARGETOPTION
        param = sys.argv[index]
        continue
    elif((param == "-PLUGINRSCS") or (param == "-pluginrscs") or (param == "-Pluginrscs")):
        option = PLUGINRSCOPTION
        param = sys.argv[index]
        continue
    elif((param == "-DATAFILES") or (param == "-datafiles") or (param == "-Datafiles")):
        option = DATAFILESOPTION
        param = sys.argv[index]
        continue
    
    elif((param == "-HARNESS") or (param == "-harness") or (param == "-Harness")):
        option = HARNESSOPTION
        param = sys.argv[index]
        continue

    elif((param == "-STIFTYPE") or (param == "-Stiftype") or (param == "-stiftype")):
        option = STIFTYPEOPTION
        param = sys.argv[index]
        continue    

    if(option == PATHOPTION):
        testspath = param

    if(option == IMAGEPATHOPTION):
        imagepath = param

    if(option == TARGETOPTION):
        target = param
        
    if(option == PLUGINRSCOPTION):
        pluginrscs.append(param)

    if(option == HARNESSOPTION):
        harness = param

    if(option == STIFTYPEOPTION):
        stiftype = param
    
#validating parameters passed
if(cmp(testspath, "") == 0):
    print "\n ERROR: tests PATH parameter not provided"
    sys.exit()

if(cmp(imagepath, "") == 0):
    print "\n ERROR: image PATH parameter not provided"
    sys.exit()

if(cmp(target, "") == 0):
    print "\n ERROR: TARGET parameter not provided"
    sys.exit()

if(cmp(harness, "") == 0):
    print "\n ERROR: HARNESS parameter not provided"
    sys.exit()
elif((cmp(harness, "STIF") == 0) or (cmp(harness, "stif") == 0) or (cmp(harness, "Stif") == 0)):
    if(cmp(stiftype, "") == 0):
        print "\n ERROR: STIFTYPE parameter not provided"
        sys.exit()

EXECUTABLE  = 0
CFGFILE     = 1
INIFILE     = 2
FLASHFILE   = 3
DATAFILE    = 4
INCLUDEFILE = 5
MMPFILE     = 6
INFFILE     = 7
PLUGINRSCFILE = 8

EXTDLL      = ".dll"
EXTEXE      = ".exe"
EXTCFG      = ".cfg"
EXTMMP      = ".mmp"

COMMONPATH  = "\\tsrc"

#get the list of files&dirs in tests folder and clean ATS3Drop folder, test.xml and testdrop package
Directory = testspath
os.system("dir "+Directory+" /B /A > files1.txt")
pre = open("files1.txt", 'r')
file = pre.readline()
while file:
   if(file[-1] == "\n"):
        file = file[:-1]
        
   if(cmp(file, "ATS3Drop") == 0):
       os.system("rmdir /S /Q " + Directory + "\\" + file)
   elif(cmp(file, "test.xml") == 0):
       os.system("del /Q " + Directory + "\\" + file)
   elif(cmp(file, "testdrop.zip") == 0):
       os.system("del /Q " + Directory + "\\" + file)
   file = pre.readline()
pre.close()
print "\nDirectory Path Parsed: "+Directory+"\n"

#list all files under tests folder
os.system("dir "+Directory+" /S/B /A-D > files.txt")
cpp = open("files.txt", 'r')

CfgFiles        = []
ExecutableFiles = []

count = 0
file = cpp.readline()

#go through each file in files.txt and pick up .cfg, and extract binary names with extension .dll and .exe from mmp files.
while file:
    if(file[-1] == "\n"):
        file = file[:-1]

    if((re.search("\.cfg$", file) != None )):
        CfgFiles.append(file)

    if((re.search("\.mmp$", file) != None )):
        #parsing mmp file for getting the executable name
        mmp = open(file, "r")
        mmpline = mmp.readline()
        while mmpline:
            if(string.find(mmpline, "TARGET ") != -1):
                mmplinetarget = string.split(mmpline)
                ExecutableFiles.append(mmplinetarget[-1])
                break
            mmpline = mmp.readline()
        mmp.close();
            
    count = count + 1
    file = cpp.readline()
cpp.close();

#validate if any .cfg exists as we are using STIP scripter.
if((cmp(harness, "STIF") == 0) or (cmp(harness, "stif") == 0) or (cmp(harness, "Stif") == 0)):
    if((cmp(stiftype, "TESTSCRIPTER") == 0) or (cmp(stiftype, "Testscripter") == 0) or (cmp(stiftype, "testscripter") == 0)):
         if(len(CfgFiles) == 0):
             print "\n CFG file not available for TestScripter type STIF module"
             sys.exit()

path = string.split(Directory, "\\")
EXECUTABLEPATH = path[0] + "\\epoc32\\RELEASE\\armv5\\UREL\\"
PLUGINRESOURCEPATH = path[0] + "\\epoc32\\data\Z\\resource\\plugins\\"

#Create test drop structure and copy files to test drop
print "\n Creating ATSDrop Directory"
ret = os.mkdir(Directory + "\\ATS3Drop")
ATS3DropDir = Directory + "\\ATS3Drop\\"
ret = os.mkdir(Directory + "\\ATS3Drop\\armv5_urel")
ATS3EXEDir = Directory + "\\ATS3Drop\\armv5_urel"
ret = os.mkdir(Directory + "\\ATS3Drop\\images")
ATS3ImageDir = Directory + "\\ATS3Drop\\images"

DropFloderCreated = TRUE

print "\n Copying files to the Drop"

#copy test binaries
for file in ExecutableFiles: 
    print("\n copy " + EXECUTABLEPATH + file +" to "+ ATS3EXEDir)
    ret = os.system("copy /Y "+ EXECUTABLEPATH + file +" "+ ATS3EXEDir)
    if(ret != 0):
        print "Error : unable to copy " + EXECUTABLEPATH + file
        sys.exit()
    
#copy STIF .cfg files
for file in CfgFiles:
    print("\n copy " + file +" to "+ ATS3DropDir)
    ret = os.system("copy /Y "+ file +" "+ ATS3DropDir)
    if(ret != 0):
        print "Error : unable to copy " + file
        sys.exit()        
#copy ecom plugin resource files
for file in pluginrscs:
    print("\n copy " + PLUGINRESOURCEPATH + file +" to "+ ATS3DropDir)
    ret = os.system("copy /Y "+ PLUGINRESOURCEPATH + file +" "+ ATS3DropDir)
    if(ret != 0):
        print "Error : unable to copy " + PLUGINRESOURCEPATH + file
        sys.exit()   

#copy images files

#list all files under the given image path
os.system("dir "+ imagepath +" /S/B /A-D > imgs.txt")
imgs = open("imgs.txt", 'r')

ImageFiles  = []
line = imgs.readline()
#get the complete path and name of the core image
while line:
    if(line[-1] == "\n"):
        line = line[:-1]

    if((re.search("\.C00$", line) != None )):
        ImageFiles.append(line)
        break;
    line = imgs.readline();
imgs.close();

#get the complete path and name of the variant image
imgs = open("imgs.txt", 'r')
line = imgs.readline()
while line:
    if(line[-1] == "\n"):
        line = line[:-1]
    if((re.search("\.V01$", line) != None )):
        ImageFiles.append(line)
        break
    line = imgs.readline();
imgs.close();

#get the complete path and name of userdisk erase image
imgs = open("imgs.txt", 'r')
line = imgs.readline()
while line:
    if(line[-1] == "\n"):
        line = line[:-1]
    if((re.search("\erase_userdisk.fpsx$", line) != None )):
        ImageFiles.append(line)
        break
    line = imgs.readline();
imgs.close();

#copy image files
for file in ImageFiles:
    print("\n copy " + file +" to "+ ATS3ImageDir)
    ret = os.system("copy /Y "+ file +" "+ ATS3ImageDir)
    if(ret != 0):
        print "Error : unable to copy " + file
        sys.exit()   

DLLTARGETPATH   = "c:\\sys\\bin\\"
PLUGINRSCTARGETPATH = "c:\\resource\\plugins\\"
CFGTARGETPATH   = "e:\\testing\\"
LOGTARGETPATH   = "e:\\temp\\lc_apitest\\"

imagestoflash = []
cfgFileNoPath = []
filestoinstall = []
BINARYFILE = 0
NONBINARYFILE = 1

for line in ImageFiles:
    imagestoflash.append(line.split("\\")[-1])
for line in CfgFiles:
    cfgFileNoPath.append(line.split("\\")[-1])
    filestoinstall.append([line.split("\\")[-1], NONBINARYFILE, CFGTARGETPATH])
for file in ExecutableFiles:
    filestoinstall.append([file, BINARYFILE, DLLTARGETPATH])
for file in pluginrscs:
    filestoinstall.append([file, NONBINARYFILE, PLUGINRSCTARGETPATH])


#Create XML if the test drop folder is created
if(DropFloderCreated == TRUE):
    # Create the minidom document
    #test tage
    xmldoc = Document()
    xmltest = xmldoc.createElement("test")
    xmldoc.appendChild(xmltest)

    #id tag
    xmlid = xmldoc.createElement("id")
    xmltest.appendChild(xmlid)
    xmlidtext = xmldoc.createTextNode("1")
    xmlid.appendChild(xmlidtext)

    xmlname = xmldoc.createElement("name")
    xmltest.appendChild(xmlname)
    xmlnametext = xmldoc.createTextNode("LC API test")
    xmlname.appendChild(xmlnametext)

    #target tag
    xmltarget = xmldoc.createElement("target")
    xmltest.appendChild(xmltarget)

    xmltergetdevice1 = xmldoc.createElement("device")
    xmltergetdevice1.setAttribute("alias", "DEFAULT")
    xmltergetdevice1.setAttribute("rank", "none")
    xmltarget.appendChild(xmltergetdevice1)
    
    xmltargetproperty1 = xmldoc.createElement("property")
    xmltargetproperty1.setAttribute("value",harness)
    xmltargetproperty1.setAttribute("name","HARNESS")
    xmltergetdevice1.appendChild(xmltargetproperty1)

    xmltargetproperty2 = xmldoc.createElement("property")
    xmltargetproperty2.setAttribute("value",target)
    xmltargetproperty2.setAttribute("name","TYPE")
    xmltergetdevice1.appendChild(xmltargetproperty2)

    #plan tag
    xmlplan1 = xmldoc.createElement("plan")
    xmlplan1.setAttribute("passrate","100")
    xmlplan1.setAttribute("enabled","true")
    xmlplan1.setAttribute("harness",harness)
    xmlplan1.setAttribute("name",string.rstrip("Test Plan"))
    xmlplan1.setAttribute("id","1.1")
    xmltest.appendChild(xmlplan1)

    #session tag
    xmlp1session1 = xmldoc.createElement("session")
    xmlp1session1.setAttribute("passrate","100")
    xmlp1session1.setAttribute("enabled","true")
    xmlp1session1.setAttribute("harness",harness)
    xmlp1session1.setAttribute("name","session")
    xmlp1session1.setAttribute("id","1.1.1")
    xmlplan1.appendChild(xmlp1session1)

    #set tag
    xmlp1s1set1 = xmldoc.createElement("set")
    xmlp1s1set1.setAttribute("passrate","100")
    xmlp1s1set1.setAttribute("enabled","true")
    xmlp1s1set1.setAttribute("harness",harness)
    xmlp1s1set1.setAttribute("name","set")
    xmlp1s1set1.setAttribute("id","1.1.1.1")
    xmlp1session1.appendChild(xmlp1s1set1)

    xmlp1s1s1target = xmldoc.createElement("target")
    xmlp1s1set1.appendChild(xmlp1s1s1target)

    xmlp1s1s1targetdevice = xmldoc.createElement("device")
    xmlp1s1s1targetdevice.setAttribute("alias","DEFAULT")
    xmlp1s1s1targetdevice.setAttribute("rank","master")
    xmlp1s1s1target.appendChild(xmlp1s1s1targetdevice)

    #case tag
    xmlp1s1s1case1 = xmldoc.createElement("case")
    xmlp1s1s1case1.setAttribute("passrate","100")
    xmlp1s1s1case1.setAttribute("enabled","true")
    xmlp1s1s1case1.setAttribute("harness",harness)
    xmlp1s1s1case1.setAttribute("name","Test Case 1")
    xmlp1s1s1case1.setAttribute("id","1.1.1.1.1")
    xmlp1s1set1.appendChild(xmlp1s1s1case1)

    #flash tags if there are images    
    for xmlimage in imagestoflash:
        xmlp1s1s1c1flash = xmldoc.createElement("flash")
        xmlp1s1s1case1.appendChild(xmlp1s1s1c1flash)
        xmlp1s1s1c1flash.setAttribute("target-alias","DEFAULT")
        xmlp1s1s1c1flash.setAttribute("images","ATS3Drop\\images\\" + xmlimage)
            
    #install steps
    xmlstepcount = 0
    for file in filestoinstall:
        xmlstepcount = xmlstepcount + 1
        #step1
        xmlp1s1s1c1step = xmldoc.createElement("step")
        xmlp1s1s1c1step.setAttribute("significant","false")
        xmlp1s1s1c1step.setAttribute("passrate","100")
        xmlp1s1s1c1step.setAttribute("enabled","true")
        xmlp1s1s1c1step.setAttribute("harness",harness)
        xmlp1s1s1c1step.setAttribute("name","Step "+str(xmlstepcount))
        xmlp1s1s1c1step.setAttribute("id","1.1.1.1.1."+str(xmlstepcount))
        xmlp1s1s1case1.appendChild(xmlp1s1s1c1step)

        xmlp1s1s1c1s1command1 = xmldoc.createElement("command")
        xmlp1s1s1c1step.appendChild(xmlp1s1s1c1s1command1)
        xmlp1s1s1c1s1command1text = xmldoc.createTextNode("install")
        xmlp1s1s1c1s1command1.appendChild(xmlp1s1s1c1s1command1text)

        xmlp1s1s1c1s1params1 = xmldoc.createElement("params")
        xmlp1s1s1c1step.appendChild(xmlp1s1s1c1s1params1)
        if(file[1] == BINARYFILE):
            #parameter1
            xmlp1s1s1c1s1p1param1 = xmldoc.createElement("param")
            xmlp1s1s1c1s1p1param1.setAttribute("type","binary")
            xmlp1s1s1c1s1params1.appendChild(xmlp1s1s1c1s1p1param1)
            #end of parameter1
        #parameter2
        xmlp1s1s1c1s1p1param1 = xmldoc.createElement("param")
        xmlp1s1s1c1s1p1param1.setAttribute("src",file[0])
        xmlp1s1s1c1s1params1.appendChild(xmlp1s1s1c1s1p1param1)
        #end of parameter2
        #parameter3
        xmlp1s1s1c1s1p1param1 = xmldoc.createElement("param")
        xmlp1s1s1c1s1p1param1.setAttribute("dst",file[2] + file[0])
        xmlp1s1s1c1s1params1.appendChild(xmlp1s1s1c1s1p1param1)
        #end of parameter3
        #parameter4
        xmlp1s1s1c1s1p1param1 = xmldoc.createElement("param")
        xmlp1s1s1c1s1p1param1.setAttribute("component-path","ATS3Drop")
        xmlp1s1s1c1s1params1.appendChild(xmlp1s1s1c1s1p1param1)
        #end of parameter4
        #end of Step1

    xmlstepcount = xmlstepcount + 1
    xmlp1s1s1c1step6 = xmldoc.createElement("step")
    xmlp1s1s1c1step6.setAttribute("significant","false")
    xmlp1s1s1c1step6.setAttribute("passrate","100")
    xmlp1s1s1c1step6.setAttribute("enabled","true")
    xmlp1s1s1c1step6.setAttribute("harness",harness)
    xmlp1s1s1c1step6.setAttribute("name","Step "+str(xmlstepcount))
    xmlp1s1s1c1step6.setAttribute("id","1.1.1.1.1."+str(xmlstepcount))
    xmlp1s1s1case1.appendChild(xmlp1s1s1c1step6)

    xmlp1s1s1c1s6command1 = xmldoc.createElement("command")
    xmlp1s1s1c1step6.appendChild(xmlp1s1s1c1s6command1)
    xmlp1s1s1c1s1command1text = xmldoc.createTextNode("makedir")
    xmlp1s1s1c1s6command1.appendChild(xmlp1s1s1c1s1command1text)

    xmlp1s1s1c1s6params1 = xmldoc.createElement("params")
    xmlp1s1s1c1step6.appendChild(xmlp1s1s1c1s6params1)
    
    xmlp1s1s1c1s6p1param1 = xmldoc.createElement("param")
    xmlp1s1s1c1s6p1param1.setAttribute("dir","e:\\temp\\lc_apitest")
    xmlp1s1s1c1s6params1.appendChild(xmlp1s1s1c1s6p1param1)  
        
    #execute test case step
    if((cmp(harness, "STIF") == 0) or (cmp(harness, "stif") == 0) or (cmp(harness, "Stif") == 0)):
        if((cmp(stiftype, "TESTSCRIPTER") == 0) or (cmp(stiftype, "Testscripter") == 0) or (cmp(stiftype, "testscripter") == 0)):
            for cfgfiles_ in cfgFileNoPath:
                xmlstepcount = xmlstepcount + 1
                xmlp1s1s1c1step = xmldoc.createElement("step")
                xmlp1s1s1c1step.setAttribute("significant","false")
                xmlp1s1s1c1step.setAttribute("passrate","100")
                xmlp1s1s1c1step.setAttribute("enabled","true")
                xmlp1s1s1c1step.setAttribute("harness",harness)
                xmlp1s1s1c1step.setAttribute("name","Step "+str(xmlstepcount))
                xmlp1s1s1c1step.setAttribute("id","1.1.1.1.1."+str(xmlstepcount))
                xmlp1s1s1case1.appendChild(xmlp1s1s1c1step)

                xmlp1s1s1c1s1command1 = xmldoc.createElement("command")
                xmlp1s1s1c1step.appendChild(xmlp1s1s1c1s1command1)
                xmlp1s1s1c1s1command1text = xmldoc.createTextNode("run-cases")
                xmlp1s1s1c1s1command1.appendChild(xmlp1s1s1c1s1command1text)

                xmlp1s1s1c1s1params1 = xmldoc.createElement("params")
                xmlp1s1s1c1step.appendChild(xmlp1s1s1c1s1params1)
                
                xmlp1s1s1c1s1p1param1 = xmldoc.createElement("param")
                xmlp1s1s1c1s1p1param1.setAttribute("module","testscripter")
                xmlp1s1s1c1s1params1.appendChild(xmlp1s1s1c1s1p1param1)

                xmlp1s1s1c1s1p1param2 = xmldoc.createElement("param")
                xmlp1s1s1c1s1p1param2.setAttribute("filter","*")
                xmlp1s1s1c1s1params1.appendChild(xmlp1s1s1c1s1p1param2)

                xmlp1s1s1c1s1p1param3 = xmldoc.createElement("param")
                xmlp1s1s1c1s1p1param3.setAttribute("timeout","3000")
                xmlp1s1s1c1s1params1.appendChild(xmlp1s1s1c1s1p1param3)

                xmlp1s1s1c1s1p1param4 = xmldoc.createElement("param")
                xmlp1s1s1c1s1p1param4.setAttribute("testcase-file", CFGTARGETPATH + cfgfiles_)
                xmlp1s1s1c1s1params1.appendChild(xmlp1s1s1c1s1p1param4)

    #fetch STIF test report
    xmlstepcount = xmlstepcount + 1
    xmlp1s1s1c1step6 = xmldoc.createElement("step")
    xmlp1s1s1c1step6.setAttribute("significant","false")
    xmlp1s1s1c1step6.setAttribute("passrate","100")
    xmlp1s1s1c1step6.setAttribute("enabled","true")
    xmlp1s1s1c1step6.setAttribute("harness",harness)
    xmlp1s1s1c1step6.setAttribute("name","Step "+str(xmlstepcount))
    xmlp1s1s1c1step6.setAttribute("id","1.1.1.1.1."+str(xmlstepcount))
    xmlp1s1s1case1.appendChild(xmlp1s1s1c1step6)

    xmlp1s1s1c1s6command1 = xmldoc.createElement("command")
    xmlp1s1s1c1step6.appendChild(xmlp1s1s1c1s6command1)
    xmlp1s1s1c1s1command1text = xmldoc.createTextNode("fetch-log")
    xmlp1s1s1c1s6command1.appendChild(xmlp1s1s1c1s1command1text)

    xmlp1s1s1c1s6params1 = xmldoc.createElement("params")
    xmlp1s1s1c1step6.appendChild(xmlp1s1s1c1s6params1)
    
    xmlp1s1s1c1s6p1param1 = xmldoc.createElement("param")
    xmlp1s1s1c1s6p1param1.setAttribute("type","text")
    xmlp1s1s1c1s6params1.appendChild(xmlp1s1s1c1s6p1param1)

    xmlp1s1s1c1s6p1param2 = xmldoc.createElement("param")
    xmlp1s1s1c1s6p1param2.setAttribute("delete","true")
    xmlp1s1s1c1s6params1.appendChild(xmlp1s1s1c1s6p1param2)

    xmlp1s1s1c1s6p1param3 = xmldoc.createElement("param")
    xmlp1s1s1c1s6p1param3.setAttribute("path","C:\\Logs\\TestFramework\\*")
    xmlp1s1s1c1s6params1.appendChild(xmlp1s1s1c1s6p1param3)     

    #fetch test module own logs 
    xmlstepcount = xmlstepcount + 1
    xmlp1s1s1c1step6 = xmldoc.createElement("step")
    xmlp1s1s1c1step6.setAttribute("significant","false")
    xmlp1s1s1c1step6.setAttribute("passrate","100")
    xmlp1s1s1c1step6.setAttribute("enabled","true")
    xmlp1s1s1c1step6.setAttribute("harness",harness)
    xmlp1s1s1c1step6.setAttribute("name","Step "+str(xmlstepcount))
    xmlp1s1s1c1step6.setAttribute("id","1.1.1.1.1."+str(xmlstepcount))
    xmlp1s1s1case1.appendChild(xmlp1s1s1c1step6)

    xmlp1s1s1c1s6command1 = xmldoc.createElement("command")
    xmlp1s1s1c1step6.appendChild(xmlp1s1s1c1s6command1)
    xmlp1s1s1c1s1command1text = xmldoc.createTextNode("fetch-log")
    xmlp1s1s1c1s6command1.appendChild(xmlp1s1s1c1s1command1text)

    xmlp1s1s1c1s6params1 = xmldoc.createElement("params")
    xmlp1s1s1c1step6.appendChild(xmlp1s1s1c1s6params1)
    
    xmlp1s1s1c1s6p1param1 = xmldoc.createElement("param")
    xmlp1s1s1c1s6p1param1.setAttribute("type","text")
    xmlp1s1s1c1s6params1.appendChild(xmlp1s1s1c1s6p1param1)

    xmlp1s1s1c1s6p1param2 = xmldoc.createElement("param")
    xmlp1s1s1c1s6p1param2.setAttribute("delete","true")
    xmlp1s1s1c1s6params1.appendChild(xmlp1s1s1c1s6p1param2)

    xmlp1s1s1c1s6p1param3 = xmldoc.createElement("param")
    xmlp1s1s1c1s6p1param3.setAttribute("path","e:\\temp\\lc_apitest\\*")
    xmlp1s1s1c1s6params1.appendChild(xmlp1s1s1c1s6p1param3)
    
    #files section
    xmlfiles = xmldoc.createElement("files")
    xmltest.appendChild(xmlfiles)
    for file in filestoinstall:
        xmlfilesfile1 = xmldoc.createElement("file")
        if(file[1] == BINARYFILE):
            xmlfile1text = xmldoc.createTextNode("ATS3Drop\\armv5_urel\\"+file[0])
            xmlfilesfile1.appendChild(xmlfile1text)
            xmlfiles.appendChild(xmlfilesfile1)
        else:
            xmlfile1text = xmldoc.createTextNode("ATS3Drop\\"+file[0])
            xmlfilesfile1.appendChild(xmlfile1text)
            xmlfiles.appendChild(xmlfilesfile1)  
    #image files in files section
    for xmlimage in imagestoflash:
        xmlflashfile = xmldoc.createElement("file")
        xmlflashfiletext = xmldoc.createTextNode("ATS3Drop\\images\\" + xmlimage)
        xmlflashfile.appendChild(xmlflashfiletext)
        xmlfiles.appendChild(xmlflashfile)

    #post action: send logs by email
    xmlpostaction = xmldoc.createElement("postAction")
    xmltest.appendChild(xmlpostaction)
    xmlactiontype = xmldoc.createElement("type")
    xmlpostaction.appendChild(xmlactiontype)
    xmlactiontext = xmldoc.createTextNode("SendEmailAction")
    xmlactiontype.appendChild(xmlactiontext)

    xmlsendemailparams = xmldoc.createElement("params")
    xmlpostaction.appendChild(xmlsendemailparams)
    
    xmlsendemailparam = xmldoc.createElement("param")
    xmlsendemailparam.setAttribute("name", "type")
    xmlsendemailparam.setAttribute("value", "ATS3_REPORT")
    xmlsendemailparams.appendChild(xmlsendemailparam)
 
    xmlsendemailparam = xmldoc.createElement("param")
    xmlsendemailparam.setAttribute("name", "to")
    xmlsendemailparam.setAttribute("value","ext-aminul.2.islam@nokia.com;yongjiang.yu@nokia.com")
    xmlsendemailparams.appendChild(xmlsendemailparam)

    xmlsendemailparam = xmldoc.createElement("param")
    xmlsendemailparam.setAttribute("name", "subject")
    xmlsendemailparam.setAttribute("value","LC API test report")
    xmlsendemailparams.appendChild(xmlsendemailparam)
    
    xmlsendemailparam = xmldoc.createElement("param")
    xmlsendemailparam.setAttribute("name", "unzip")
    xmlsendemailparam.setAttribute("value","true")
    xmlsendemailparams.appendChild(xmlsendemailparam)
    
    xmlsendemailparam = xmldoc.createElement("param")
    xmlsendemailparam.setAttribute("name", "send-files")
    xmlsendemailparam.setAttribute("value","true")
    xmlsendemailparams.appendChild(xmlsendemailparam)

    xmlsendemailparam = xmldoc.createElement("param")
    xmlsendemailparam.setAttribute("name", "report-dir")
    xmlsendemailparam.setAttribute("value","\\\\jamppa\\ATS\\filestore\\api_auto_test_logs")
    xmlsendemailparams.appendChild(xmlsendemailparam)    

    # Print our newly created XML and create test.xml
    print xmldoc.toprettyxml(indent="     ")
    xmlfile = open(Directory + "\\test.xml", "w")
    xmldoc.writexml(xmlfile, indent="     ", addindent="     ", newl="\n")
    xmlfile.close()

    #create the testdrop.zip file
    os.chdir(Directory)    

    ret = os.system("zip -R testdrop.zip test.xml  \".\\ATS3drop\\*\" \".\\ATS3drop\\armv5_urel\\*\" \".\\ATS3drop\\images\\*\"")
    if(ret != 0):
        print "Error : unable to create test.xml"
        sys.exit()

print "\n Test Drop Sucessfully created in "+Directory
