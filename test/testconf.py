#
# BeIDConnect Native Host
#
#

import sys
import os.path

def get_exe():
    return {
        'darwin': ['./Build/Release/BeIDConnect', ''],
        'win32':['.\VS_2019\Release\BeIDConnect.exe', ''],
        'linux':  ['./linux/beidconnect', '']
    }.get(sys.platform, '')
