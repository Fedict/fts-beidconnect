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
    raw_input('%s\n[press ENTER to continue]' % msg)

class TestLongrunningHost(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        print(' *** Testing eIDLink as native messaging host *** ')

    @classmethod
    def tearDownClass(cls):
        print(' *** Finished testing eIDLink *** ')

    def transceive(self, msg):
        should_close_fds = sys.platform.startswith('win32') == False
        p = subprocess.Popen(testconf.get_exe(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=should_close_fds, stderr=None)
        print('SEND: %s' % msg)
        p.stdin.write(struct.pack('=I', len(msg)))
        std_output = p.communicate(input=msg.encode())[0]
        response_length = struct.unpack_from('=I', std_output)[0]
        response_length, response = struct.unpack_from('=I%is' % (response_length-2), std_output)
        response_print = json.dumps(json.loads(response))
        print('RECV: %s' % response_print)
        p.terminate()
        p.wait()
        return json.loads(response)

    def complete_msg(self, msg):
        msg['nonce'] = str(uuid.uuid4())
        msg['lang'] = 'en'
        msg['origin'] = 'https://example.com/test'
        return msg

    def test1_version(self):
         cmd = {
             "operation":"VERSION",
             "mac":"0123456789ABCDEF0123456789ABCDEF",
             "correlationId":"07386ce7-f73e-4e99-dfc3-8d69b6adf33d"
         }
         resp = self.transceive(json.dumps(cmd))
         self.assertEqual(resp['version'], "1.0.0")
   
    def test2_info(self):
         cmd = {
            "operation":"INFO",
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"07386ce7-f73e-4e99-dfc3-8d69b6adf33d"
         }
         resp = self.transceive(json.dumps(cmd))

    def test3_read_usercerts(self):
        cmd = {
            "operation":"USERCERTS",
            "type":"NONREPUDIATION",
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"3daa114f-1ae9-4ae1-ba15-c79f7383ab35",
            "origin":"https://eazysign-qa.zetes.be",
        }
        resp = self.transceive(json.dumps(cmd))
        #self.assertEqual(resp['nonce'], original_nonce)

    def test4_read_certificate_chain(self):
        cert = "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"
        cmd = {
            "operation":"CERTCHAIN",
            "cert": cert,
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"3daa114f-1ae9-4ae1-ba15-c79f7383ab35",
            "origin":"https://eazysign-qa.zetes.be",
        }
        resp = self.transceive(json.dumps(cmd))
        #self.assertEqual(resp['nonce'], original_nonce)

    def est_sign(self):
        cert = "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"
        cmd = {
            "operation":"SIGN",
            "cert": cert,
            "algo":"SHA-256",
            "digest":"JMRVmmssdqPelSUruIhdDOTGXc2Y2dOq8Bf989ZDPH0=",
            "language":"en",
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"07386ce7-f73e-4e99-dfc3-8d69b6adf33d",
            "origin":"https://eazysign-qa.zetes.be",
        }
        resp = self.transceive(json.dumps(cmd))


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
