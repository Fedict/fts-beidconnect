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
        cert = "MIIF1TCCA72gAwIBAgIQEAAAAAAA8evx4wAAAAGTsTANBgkqhkiG9w0BAQsFADAtMQswCQYDVQQGEwJCRTEeMBwGA1UEAwwVZUlEIFRFU1QgRm9yZWlnbmVyIENBMB4XDTE3MDUyOTIyMDAwMFoXDTIwMDUyOTIyMDAwMFowfTEUMBIGA1UEBRMLOTgwODIxMDAwMjkxEzARBgNVBCoMCkJlcnQgQWxpY2UxGDAWBgNVBAUTD1NwZWNpbWVuIGVWSyAxODEpMCcGA1UEAwwgQmVydCBTcGVjaW1lbiBlVksgMTggKFNpZ25hdHVyZSkxCzAJBgNVBAYTAkJFMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA49oyeB2rwqGNkwENXhBuZzXWKJN4RESxd+oLUb8jliwzb2KWk97nk7FhvBGAM7FFV6EP6lZKXmByMsUrP0f0zYkwsPl2SQVMJRiPpBdbE+ZyJVxersSdaTysDuu2RZKR+F1PbwQMEEfGgH4cJ2UVOb/tV4MjJV5KlnrV13IHFESJh/xbJA//Wphjh5kCNpdnN6TWolN3218hJkAvV+98no3MHg0uzQ01NkBF2U0x5llhlJAVX5ua+IZ6k56BL/Uxo6jMInLxzaNxCcK2glSbPyJMXMj01sNTIW58NBMMTktQkkrTX+DGH8edf38xjmsBBOIBf3VptYdwiwjgEmaAWwIDAQABo4IBnzCCAZswHwYDVR0jBBgwFoAU/TbPuTELnWd7LuET5UCD9wgdjJgwDgYDVR0PAQH/BAQDAgZAMGkGA1UdIARiMGAwXgYHYDgMAQECATBTMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MB4GCCsGAQUFBwICMBIaEFRFU1QgdXNlciBub3RpY2UwIgYIKwYBBQUHAQMEFjAUMAgGBgQAjkYBATAIBgYEAI5GAQQwRQYDVR0fBD4wPDA6oDigNoY0aHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY3JsL2ZvcmVpZ25lcmNhLmNybDB/BggrBgEFBQcBAQRzMHEwPAYIKwYBBQUHMAKGMGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NlcnQvcm9vdGNhLmNydDAxBggrBgEFBQcwAYYlaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmU6ODg4ODARBglghkgBhvhCAQEEBAMCBSAwDQYJKoZIhvcNAQELBQADggIBABlETs6MUuqAnCU1yQaNkXa+JTNZ02XNOMTbmwhBLTcqnfyM5vxIu4OwBAfThgf735pr3ffe8kXFelypegV9tNufdmRjqAFq9CVcNbMTsViAd3/TaPCIhveicD4a78KBKwMcEXWvmsoHlYAdM5nZokpJg7sX2NhpjVC8JexJLJKiPRNvSzKTmuUZtus0M1f3E4dYGjQMH3UpVp3bE5OAo//2Y29P6O82Nfi2vJjxAE6YBzAQltb+oEuhKiR0HS/YSgXaYPULlRmPyxetd30tZejgufNyZWjl2+v7xvBym+wBf64Vft+pgO0Dxdq6wycjFF+MtdsfhGQ7Fq7KWut8W7sLzhuXTvheFCzxQzsabQG4kqOZtC7cj32usiL8wb82NdonrLXYnze0RBb3S0azcldcQbEwYX/UInFOnHsKOALTR8Ho4uQRVDmT2XZEHlXWCVg5sA3sT0tWZCfDaoTPkjXMZwVVdat8Os/pb7pBrVe7aig36ME0chC3BscToCM2g56hEWxTy28tOR98O0jEfpxmkL4eJRwhkd610LuzwSkRrhUBZZwvc94aD6F3njxbFXB6xGN53qlwsJXPxFLocRP4a57fwaoU8MVYuEkceOdYOR4G/G0Kfm3bIdTEJ44jIha1UATNIQ287it7UfPYdmRAi0X0JaCX9U6DEg0vSUqi"
        cmd = {
            "operation":"CERTCHAIN",
            "cert": cert,
            "mac":"0123456789ABCDEF0123456789ABCDEF",
            "correlationId":"3daa114f-1ae9-4ae1-ba15-c79f7383ab35",
            "origin":"https://eazysign-qa.zetes.be",
        }
        resp = self.transceive(json.dumps(cmd))
        #self.assertEqual(resp['nonce'], original_nonce)

    def test_sign(self):
        cert = "MIIF1TCCA72gAwIBAgIQEAAAAAAA8evx4wAAAAGTsTANBgkqhkiG9w0BAQsFADAtMQswCQYDVQQGEwJCRTEeMBwGA1UEAwwVZUlEIFRFU1QgRm9yZWlnbmVyIENBMB4XDTE3MDUyOTIyMDAwMFoXDTIwMDUyOTIyMDAwMFowfTEUMBIGA1UEBRMLOTgwODIxMDAwMjkxEzARBgNVBCoMCkJlcnQgQWxpY2UxGDAWBgNVBAUTD1NwZWNpbWVuIGVWSyAxODEpMCcGA1UEAwwgQmVydCBTcGVjaW1lbiBlVksgMTggKFNpZ25hdHVyZSkxCzAJBgNVBAYTAkJFMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA49oyeB2rwqGNkwENXhBuZzXWKJN4RESxd+oLUb8jliwzb2KWk97nk7FhvBGAM7FFV6EP6lZKXmByMsUrP0f0zYkwsPl2SQVMJRiPpBdbE+ZyJVxersSdaTysDuu2RZKR+F1PbwQMEEfGgH4cJ2UVOb/tV4MjJV5KlnrV13IHFESJh/xbJA//Wphjh5kCNpdnN6TWolN3218hJkAvV+98no3MHg0uzQ01NkBF2U0x5llhlJAVX5ua+IZ6k56BL/Uxo6jMInLxzaNxCcK2glSbPyJMXMj01sNTIW58NBMMTktQkkrTX+DGH8edf38xjmsBBOIBf3VptYdwiwjgEmaAWwIDAQABo4IBnzCCAZswHwYDVR0jBBgwFoAU/TbPuTELnWd7LuET5UCD9wgdjJgwDgYDVR0PAQH/BAQDAgZAMGkGA1UdIARiMGAwXgYHYDgMAQECATBTMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MB4GCCsGAQUFBwICMBIaEFRFU1QgdXNlciBub3RpY2UwIgYIKwYBBQUHAQMEFjAUMAgGBgQAjkYBATAIBgYEAI5GAQQwRQYDVR0fBD4wPDA6oDigNoY0aHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY3JsL2ZvcmVpZ25lcmNhLmNybDB/BggrBgEFBQcBAQRzMHEwPAYIKwYBBQUHMAKGMGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NlcnQvcm9vdGNhLmNydDAxBggrBgEFBQcwAYYlaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmU6ODg4ODARBglghkgBhvhCAQEEBAMCBSAwDQYJKoZIhvcNAQELBQADggIBABlETs6MUuqAnCU1yQaNkXa+JTNZ02XNOMTbmwhBLTcqnfyM5vxIu4OwBAfThgf735pr3ffe8kXFelypegV9tNufdmRjqAFq9CVcNbMTsViAd3/TaPCIhveicD4a78KBKwMcEXWvmsoHlYAdM5nZokpJg7sX2NhpjVC8JexJLJKiPRNvSzKTmuUZtus0M1f3E4dYGjQMH3UpVp3bE5OAo//2Y29P6O82Nfi2vJjxAE6YBzAQltb+oEuhKiR0HS/YSgXaYPULlRmPyxetd30tZejgufNyZWjl2+v7xvBym+wBf64Vft+pgO0Dxdq6wycjFF+MtdsfhGQ7Fq7KWut8W7sLzhuXTvheFCzxQzsabQG4kqOZtC7cj32usiL8wb82NdonrLXYnze0RBb3S0azcldcQbEwYX/UInFOnHsKOALTR8Ho4uQRVDmT2XZEHlXWCVg5sA3sT0tWZCfDaoTPkjXMZwVVdat8Os/pb7pBrVe7aig36ME0chC3BscToCM2g56hEWxTy28tOR98O0jEfpxmkL4eJRwhkd610LuzwSkRrhUBZZwvc94aD6F3njxbFXB6xGN53qlwsJXPxFLocRP4a57fwaoU8MVYuEkceOdYOR4G/G0Kfm3bIdTEJ44jIha1UATNIQ287it7UfPYdmRAi0X0JaCX9U6DEg0vSUqi"
        cmd = {
            "operation":"SIGN",
            "cert": cert,
            "algo":"SHA-256",
            "digest":"JMRVmmssdqPelSUruIhdDOTGXc2Y2dOq8Bf989ZDPH0=",
            "pin": "1234",
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
