#!/usr/bin/python

import json
import subprocess
import struct
import sys
import unittest
import uuid
import re
import testconf

unittest.TestLoader.sortTestMethodsUsing = None

# https://developer.chrome.com/extensions/nativeMessaging#native-messaging-host-protocol

def instruct(msg):
    input('%s\n[press ENTER to continue]' % msg)

class TestLongrunningHost(unittest.TestCase):

    _pin = 0
     
    @property
    def pin(self):
        print("Getting pin...")
        return self._pin

    @pin.setter
    def pin(self, value):
        print("Setting pin...")
        self._pin = value

    @classmethod
    def setUpClass(self):
        print(' *** Testing BeIDConnect as native messaging host *** ')
        print('this test also needs a pin for testing dummy signatures')
        print ()
        #self.pin = input("Enter Pin: ")
        self.pin = "1234"

    @classmethod
    def tearDownClass(cls):
        print(' *** Finished testing BeIDConnect *** ')

    def transceive(self, msg):
        should_close_fds = sys.platform.startswith('win32') == False
        p = subprocess.Popen(testconf.get_exe(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=should_close_fds, stderr=None)
        print()
        print(('SEND: %s' % msg))
        p.stdin.write(struct.pack('=I', len(msg)))
        std_output = p.communicate(input=msg.encode())[0]
        response_length = struct.unpack_from('=I', std_output)[0]
        response_length, response = struct.unpack_from('=I%is' % (response_length-2), std_output)
        response_print = json.dumps(json.loads(response))
        print(('RECV: %s' % response_print))
        return json.loads(response)

    def complete_msg(self, msg):
        msg['nonce'] = str(uuid.uuid4())
        msg['lang'] = 'en'
        msg['origin'] = 'https://example.com/test'
        return msg

    def test1_version(self):
         cmd = {
             "operation":"VERSION",
             "correlationId":"07386ce7-f73e-4e99-dfc3-8d69b6adf33d"
         }
         resp = self.transceive("{\"operation\":\"VERSION\",\"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\"}")
         self.assertEqual(resp['version'], "2.1")
         self.pin = 1234

    def test3_read_usercerts(self):
        cmd = {
            "operation":"USERCERTS",
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"3daa114f-1ae9-4ae1-ba15-c79f7383ab35",
            "origin":"https://something.belgium.be",
        }
        resp = self.transceive(json.dumps(cmd))
        self.assertEqual(resp['result'], "OK")

    def test4a_read_certificate_chain_nonrep(self):
        cmd = {
            "operation":"USERCERTS",
            "keyusage":"NONREPUDIATION",
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"3daa114f-1ae9-4ae1-ba15-c79f7383ab35",
            "origin":"https://something.belgium.be",
        }
        resp = self.transceive(json.dumps(cmd))
        reader = (resp)['Readers'][0]
        cert = (reader)['certificates'][0]
        self.assertEqual(resp['result'], "OK")

        cmd = {
            "operation":"CERTCHAIN",
            "cert": cert,
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"3daa114f-1ae9-4ae1-ba15-c79f7383ab35",
            "origin":"https://something.belgium.be",
        }
        resp = self.transceive(json.dumps(cmd))
        self.assertEqual(resp['result'], "OK")

    def test4b_read_certificate_chain_aut(self):
        cmd = {
            "operation":"USERCERTS",
            "keyusage":"DIGITALSIGNATURE",
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"3daa114f-1ae9-4ae1-ba15-c79f7383ab35",
            "origin":"https://something.belgium.be",
        }
        resp = self.transceive(json.dumps(cmd))
        self.assertEqual(resp['result'], "OK")

        reader = (resp)['Readers'][0]
        cert = (reader)['certificates'][0]
        cmd = {
            "operation":"CERTCHAIN",
            "cert": cert,
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"3daa114f-1ae9-4ae1-ba15-c79f7383ab35",
            "origin":"https://something.belgium.be",
        }
        resp = self.transceive(json.dumps(cmd))
        self.assertEqual(resp['result'], "OK")

    def test_sign(self):
        cmd = {
            "operation":"USERCERTS",
            "keyusage":"NONREPUDIATION",
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"3daa114f-1ae9-4ae1-ba15-c79f7383ab35",
            "origin":"https://something.belgium.be",
        }
        resp = self.transceive(json.dumps(cmd))
        reader = (resp)['Readers'][0]
        cert = (reader)['certificates'][0]

        instruct

        cmd = {
            "operation":"SIGN",
            "cert": cert,
            "algo":"SHA-256",
            "digest":"JMRVmmssdqPelSUruIhdDOTGXc2Y2dOq8Bf989ZDPH0=",
            "pin": self.pin,
            "language":"en",
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"07386ce7-f73e-4e99-dfc3-8d69b6adf33d",
            "origin":"https://something.belgium.be",
        }
        resp = self.transceive(json.dumps(cmd))
        self.assertEqual(resp['result'], "OK")


if __name__ == '__main__':
    if len(sys.argv) == 2:
        # Run a single test given in the argument
        singleTestName = sys.argv[1]
        suite = unittest.TestSuite()
        suite.addTest(TestLongrunningHost(singleTestName))
        runner = unittest.TextTestRunner()
        # Run test suite with a single test
        runner.run(suite)
    else:
        # Run all the tests
        unittest.main()
