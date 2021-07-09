#include "BEIDCard.hpp"
#include "log.hpp"
#include "util.h"
#include "general.h"
#include "hash.h"
#include "asn1.hpp"
#include "x509Util.h"
#include <unordered_map>

#define BEID_READ_BINARY_MAX_LEN     250			 /*maximum length to read with single card command*/

#define prepareAuthenticationCmd			   		"\x00\x22\x41\xB6\x05\x04\x80\x01\x84\x82"
#define prepareAuthenticationSHA1Cmd				"\x00\x22\x41\xB6\x05\x04\x80\x02\x84\x82"
#define prepareAuthenticationMD5Cmd					"\x00\x22\x41\xB6\x05\x04\x80\x04\x84\x82"

#define prepareNonRepudiationCmd					   "\x00\x22\x41\xB6\x05\x04\x80\x01\x84\x83"
#define prepareNonRepudiationSHA1Cmd				"\x00\x22\x41\xB6\x05\x04\x80\x02\x84\x83"
#define prepareNonRepudiationMD5Cmd					"\x00\x22\x41\xB6\x05\x04\x80\x04\x84\x83"

//#define resetPinApdu                  "\x00\x20\x00\x02\x08\x2C\x33\x33\x33\x11\x11\x11\xFF" (13 bytes) New PIN: 1234

#define unblockCardApdu								"\x00\x2c\x00\x01\x08\x2c\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
#define verifyPinApdu								"\x00\x20\x00\x01\x08\x2f\xff\xff\xff\xff\xff\xff\xff"
#define logoutApdu									"\x80\xE6\x00\x00"
#define beid_apdu_change_pin_1						"\x00\x24\x00\x01\x08\x2f\xff\xff\xff\xff\xff\xff\xff"

#define beid_apdu_change_pin_0					"\x00\x24\x01\x01\x10\x2f\xff\xff\xff\xff\xff\xff\xff"\
"\x2f\xff\xff\xff\xff\xff\xff\xff"

#define generateSignatureCmd						"\x00\x2A\x9E\x9A"
#define retrieveSignatureCmd						"\x00\xC0\x00\x00\x80"
#define getResultCmd                         "\x00\xC0\x00\x00\x00"
#define selectFileCmd								"\x00\xa4\x02\x00\x02\x3f\x00"
#define selectSerialCmd								"\x00\xA4\x08\x0C\x06\x3F\x00\xDF\x00\x50\x32"
#define selectAuthenticationCertificateCmd	"\x00\xA4\x08\x0C\x06\x3F\x00\xDF\x00\x50\x38"
#define selectNonRepudiationCertificateCmd	"\x00\xA4\x08\x0C\x06\x3F\x00\xDF\x00\x50\x39"
#define selectCaCertificateCmd					"\x00\xA4\x08\x0C\x06\x3F\x00\xDF\x00\x50\x3a"
#define selectRootCaCertificateCmd				"\x00\xA4\x08\x0C\x06\x3F\x00\xDF\x00\x50\x3b"

#define select_Belpic_AID							"\x00\xA4\x04\x0C\x0C\xA0\x00\x00\x01\x77\x50\x4B\x43\x53\x2D\x31\x35\x00"
#define select_ID_AID								"\x00\xA4\x04\x0C\x0C\xA0\x00\x00\x01\x77\x49\x64\x46\x69\x6C\x65\x73\x00"

#define selectBEIDApplet							"\x00\xA4\x04\x00\x0F\xA0\x00\x00\x00\x30\x29\x05\x70\x00\xAD\x13\x10\x01\x01\xFF"

#define selectCardMgr								"\x00\xA4\x04\x00\x07\xA0\x00\x00\x00\x03\x00\x00"
#define selectMF                        		"\x00\xA4\x02\x0C\x02\x3F\x00"
#define selectDF00                       		"\x00\xA4\x02\x0C\x02\xDF\x00"
#define selectDF01                    			"\x00\xA4\x02\x0C\x02\xDF\x01"
#define selectIdentityFileCmd					   "\x00\xA4\x02\x0C\x02\x40\x31" /* ends with 00=> Le=0 */
#define selectIdentityFileSignatureCmd       "\x00\xA4\x02\x0C\x02\x40\x32"
#define selectAddressFileCmd					   "\x00\xA4\x02\x0C\x02\x40\x33"
#define selectAddressFileSignatureCmd			"\x00\xA4\x02\x0C\x02\x40\x34"
#define selectPhotoCommand					      "\x00\xA4\x02\x0C\x02\x40\x35"

#define beid_apdu_binary							"\x00\xB0\x00\x00\x00"


std::string* BEIDCard::valueForKey(std::string* key)
{
   return nullptr;
}

#define WHERE "BEIDCard::read_certificate"
int BEIDCard::readCertificate(int format, int type, std::vector<char> &cert)
{
   int ret;
   int l_lengthbuf = 4;
   unsigned char lengthbuf[5];
   int l_rawcert, l_cert = 0;
   unsigned char *p_rawcert=0, *p_cert = 0;
   fstream myFile;
   std::unordered_map<int,std::vector<char>> idFiles;

   const char* rrncert  = "\x3F\x00\xDF\x00\x50\x3C";
   const char* authcert = "\x3F\x00\xDF\x00\x50\x38";
   const char* signcert = "\x3F\x00\xDF\x00\x50\x39";
   const char* cacert   = "\x3F\x00\xDF\x00\x50\x3A";
   const char* rootcert = "\x3F\x00\xDF\x00\x50\x3B";
   int path_len = 6;

   idFiles[CERT_TYPE_RRN] = std::vector<char>(rrncert, rrncert+ path_len);
   idFiles[CERT_TYPE_AUTH] = std::vector<char>(authcert, authcert+ path_len);
   idFiles[CERT_TYPE_NONREP] = std::vector<char>(signcert, signcert+ path_len);
   idFiles[CERT_TYPE_CA] = std::vector<char>(cacert, cacert+ path_len);
   idFiles[CERT_TYPE_ROOT] = std::vector<char>(rootcert, rootcert+ path_len);

   ret = Card::getFile((unsigned char *)idFiles[type].data(), path_len, &l_lengthbuf, lengthbuf);
   if (ret) {
      log_error("%s GetFile (cert) returned 0x%08X", WHERE, ret);
      CLEANUP(ret);
   }
   
   l_rawcert = (lengthbuf[2] << 8) + lengthbuf[3];
   if (l_rawcert > 0) {
      l_rawcert += 4;

      if((p_rawcert = (unsigned char*) malloc(l_rawcert)) == NULL) {
         CLEANUP(E_ALLOC_ERR);
      }
      ret = Card::getFile((unsigned char *)idFiles[type].data(), 6, &l_rawcert, p_rawcert);
      if (ret) {
         log_error("%s GetFile (cert) returned 0x%08X", WHERE, ret);
         CLEANUP(ret);
      }
   }
   
   if (format == FORMAT_RADIX64) {
      l_cert =  base64encode_len(l_rawcert);
      if ((p_cert = (unsigned char*) malloc(l_cert)) == NULL) {
         CLEANUP(E_ALLOC_ERR);
      }
      base64encode(p_rawcert, l_rawcert, p_cert);
      std::vector<char> buf(p_cert, p_cert + l_cert);
      cert = buf;
   }
   else {
      std::vector<char> buf(p_rawcert, p_rawcert + l_rawcert);
      cert = buf;
   }
   
cleanup:

   if (p_cert)
      free(p_cert);
   if (p_rawcert)
      free(p_rawcert);
   return (ret);
}
#undef WHERE


#define WHERE "BEIDCard::readUserCertificates"
int BEIDCard::readUserCertificates(int format, int certType, std::vector<std::vector<char>> &certificates)
{
   int ret = 0;

   if ((certType == 0) || (certType == CERT_TYPE_NONREP )) {
      std::vector<char> buf;
      ret = readCertificate(format, CERT_TYPE_NONREP, buf);
      if (ret) {
         log_error("%s readCertificate(non-rep) returned 0x%08X", WHERE, ret);
         CLEANUP(ret);
      }
      else {
         certificates.push_back(buf);
      }
   }

   if ((certType == 0) || (certType == CERT_TYPE_AUTH )) {
      std::vector<char> buf;
      ret = readCertificate(format, CERT_TYPE_AUTH, buf);
      if (ret) {
         log_error("%s readCertificate(non-rep) returned 0x%08X", WHERE, ret);
         CLEANUP(ret);
      }
      else {
         certificates.push_back(buf);
      }
   }
   
cleanup:
   
   return (ret);
}
#undef WHERE


#define WHERE "BEIDCard::readCertificateChain"
int BEIDCard::readCertificateChain(int format, unsigned char *cert, int l_cert, std::vector<std::vector<char>>  &subCerts, std::vector<char> &root)
{
   int ret;
   unsigned char* p_cert = 0;
#define X509_ISSUER           "\1\1\4"
#define X509_SUBJECT          "\1\1\6"
   ASN1_ITEM subject, issuer;
   
   //-----------------------------------------------------------------------------
   // CA certificate
   //-----------------------------------------------------------------------------
   std::vector<char> cacert;
   std::vector<char> rootcert;

   ret = readCertificate(FORMAT_RAW, CERT_TYPE_CA, cacert);
   if (ret) {
      log_error("%s readCertificate(ca) returned 0x%08X", WHERE, ret);
      CLEANUP(ret);
   }

   ret = asn1_get_item((unsigned char*) cacert.data(), (unsigned int)cacert.size(), X509_SUBJECT, &subject);
   if (ret) {
      log_error("%s: Could not get subject name from certificate", WHERE);
      goto cleanup;
   }
   
   ret = asn1_get_item((unsigned char*) cert, (unsigned int)l_cert, X509_ISSUER, &issuer);
   if (ret) {
      log_error("%s: Could not get issuer name from certificate", WHERE);
      goto cleanup;
   }
   
   if (asn_compare_items(&subject, &issuer) != 0) {
      //this is not the issuer we are looking for
      log_error("%s E: card does not contain the requested issuer certificate",WHERE);
      ret = -1;
      goto cleanup;
   }

   if (format == FORMAT_RADIX64) {
      l_cert =  base64encode_len((int)cacert.size());
      if ((p_cert = (unsigned char*) malloc(l_cert)) == NULL) {
         CLEANUP(E_ALLOC_ERR);
      }
      base64encode((unsigned char*) cacert.data(), (int) cacert.size(), p_cert);
      std::vector<char> buf(p_cert, p_cert + l_cert);
      subCerts.push_back(buf);
      free(p_cert);
      p_cert = NULL;
   }
   else {
      subCerts.push_back(cacert);
   }
   
   //-----------------------------------------------------------------------------
   // root certificate
   //-----------------------------------------------------------------------------
   ret = readCertificate(FORMAT_RAW, CERT_TYPE_ROOT, rootcert);
   if (ret) {
      log_error("%s readCertificate(ca) returned 0x%08X", WHERE, ret);
      CLEANUP(ret);
   }
   
   if (format == FORMAT_RADIX64) {
      l_cert =  base64encode_len((int) rootcert.size());
      if ((p_cert = (unsigned char*) malloc(l_cert)) == NULL) {
         CLEANUP(E_ALLOC_ERR);
      }
      base64encode((unsigned char*) rootcert.data(), (int)rootcert.size(), p_cert);
      std::vector<char> buf(p_cert, p_cert + l_cert);
      root = buf;
      free(p_cert);
      p_cert = NULL;
   }
   else {
      root = rootcert;
   }
   
cleanup:
   
   return (ret);
}
#undef WHERE


#define WHERE "BEIDCard::SelectKey()"
int BEIDCard::selectKey(int pintype, unsigned char* cert, int l_cert)
{
   int ret = 0;
   int cmdlen, recvlen;
   unsigned char cmd[512], recv[512];
   //int l_cardcert;
   //unsigned char* cardcert = NULL;
   int sw;

   std::vector<char> cardcert;
   ret = readCertificate(FORMAT_RAW, pintype, cardcert);
   if (ret) {
      log_error("E: could not read certificate from eID card");
      goto cleanup;
   }
   
   //check if the current card is the one that contains the certificate we want to use for signing
   //in case of multiple readers, we should search until we find the right card
   if (l_cert != cardcert.size()) {
      CLEANUP(E_SRC_CERT_NOT_FOUND);
   }
   if (memcmp(cert, cardcert.data(), l_cert) != 0) {
      CLEANUP(E_SRC_CERT_NOT_FOUND);
   }
   
   ret = getKeyInfo((unsigned char*)cardcert.data(), (unsigned int) cardcert.size(), &currentSelectedKeyType, &currentSelectedKeyLength);
   if (ret) {
      log_error("E: getKeyInfo(type,size) returned %0X (%d)", ret);
      goto cleanup;
   }

   if (pintype == CERT_TYPE_NONREP)
   {
      cmdlen = sizeof(prepareNonRepudiationCmd)-1;
      memcpy(cmd, prepareNonRepudiationCmd, cmdlen);
   }
   else
   {
      cmdlen = sizeof(prepareAuthenticationCmd)-1;
      memcpy(cmd, prepareAuthenticationCmd, cmdlen);
   }
   
   recvlen = 255;
   ret = reader->apdu(cmd, cmdlen, recv, &recvlen, &sw);
   if (ret < 0)
      goto cleanup;
   
   if ( (sw != 0x9000))
   {
      CLEANUP(-1);
   }
cleanup:
   
   return ret;
}
#undef WHERE


#define WHERE "BEIDCard::logon()"
int BEIDCard::logon(int l_pin, char *pin)
{
   int ret;
   unsigned char recv[1024];
   int recvlen = 1024;
   unsigned char cmd[255];
   int cmdlen;
   int i;
   int sw = 0;
   
   int begintransaction = 1;
   
   //begin transaction
   ret = reader->beginTransaction();
   if (ret) {
      log_error("E: Logon: Could not start transaction");
      begintransaction = 0;
      CLEANUP(E_SRC_START_TRANSACTION);
   }
   
   //test Pin conditions
   if ((l_pin > 0) && (l_pin < 4))
      CLEANUP(E_PIN_TOO_SHORT);
   if (l_pin > 12)
      CLEANUP(E_PIN_TOO_LONG);
   
   cmdlen = sizeof(verifyPinApdu)-1;
   memcpy(cmd, verifyPinApdu, cmdlen);
   
   //      pin_verify->bmFormatString = 0x89;  //FORMAT_BCD  (Belpic eid card)
   //      pin_verify->bmPINBlockString = 0x047; //0x04;//47
   //      pin_verify->bmPINLengthFormat = 0x04;
   //      pin_verify->wPINMaxExtraDigit = 0x040c; //test apg8201
   //      "\x00\x20\x00\x01\x08\x24\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
   if ((l_pin == 0) && (reader->isPinPad())) {
      ret = reader->verify_pinpad(0x89, 0x47, 0x04, 0x040c, cmd, cmdlen, &sw);  //TODO this should be handled more abstract since card shouldn't anything about the winswcard constants
      if (ret) {
         log_error("%s: reader->verify_pinpad() returned 0x%0x", WHERE, ret);
      }
   }
   else if (pin != 0) {
      //BCD encoding
      cmd[5] = 0x20 + l_pin;  /* 0x20 + length of pin */
      
      for (i = 0; i < l_pin; i++)
      {
         if (i%2 == 0)
            cmd[6+i/2] = 0x0F + (((pin[i]-48)<<4) & 0xF0);	/* pin[0], pin[1] in nibbles */
         else
            cmd[6+(i/2)] = (cmd[6+(i/2)] & 0xF0) + ((pin[i]-48)&0x0F);	/* pin[0], pin[1] in nibbles */
      }
      ret = reader->apdu(cmd, cmdlen, recv, &recvlen, &sw);
      if (ret) {
         log_error("%s: reader->apdu(verify) returned 0x%0x", WHERE, ret);
      }
   }
   else {
      ret = -1;
   }
   
    if (ret < 0)
      CLEANUP(ret);
   
   switch (sw)
   {
      case 0x9000:
         ret = 0;
         break;
      case 0x63C3:
         ret = E_PIN_3_ATTEMPTS;
         break;
      case 0x63C2:
         ret = E_PIN_2_ATTEMPTS;
         break;
      case 0x63C1:
         ret = E_PIN_1_ATTEMPT;
         break;
      case 0x6983:
         ret = E_PIN_BLOCKED;
         break;
      case 0x6985:
         ret = E_SRC_COMMAND_NOT_ALLOWED;
         break;
      default: ret = E_PIN_INCORRECT;
   }
   
cleanup:
   if (begintransaction)
      reader->endTransaction();
   
   return (ret);
}
#undef WHERE

#define WHERE "BEIDCard::logoff()"
int BEIDCard::logoff()
{
   int ret;
   unsigned char recv[1024];
   int recvlen = 1024;
   unsigned char cmd[255];
   int cmdlen;
   int sw = 0;
   
   int begintransaction = 1;
   
   //begin transaction
   ret = reader->beginTransaction();
   if (ret) {
      log_error("E: Logoff: Could not start transaction");
      begintransaction = 0;
      CLEANUP(E_SRC_START_TRANSACTION);
   }
   
   cmdlen = sizeof(logoutApdu)-1;
   memcpy(cmd, logoutApdu, cmdlen);
   
   ret = reader->apdu(cmd, cmdlen, recv, &recvlen, &sw);
   if (ret) {
      log_error("%s: reader->apdu(verify) returned 0x%0x", WHERE, ret);
   }

cleanup:
   if (begintransaction)
      reader->endTransaction();
   
   return (ret);
}
#undef WHERE

#define WHERE "BEIDCard::sign()"
int BEIDCard::sign(unsigned char* in, unsigned int l_in, int hashAlgo, unsigned char *out, unsigned int *l_out, int *sw)
{
   int ret = 0;
   unsigned char recv[1024];
   int recvlen = 1024;
   unsigned char cmd[255];
   int sign_lg = currentSelectedKeyLength;
   int cmdlen;
   int begintransaction = 1;

   if (sign_lg > *l_out) {
      return (-1);
   }

   //begin transaction
   ret = reader->beginTransaction();
   if (ret) {
      log_error("E: reader->beginTransaction() failed");
      begintransaction = 0;
      CLEANUP(E_SRC_START_TRANSACTION);
   }
   
   if ((l_in == 0) || (l_in != hash_length_for_algo(hashAlgo)) ) {
      log_error("hash input has wrong length for the specified digesting algo");
      CLEANUP(E_DIGEST_LEN);
   }
   
   memset(cmd, 0xFF, sizeof(cmd));
   
   /* sign */
   cmdlen = sizeof(generateSignatureCmd)-1;
   memcpy(cmd, generateSignatureCmd, cmdlen);
   
   if (currentSelectedKeyType == X509_KEYTYPE_RSA) {
      cmd[cmdlen] = hash_length_for_algoheader(hashAlgo) + l_in;
      cmdlen++;
      memcpy(&cmd[cmdlen], hash_header_for_algo(hashAlgo), hash_length_for_algoheader(hashAlgo));
      cmdlen += hash_length_for_algoheader(hashAlgo);
      memcpy(&cmd[cmdlen], in, l_in);
      cmdlen += l_in;
      cmd[cmdlen] = 0x00;      /* append Le */
   }
   else {
      //EC
      cmd[cmdlen] = l_in; cmdlen++;
      memcpy(&cmd[cmdlen], in, l_in);
      cmdlen += l_in;
      cmd[cmdlen] = 0;     /* Le should be 0 */
   }
   
   recvlen = (int) sizeof(recv);
   
   ret = reader->apdu(cmd, cmdlen, recv, &recvlen, sw);
   if (ret < 0) {
      log_error("%s: reader->apdu returned 0x%0X", WHERE, ret);
      goto cleanup;
   }
   
   cmdlen = sizeof(retrieveSignatureCmd)-1;
   memcpy(cmd, retrieveSignatureCmd, cmdlen);
   
   switch (*sw) {
      case 0x6160:
         cmd[4] = 0x60;
         break;
      case 0x6180:
         cmd[4] = 0x80;
         break;
      case 0x6100:
         cmd[4] = 0x00;
         break;
      case 0x6401:
         CLEANUP(E_PIN_CANCELLED);
      default:
         CLEANUP(-1);
   }
   
   recvlen = *l_out;
   
   ret = reader->apdu(cmd, cmdlen, out, &recvlen, sw);
   if ((ret < 0) || (*sw != 0x9000)) {
      log_error("%s: reader->apdu returned 0x%0X (sw=0x%0X)", WHERE, ret, *sw);
      goto cleanup;
   }

   *l_out = (unsigned int) recvlen;
   
   if (currentSelectedKeyType == X509_KEYTYPE_EC) {
      //signature is a concatenation of 2 values, so we encode these in ASN1 to return the ASN1 format
      ASN1_LIST     asn1_list;
      memset(&asn1_list, 0, sizeof(asn1_list));
      unsigned char part1[256]; //arbitrary length...
      unsigned char part2[256]; //arbitrary length...
      unsigned int size1 = *l_out/2;
      unsigned int size2 = *l_out/2;
      unsigned char *p = 0;
      memset(part1, 0, sizeof(part1));
      memset(part2, 0, sizeof(part2));
      //normalize as ASN_INTEGER (leading 0 if first byte > 0x7F)
      p = part1;
      if (out[0] > 0x7f) {
         p++;
         size1 += 1;
      }
      memcpy(p, out, *l_out/2);

      p = part2;
      if (out[*l_out/2] > 0x7f) {
         p++;
         size2 += 1;
      }
      memcpy(p, &out[*l_out/2], *l_out/2);
      
      asn1_add_item(&asn1_list, ASN_SEQUENCE, 0, 0, 2);
      {
         asn1_add_item(&asn1_list, ASN_INTEGER, part1, size1, 0);
         asn1_add_item(&asn1_list, ASN_INTEGER, part2, size2, 0);
      }
 
      unsigned char asn1_signature[512];
      unsigned int l_asn1_signature = 512;
      if ((ret = asn1_encode_list(&asn1_list, asn1_signature, &l_asn1_signature)) != 0)
      {
         log_error("asn1_encode returned 0x%08x (%d)", ret, ret);
         goto cleanup;
      }
      if (l_asn1_signature)
         memcpy(out, asn1_signature, l_asn1_signature);
      *l_out = l_asn1_signature;
   }

cleanup:
   
   if (begintransaction)
      reader->endTransaction();
   
   return (ret);
}
#undef WHERE

#define WHERE "BEIDCard::selectFile"
int BEIDCard::selectFile(unsigned char *file, int l_file)
{
   int ret = 0;
   unsigned char apdu[256] = "\x00\xA4\x02\x0C\x00";
   unsigned int l_apdu = 5;
   unsigned char recv[256];
   int recvlen = 256;
   int sw = 0;
#if 0
   apdu[4] = l_file;
   memcpy(apdu + l_apdu, file, l_file);
   
   l_apdu += l_file;
   
   do_sleep(5);
   ret = reader->apdu(apdu, l_apdu, recv, &recvlen, &sw);
   if (ret < 0) {
      log_error("%s: reader->apdu returned 0x%0X", WHERE, ret);
      return(ret);
   }
   if (sw != 0x9000)
      return(-1);
#else
   apdu[4] = 2;
   l_apdu += 2;
   for (int i = 0; i < (l_file / 2); i++) {
      memcpy(&apdu[5], &file[i*2], 2);
      do_sleep(5);
      ret = reader->apdu(apdu, l_apdu, recv, &recvlen, &sw);
      if (ret < 0) {
         log_error("%s: reader->apdu returned 0x%0X", WHERE, ret);
         return(ret);
      }
      if (sw != 0x9000)
         return(-1);
   }
#endif
   
   return(ret);
}
#undef WHERE

#define WHERE "BEIDCard::readFile2()"
int BEIDCard::readFile2(unsigned int offset, int* p_len, unsigned char* p_out)
{
   int iReturn = 0;
   
   unsigned int l_req;
   unsigned int l_read = 0;
   
   unsigned char recv[1024];
   /*DWORD*/int recvlen = 1024;
   unsigned char cmd[255];
   
   int sw;
   
   int 	cmdlen = sizeof(beid_apdu_binary)-1;
   memcpy(cmd, beid_apdu_binary, cmdlen);
   
   while((*p_len - l_read) > 0)                                       /* loop for big datasize     */
   {
      cmd[2] = (unsigned char)((offset + l_read) >> 8);             /* set reading startpoint    */
      cmd[3] = (unsigned char)(offset + l_read);
      l_req = *p_len - l_read;
      
      if(l_req > BEID_READ_BINARY_MAX_LEN)                                  /*if more than maximum length*/
         l_req = BEID_READ_BINARY_MAX_LEN;                               /* is requested, than read MAX_LEN  */
      
      cmd[4] = (unsigned char)(l_req);
      
      recvlen = 1024;
      
      do_sleep(20);  //reading ID fails if we don't wait 1ms here ????
      
      iReturn = reader->apdu(cmd, cmdlen, recv, &recvlen, &sw);
      if (iReturn) {
         log_error("%s: reader->apdu returned 0x%0X", WHERE, iReturn);
         goto cleanup;
      }
      
      //check if we want to read too much
      if ((sw & 0xFF00)  == 0x6c00)
      {
         *p_len = l_read + (sw & 0xFF);
         continue;
      }
      else if (sw == 0x6B00)
      {
         //we wanted to read data that is not there so we stop reading and return what we have received so far
         break;
      }
      else if(recvlen == 0)
         break;

      //check if we received less than we requested for (DIGIPASS...)
      //we cannot do it this way since if we are on a boundary, this will not work read 250 bytes recv 250 bytes gives 90 00 but following read of 250 bytes will return 6B00,
      //so we need to keep reading and when receivein 6B 00, we stop and return what we have read so far
      memcpy( p_out + l_read, recv, recvlen);                     /* store read bytes          */
      l_read += recvlen;

	  if ((int)l_req > recvlen)
		  break;
   }
   
   *p_len = l_read;                                                   /*set whole length for answer*/
   
cleanup:
   
   return iReturn;
}
#undef WHERE


#define WHERE "BEIDCard::getFile"
std::vector<char> BEIDCard::getFile(int format, std::string fileType)
{
   int ret = 0;
   int len = MAX_ID_FILE_SIZE;
   unsigned char p[MAX_ID_FILE_SIZE];
   unsigned char* p_out;
 
   std::unordered_map<std::string,std::vector<char>> idFiles;
   std::vector<char> file;

   const char* id           = "\x3F\x00\xDF\x01\x40\x31";
   const char* id_sig       = "\x3F\x00\xDF\x01\x40\x32";
   const char* address      = "\x3F\x00\xDF\x01\x40\x33";
   const char* address_sig  = "\x3F\x00\xDF\x01\x40\x34";
   const char* photo        = "\x3F\x00\xDF\x01\x40\x35";
   const char* rrncert      = "\x3F\x00\xDF\x00\x50\x3C";
   const char* authcert     = "\x3F\x00\xDF\x00\x50\x38";
   const char* signcert     = "\x3F\x00\xDF\x00\x50\x39";
   const char* cacert       = "\x3F\x00\xDF\x00\x50\x3A";
   const char* rootcert     = "\x3F\x00\xDF\x00\x50\x3B";
   int path_len = 6;
   
   idFiles["id"] = std::vector<char>(id, id+ path_len);
   idFiles["id_sig"] = std::vector<char>(id_sig, id_sig+ path_len);
   idFiles["address"] = std::vector<char>(address, address+ path_len);
   idFiles["address_sig"] = std::vector<char>(address_sig, address_sig+ path_len);
   idFiles["photo"] = std::vector<char>(photo, photo+ path_len);
   idFiles["rrncert"] = std::vector<char>(rrncert, rrncert+ path_len);
   idFiles["authcert"] = std::vector<char>(authcert, authcert+ path_len);
   idFiles["signcert"] = std::vector<char>(signcert, signcert+ path_len);
   idFiles["cacert"] = std::vector<char>(cacert, cacert+ path_len);
   idFiles["rootcert"] = std::vector<char>(rootcert, rootcert+ path_len);

   ret = Card::getFile((unsigned char *)idFiles[fileType].data(), path_len, &len, p);
   if (ret) {
      log_error("%s getFile returned %d (0x%0X)", WHERE, ret, ret);
      CLEANUP(ret);
   }
   
   if (format == FORMAT_RADIX64) {
      int b64len =  base64encode_len(len);
      if ((p_out = (unsigned char*) malloc(b64len)) == NULL) {
         CLEANUP(E_ALLOC_ERR);
      }
      base64encode(p, len, p_out);
      std::vector<char> buf(p_out, p_out + b64len);
      file = buf;
      free(p_out);
      p_out = NULL;
   }
   else {
      std::vector<char> buf(p, p + len);
      file = buf;
   }

cleanup:
   return file;
}

