#!/usr/bin/env python
# -*- coding: utf-8 -*-
import argparse
import os
import urllib2
import shlex
import subprocess
import random
import psutil
import argparse
import os
import urllib2
import shlex
import subprocess
import random
import sys
import shutil
import time
import signal
import base64
import hashlib
import json
import requests
import multiprocessing
import logging

from datetime import datetime


class CVerify:

    def __init__( self, 
                  cb_id,
                  cb_name, 
                  crashName
                ):
        self.cwd              = os.getcwd()
        self.cb_id            = cb_id
        self.cb_name          = cb_name
        self.crashName        = crashName

        self.pindir           = self.cwd + "/CVerify/pin-3.4"
        self.pin              = self.cwd + "/CVerify/pin-3.4/pin"
        self.basePinTracerCPP = self.cwd + "/CVerify/pinTracer.cpp"


    def prepare_pincppFile(self):        

        self.pintoolSrc = self.cwd + "/CVerify/pinTracer_" + str(self.cb_id) + ".cpp"
        self.pintoolObj = self.cwd + "/CVerify/pinTracer_" + str(self.cb_id) + ".o"
        self.pinoutPlg  = self.cwd + "/CVerify/pinTracer_" + str(self.cb_id) + ".so"

        shutil.copyfile(self.basePinTracerCPP, self.pintoolSrc)

        # ----------------------------------------------------------------------------------------------------- #
        cmd = "sed -i " +  "'s#CRASHFILE_NAME#" + self.crashName + "#g' " + self.pintoolSrc
        cmd_list = shlex.split(cmd)
        devnull = open(os.devnull, 'wb')
        proc = subprocess.Popen(cmd_list, shell=False, stdout=devnull, stderr=devnull)
        proc.communicate()
        # ----------------------------------------------------------------------------------------------------- #


        # ----------------------------------------------------------------------------------------------------- #
        cf_sz =  os.path.getsize(self.crashName)
        cmd = "sed -i " +  "'s#CRASH_FILESIZE#" + str(cf_sz) + "#g' " + self.pintoolSrc
        cmd_list = shlex.split(cmd)
        devnull = open(os.devnull, 'wb')
        proc = subprocess.Popen(cmd_list, shell=False, stdout=devnull, stderr=devnull)
        proc.communicate()
        # ----------------------------------------------------------------------------------------------------- #


    def compile_pintool(self):
        compile_cmd = "g++ -Wall -Werror -Wno-unknown-pragmas -D__PIN__=1 -DPIN_CRT=1 -fno-stack-protector -fno-exceptions "
        compile_cmd = compile_cmd + "-funwind-tables -fasynchronous-unwind-tables -fno-rtti -DTARGET_IA32 -DHOST_IA32 -DTARGET_LINUX -fabi-version=2 "
        compile_cmd = compile_cmd + "-I" + self.pindir + "/source/include/pin "
        compile_cmd = compile_cmd + "-I" + self.pindir + "/source/include/pin/gen "
        compile_cmd = compile_cmd + "-isystem " + self.pindir + "/extras/stlport/include "
        compile_cmd = compile_cmd + "-isystem " + self.pindir + "/extras/libstdc++/include "
        compile_cmd = compile_cmd + "-isystem " + self.pindir + "/extras/crt/include "
        compile_cmd = compile_cmd + "-isystem " + self.pindir + "/extras/crt/include/arch-x86 "
        compile_cmd = compile_cmd + "-isystem " + self.pindir + "/extras/crt/include/kernel/uapi "
        compile_cmd = compile_cmd + "-isystem " + self.pindir + "/extras/crt/include/kernel/uapi/asm-x86 "
        compile_cmd = compile_cmd + "-I" + self.pindir + "/extras/components/include "
        compile_cmd = compile_cmd + "-I" + self.pindir + "/extras/xed-ia32/include/xed "
        compile_cmd = compile_cmd + "-I" + self.pindir + "/source/tools/InstLib "
        compile_cmd = compile_cmd + "-O3 -fomit-frame-pointer -fno-strict-aliasing -m32 -c -o "
        compile_cmd = compile_cmd + self.pintoolObj + " "
        compile_cmd = compile_cmd + self.pintoolSrc

        os.system(compile_cmd)

        link_cmd = "g++ -shared -Wl,--hash-style=sysv " 
        link_cmd = link_cmd + self.pindir + "/ia32/runtime/pincrt/crtbeginS.o "
        link_cmd = link_cmd + "-Wl,-Bsymbolic -Wl,--version-script=" 
        link_cmd = link_cmd + self.pindir + "/source/include/pin/pintool.ver " 
        link_cmd = link_cmd + "-fabi-version=2 -m32 -o "
        link_cmd = link_cmd + self.pinoutPlg + " "
        link_cmd = link_cmd + self.pintoolObj + " "
        link_cmd = link_cmd + "-L" + self.pindir + "/ia32/runtime/pincrt "
        link_cmd = link_cmd + "-L" + self.pindir + "/ia32/lib "
        link_cmd = link_cmd + "-L" + self.pindir + "/ia32/lib-ext "
        link_cmd = link_cmd + "-L" + self.pindir + "/extras/xed-ia32/lib "
        link_cmd = link_cmd + "-lpin -lxed "
        link_cmd = link_cmd + self.pindir + "/ia32/runtime/pincrt/crtendS.o "
        link_cmd = link_cmd + "-lpin3dwarf -ldl-dynamic -nostdlib -lstlport-dynamic -lm-dynamic -lc-dynamic -lunwind-dynamic "
        link_cmd = link_cmd + "-Wl,-rpath=" + self.pindir + "/link-lib/"

        #print "link: " + link_cmd
        os.system(link_cmd)


    def run_pintool(self):
        self.outKnobFile = self.cwd + "/CVerify/pin-out/" + str(self.cb_id) + "/" + "knob" 
        cmd = self.pin + " -t " + self.pinoutPlg + " -o " + self.outKnobFile + " -- " + self.cb_name + " < " + self.crashName

        #print "run: " + cmd
        os.system(cmd)

    ## return value:
    ## '0': not exploitable !
    ## '1': EIP overwritten
    def check(self):
        file_sz = os.path.getsize(self.outKnobFile)
        if(file_sz == 0):
            return '0'

        fd = open(self.outKnobFile, "r")
        content = fd.read()
        fd.close()
        return content[0]


    def copy_to_ExpGen(self):
        self.expgenFile = self.cwd + "/ExpGen/crashes/" + str(self.cb_id) + "/" + os.path.basename(self.crashName)  
        shutil.copyfile(self.crashName, self.expgenFile)


    def verify(self):
        self.prepare_pincppFile()
        self.compile_pintool()
        self.run_pintool()
        res = self.check()
	print "outKnobFile: " + self.outKnobFile
	print "res: " + str(res)
        #if res != '0':
        #    self.copy_to_ExpGen()



def parse_args():
    """parse arguments
    :returns: dictionary representing the parsed arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--cb_id', type=int, required=True)
    parser.add_argument('--cb_path', type=str, required=True)
    parser.add_argument('--crashFile_Name', type=str, required=True)

    args = parser.parse_args()
    kwargs = vars(args)

    return kwargs


def main():
    print "note"
    argv    = parse_args()
    cverify = CVerify( argv['cb_id'],
                       argv['cb_path'],
                       argv['crashFile_Name']
                     )
    cverify.verify()
        

if __name__ == "__main__":
    main()

