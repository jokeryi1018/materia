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
        shutil.copyfile(self.basePinTracerCPP, self.pintoolSrc)

        # ----------------------------------------------------------------------------------------------------- #
        cmd = "sed -i " +  "'s#CRASHFILE_NAME#" + self.crashName + "#g' " + self.pintoolSrc
        cmd_list = shlex.split(cmd)
        devnull = open(os.devnull, 'wb')
        proc = subprocess.Popen(cmd_list, shell=False, stdout=devnull, stderr=devnull)
        proc.communicate()
        # ----------------------------------------------------------------------------------------------------- #


    def compile_pintool(self):
        compile_cmd = "g++ -Wall -Werror -Wno-unknown-pragmas -D__PIN__=1 -DPIN_CRT=1 -fno-stack-protector -fno-exceptions " +
                      "-funwind-tables -fasynchronous-unwind-tables -fno-rtti -DTARGET_IA32E -DHOST_IA32E -fPIC -DTARGET_LINUX -fabi-version=2 " + 
                      "-I" + self.pindir + "/source/include/pin " + "-I" + self.pindir + "/source/include/pin/gen " + 
                      "-isystem " + self.pindir + "/extras/stlport/include " + 
                      "-isystem " + self.pindir + "/extras/libstdc++/include " + 
                      "-isystem " + self.pindir + "/extras/crt/include " + 
                      "-isystem " + self.pindir + "/extras/crt/include/arch-x86_64 " + 
                      "-isystem " + self.pindir + "/extras/crt/include/kernel/uapi " + 
                      "-isystem " + self.pindir + "/extras/crt/include/kernel/uapi/asm-x86 " + 
                      "-I" + self.pindir + "/extras/components/include " + "-I" + self.pindir + "/extras/xed-intel64/include/xed " + 
                      "-I" + self.pindir + "/source/tools/InstLib " + 
                      "-O3 -fomit-frame-pointer -fno-strict-aliasing -c -o " + 
                      self.pintoolObj + " " + self.pintoolSrc
        os.system(compile_cmd)

        link_cmd = "g++ -shared -Wl,--hash-style=sysv " + self.pindir + "/ia32/runtime/pincrt/crtbeginS.o " + 
                   "-Wl,-Bsymbolic -Wl,--version-script=" + self.pindir + "/source/include/pin/pintool.ver " + 
                   "-fabi-version=2 -o " 
                   obj-intel64/inscount1.so obj-intel64/inscount1.o  -L../../../intel64/runtime/pincrt -L../../../intel64/lib -L../../../intel64/lib-ext -L../../../extras/xed-intel64/lib -lpin -lxed ../../../intel64/runtime/pincrt/crtendS.o -lpin3dwarf  -ldl-dynamic -nostdlib -lstlport-dynamic -lm-dynamic -lc-dynamic -lunwind-dynamic"
        os.system(link_cmd)


    def run_pintool(self):
        # ../../../pin -t obj-intel64/inscount0.so -o inscount0.log -- /bin/ls
        self.pinplugin =  

        # ----------------------------------------------------------------------------------------------------- #
        cmd = self.pin + " -t " + self.pinplugin + " -o " +  
        os.system(cmd)
        # ----------------------------------------------------------------------------------------------------- #
 

    def verify(self):
        self.prepare_pincppFile()
        self.compile_pintool()
        self.run_pintool()



def parse_args():
    """parse arguments
    :returns: dictionary representing the parsed arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--cb_id', type=int, required=True)
    parser.add_argument('--cb_name', type=str, required=True)
    parser.add_argument('--crashFile_Name', type=str, required=True)

    args = parser.parse_args()
    kwargs = vars(args)

    return kwargs


def main():
    argv    = parse_args()
    cverify = CVerify( argv['cb_id'],
                       argv['cb_name'],
                       argv['crashFile_Name']
                     )
    cverify.verify()
        

if __name__ == "__main__":
    main

