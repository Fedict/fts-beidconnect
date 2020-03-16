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

    def _est3_read_usercerts(self):
        cmd = {
            "operation":"CERT",
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"3daa114f-1ae9-4ae1-ba15-c79f7383ab35",
            "origin":"https://eazysign-qa.zetes.be",
        }
        resp = self.transceive(json.dumps(cmd))
        #self.assertEqual(resp['nonce'], original_nonce)

    def est_sign(self):
        certB64 = "MIIEiDCCA3CgAwIBAgIQAQAAAAABQ6Ge0qIAAM7ABDANBgkqhkiG9w0BAQUFADArMQswCQYDVQQGEwJCRTEcMBoGA1UEAxMTZUlEIHRlc3QgQ2l0aXplbiBDQTAeFw0xNDAxMTcxOTEyNTZaFw0yNDAxMTcxOTEyNTZaMHgxCzAJBgNVBAYTAkJFMSMwIQYDVQQDExpBbGljZSBTUEVDSU1FTiAoU2lnbmF0dXJlKTERMA8GA1UEBBMIU1BFQ0lNRU4xGzAZBgNVBCoTEkFsaWNlIEdlbGRpZ2VrYWFydDEUMBIGA1UEBRMLNzE3MTUxMDAwNzAwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDLrHTmOQq61yWzZQZlN1UYfrjWIQFlL7Jc72QSvqOREh/m04rShZfObWinU4RFhEzewIKFe5sQSb/F24WtcLy1eckQPLGjE0Yfx6KjxTal98qTrnd9L17/XiTG6LdS9F6sm9FJSZQxPXjNaboIUa4PpkBmMYn5wOJRAiiTrtujY8kkGoydUTv+9P/Yxen1dltuzk02UCzpt1h4S6pnuOh0VvdnbiAw9ekBL4zWutMK9tNZKEoJCW1DUJ73XGphWzA8AVpijll6WJJyEx2AqyqscMNBRQ3gFtdajJ47wBW7JSoCykXNH0TNW6KU+Epm+1SpGcA0/DCrWrv7XkUS2lepAgMBAAGjggFZMIIBVTAOBgNVHQ8BAf8EBAMCBkAwRwYDVR0gBEAwPjA8BgdgOAEoKCgBMDEwLwYIKwYBBQUHAgEWI2h0dHA6Ly9jZXJ0cy5kZXYuZWlkLmJlbGdpdW0uYmUvY3BzMDoGA1UdHwQzMDEwL6AtoCuGKWh0dHA6Ly9jcmwuZGV2LmVpZC5iZWxnaXVtLmJlL2NpdGl6ZW4uY3JsMBgGCCsGAQUFBwEDBAwwCjAIBgYEAI5GAQEwcAYIKwYBBQUHAQEEZDBiMDQGCCsGAQUFBzAChihodHRwOi8vY2VydHMuZGV2LmVpZC5iZWxnaXVtLmJlL3Jvb3QuY3J0MCoGCCsGAQUFBzABhh5odHRwOi8vb2NzcC5kZXYuZWlkLmJlbGdpdW0uYmUwEQYJYIZIAYb4QgEBBAQDAgUgMB8GA1UdIwQYMBaAFAaLj6IA8bWGg/x7UiXGcTw0JJbCMA0GCSqGSIb3DQEBBQUAA4IBAQBel+5OA/oN2kJh2ocM8zxqNmaSJUcVt0LsdxtI8o5wZFeizhZjOOzjWsJPJ0zVEllJBiFEG0hSgxULpYQDZeR+EABAPb6dfCVFCLKhbm5o3gvp1d7HD6VzClmX7Sx+OqeffM560HtPp+e9s8xMCIYrjN2C5+Yl1/hFwxuol/F7hlRUPH2q30HT5gSkqNA8hPwfvqhCTeBmMm7P9H5Uu7lm438yrdce0udBZSWqlK0SO3dQNL3pummSKZbotlUCPuN8pNLw9OM7VcjoBqL3zjyUccYHSw38Oq0XlvMSTOTn2yMUUVL/uKvL5kV17nj+ztP9HZD31Re1ay5WoG3kmhmH"
        cmd = {
            "operation":"SIGN",
            "cert": certB64,
            "algo":"SHA-256",
            "digest":"JMRVmmssdqPelSUruIhdDOTGXc2Y2dOq8Bf989ZDPH0=",
            "language":"en",
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"07386ce7-f73e-4e99-dfc3-8d69b6adf33d",
            "src":"page.js",
            "origin":"https://eazysign-qa.zetes.be",
            "tab":4
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
